/***************************************************************************
   bpmdetector.cpp  -  The bpm detection queue
   -------------------
   begin                : Sat, Aug 4., 2007
   copyright            : (C) 2007 by Micah Lee
   email                : snipexv@gmail.com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#include "bpmdetector.h"
#include "bpmscheme.h"
#include "soundsourceproxy.h"
#include "trackinfoobject.h"
#include "mathstuff.h"
#include "enginespectralfwd.h"
#include "peaklist.h"
#include "probabilityvector.h"
#include "windowkaiser.h"
#include "wavesegmentation.h"
#include "readerextractbeat.h"
#include <q3memarray.h>
#include <QtDebug>
#include <qapplication.h>
#include "dlgbpmtap.h"
#ifdef __C_METRICS__
#include "cmetrics.h"
#endif
#include "bpmdetect.h"
#include <STTypes.h>

#include <qdatetime.h>

BpmDetector::BpmDetector(ConfigObject<ConfigValue> * _config)
{
    // Allocate and calculate window
    window = new WindowKaiser(kiBlockSize, 6.5);
    windowPtr = window->getWindowPtr();

    // Allocate memory for windowed portion of signal
    windowedSamples = new CSAMPLE[kiBlockSize];

    // Allocate FFT object
    m_pEngineSpectralFwd = new EngineSpectralFwd(true, false, window);

    // Store config object
    m_Config = _config;

    start(QThread::IdlePriority);

}

BpmDetector::~BpmDetector()
{
    terminate();
    delete windowedSamples;
    delete m_pEngineSpectralFwd;
}

int BpmDetector::queueCount() {
    return m_qQueue.count();
}

void BpmDetector::enqueue(TrackInfoObject * pTrackInfoObject, BpmReceiver * pBpmReceiver)
{
    int minBpm, maxBpm;
    bool entire;

    m_qMutex.lock();
    BpmDetectionPackage * package = new BpmDetectionPackage();
    package->_TrackInfoObject = pTrackInfoObject;
    package->_BpmReceiver = pBpmReceiver;

    // Read
    minBpm = m_Config->getValueString(ConfigKey("[BPM]","BPMRangeStart")).toInt();
    maxBpm = m_Config->getValueString(ConfigKey("[BPM]","BPMRangeEnd")).toInt();
    entire = (bool)m_Config->getValueString(ConfigKey("[BPM]","AnalyzeEntireSong")).toInt();

    package->_Scheme = new BpmScheme("Default", minBpm, maxBpm, entire);

    m_qQueue.enqueue(package);
    m_qMutex.unlock();
    m_qWait.wakeAll();
}

void BpmDetector::enqueue(TrackInfoObject * pTrackInfoObject, BpmScheme *scheme, BpmReceiver * pBpmReceiver)
{
    m_qMutex.lock();
    BpmDetectionPackage * package = new BpmDetectionPackage();
    package->_TrackInfoObject = pTrackInfoObject;
    package->_BpmReceiver = pBpmReceiver;
    package->_Scheme = scheme;
    m_qQueue.enqueue(package);
    m_qMutex.unlock();
    m_qWait.wakeAll();
}

/** Calculate the BPM using the SoundTouch BPM algorithm */
void BpmDetector::calculateBPMSoundTouch(TrackInfoObject *pTrackInfoObject, BpmReceiver * pBpmReceiver,
                                         BpmScheme * pScheme)
{
    const int numSamples = 8192;

    SoundSourceProxy * pSoundSource = new SoundSourceProxy(pTrackInfoObject);
    int16_t data16[numSamples];      // for 16 bit samples
    soundtouch::SAMPLETYPE samples[ numSamples ];
    unsigned int length = 0, read = 0, totalsteps = 0, pos = 0, end = 0;
    int channels = 2;
    float frequency = 44100;

    qDebug() << "BPM detection starting for" << pTrackInfoObject->getFilename();

    if(pTrackInfoObject->getSampleRate())
    {
        frequency = pTrackInfoObject->getSampleRate();
    }
    if(pTrackInfoObject->getChannels())
    {
        channels = pTrackInfoObject->getChannels();
    }

    length = pSoundSource->length();
    if (length <= 0) return;
    if(!pScheme->getAnalyzeEntireSong())
    {
        length = length / 2;
        pos = length / 2;

    }

    if(pos %2 != 0)
    {
        //Bug Fix: above formula allows iBeatPosStart
        //to be odd (which is illegal)
        pos--;
    }

    totalsteps = ( length / numSamples );
    end = pos + length;

    // Use default minBpm and maxBpm values?
    int defaultrange = m_Config->getValueString(ConfigKey("[BPM]","BPMAboveRangeEnabled")).toInt();
    BpmDetect bpmd( channels, ( int ) frequency, defaultrange ? MIN_BPM : pScheme->getMinBpm(),
                                                 defaultrange ? MAX_BPM : pScheme->getMaxBpm() );

    int cprogress = 0;
    pSoundSource->seek(pos);
    do {
        read = pSoundSource->read(numSamples, data16);

        if(read >= 2)
        {
            pos += read;
            for ( unsigned int i = 0; i < read ; i++ ) {
                samples[ i ] = ( float ) data16[ i ] / 32768;
            }
            bpmd.inputSamples( samples, read / channels );

            cprogress++;
            if ( cprogress % 250 == 0 ) {
                if(pBpmReceiver)
                {
                    m_qMutex.lock();
                    if (pTrackInfoObject)
                        pBpmReceiver->setProgress(pTrackInfoObject, cprogress/10);
                    m_qMutex.unlock();
                }
            }
        }
    } while (read == numSamples && pos <= end);

    float BPM = bpmd.getBpm();
    if ( BPM != 0 ) {
        BPM = BpmDetect::correctBPM(BPM, pScheme->getMinBpm(), pScheme->getMaxBpm());
        pTrackInfoObject->setBpm(BPM);
        pTrackInfoObject->setBpmConfirm();
        if(pBpmReceiver){
            pBpmReceiver->setComplete(pTrackInfoObject, false, BPM);
        }
        qDebug() << "BPM detection successful for" << pTrackInfoObject->getFilename();
        qDebug() << "BPM" << BPM;
        delete pSoundSource;
        return;
    }
    else
    {
        qDebug() << "BPM detection failed, setting to 0.";

#ifdef __C_METRICS__
                cm_writemsg_ascii(1, "BPM detection failed, setting to 0.");
#endif

                if(pBpmReceiver){
                    pBpmReceiver->setComplete(pTrackInfoObject, true, BPM);
                }

                delete pSoundSource;
                return;
            }

}

/** Calculate the BPM of a track using Tue/Ken's original algorithm (we think that's what this is),
 *  which was used in Mixxx < 1.5.0. */
void BpmDetector::calculateBPMTue(TrackInfoObject *pTrackInfoObject, BpmReceiver * pBpmReceiver,
                                  BpmScheme * pScheme)
{
    SoundSourceProxy * pSoundSource = new SoundSourceProxy(pTrackInfoObject);
    unsigned int length = 0;

//The fallback is broken and is causing crashes.  This will break us out and set the BPM to 0
//Remove these lines if you get the old BPM detection working

//qDebug() << "BPM detection failed the first time. Trying old version.";


    // *********************************************************************
    // At this point the new BPM detection failed to extract a BPM. So,
    // we will fallback to the old detection algorithm in hopes of obtaining
    // a usable BPM estimation. If this also fails, then it is likely there
    // isn't a way with existing BPM detection algorithms to accurately
    // estimate the BPM for this song. In that case the user will need to
    // manually enter the bpm or tap it.
    // *********************************************************************


    // Allocate temp buffer
    SAMPLE * pBuffer = new SAMPLE[kiBlockSize*2];

    // Beat is extracted from the middle region of the sound file. Here the
    // min and max indexes are the boundaries of that region.
    int iBeatLength = math_min(length, kiBeatBlockNo*kiBlockSize);
    int iBeatBlockLength = iBeatLength/(kiBlockSize/2);
    int iBeatPosStart = math_max(0.0,length/2.0-iBeatLength/2.0);

    if(iBeatPosStart %2 != 0)
    {
        //Bug Fix: above formula allows iBeatPosStart
        //to be odd (which is illegal)
        iBeatPosStart--;
    }
    int iBeatPosEnd = math_min(length, iBeatPosStart+iBeatLength);


    // Allocate buffer for first derivative of the PSF vector
    float * pDPsf = new float[iBeatBlockLength];

    long liPos = pSoundSource->seek(iBeatPosStart);
    liPos += pSoundSource->read(kiBlockSize, pBuffer);
    int j = 0;
    while (liPos<iBeatPosEnd)
    {
        // Mix to mono, rectangular window
        for (int m=0; m<kiBlockSize; ++m)
            windowedSamples[m] = (pBuffer[m*2]+pBuffer[m*2+1])*0.5;             //*windowPtr[m];

        // Perform FFT
        m_pEngineSpectralFwd->process(windowedSamples, 0, kiBlockSize);

        // Get PSF
        pDPsf[j] = m_pEngineSpectralFwd->getPSF();

        j++;

        // Read a new block of samples
        liPos += pSoundSource->read(kiBlockSize, pBuffer);

        //qDebug() << "liPos, iBeatPosEnd: " << liPos << ", " << iBeatPosEnd;
    }

    int i;
    // Take derivate of PSF
    for (i=0; i<iBeatBlockLength-1; ++i)
        pDPsf[i+1] = math_max(0.,pDPsf[i+1]-pDPsf[i]);
    pDPsf[0] = 0.;

    // Construct list of peaks
    PeakList * pPeaks = new PeakList(iBeatBlockLength, pDPsf);
    pPeaks->update(0, iBeatBlockLength);

    // Initialize beat probability vector
    ProbabilityVector * bpv = new ProbabilityVector(60.f/(float)pScheme->getMaxBpm(),
                                                    60.f/(float)pScheme->getMinBpm(), kiBeatBins);

    // Calculate BPM
    PeakList::iterator it1 = pPeaks->begin();

    if (it1!=pPeaks->end())
        it1++;

    while (it1!=pPeaks->end())
    {
        PeakList::iterator it2 = it1;
        it2--;
        bool bInRange = true;
        while (bInRange)
        {
            // Interval in seconds between current peak (it) and a previous peak (it2)
            float interval = pPeaks->getDistance(it2,it1)/((float)pSoundSource->getSrate()/float (kiBlockSize));

            // Update beat probability vector
            if (interval<60.f/(float)pScheme->getMinBpm())
                bpv->add(interval, pDPsf[(*it1).i]*pDPsf[(*it2).i]);

            if (it2==pPeaks->begin() || interval>=60.f/(float)pScheme->getMinBpm())
                bInRange = false;
            else
                it2--;
        }
        it1++;
    }

    pTrackInfoObject->setBpm(bpv->getBestBpmValue());

    if(pBpmReceiver){
        pBpmReceiver->setComplete(pTrackInfoObject, true, bpv->getBestBpmValue());
    }


    delete [] pBuffer;
    delete [] pDPsf;
    delete pPeaks;
    delete bpv;
    delete pSoundSource;


    qDebug() << "BPM detection successful for" << pTrackInfoObject->getFilename();

}

void BpmDetector::run()
{
    while (1)
    {
        TrackInfoObject * pTrackInfoObject = NULL;
        BpmReceiver * pBpmReceiver = NULL;
        BpmScheme * pScheme = NULL;

        // Check if there is a new track to process in the queue...

        m_qMutex.lock();
        BpmDetectionPackage * package = m_qQueue.dequeue();
        m_qMutex.unlock();

        if(package != NULL)
        {
            pTrackInfoObject = package->_TrackInfoObject;
            pBpmReceiver = package->_BpmReceiver;
            pScheme = package->_Scheme;
            delete package;
        }
        else
        {
            // Wait for track to be requested
            m_qMutex.lock();
            m_qWait.wait(&m_qMutex);
            //m_qWait.wait(&m_qWaitMutex);

            //m_qMutex.lock();
            package = m_qQueue.dequeue();
            if(package != NULL)
            {
                pTrackInfoObject = package->_TrackInfoObject;
                pBpmReceiver = package->_BpmReceiver;
                pScheme = package->_Scheme;
                delete package;
            }
            m_qMutex.unlock(); 



        }
        Q_ASSERT(pTrackInfoObject != NULL);

        //
        // Track processing
        //

        // Check if BPM has been detected in the meantime
        if ((pTrackInfoObject->getBpmConfirm() == false || pTrackInfoObject->getBpm() == 0.)
            && (bool)m_Config->getValueString(ConfigKey("[BPM]","BPMDetectionEnabled")).toInt() == true) //and make sure BPM detection is enabled
        {
             //Calculumate!
             calculateBPMSoundTouch(pTrackInfoObject, pBpmReceiver, pScheme);
             //Old school / not-so-good BPM detection:
             //calculateBPMTue(pTrackInfoObject, pBpmReceiver, pScheme);
        }
    }
}


