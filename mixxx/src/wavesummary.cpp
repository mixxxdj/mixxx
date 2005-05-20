/***************************************************************************
                          wavesummary.cpp  -  description
                             -------------------
    begin                : Wed Oct 13 2004
    copyright            : (C) 2004 by Tue Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "wavesummary.h"
#include "wavesummaryevent.h"
#include "trackinfoobject.h"
#include "soundsourceproxy.h"
#include "mathstuff.h"
#include "enginespectralfwd.h"
#include "peaklist.h"
#include "probabilityvector.h"
#include "windowkaiser.h"
#include "wavesegmentation.h"
#include "readerextractbeat.h"
#include <qmemarray.h>
#include <qapplication.h>

WaveSummary::WaveSummary()
{
    // Allocate and calculate window
    window = new WindowKaiser(kiBlockSize, 6.5);
    windowPtr = window->getWindowPtr();

    // Allocate memory for windowed portion of signal
    windowedSamples = new CSAMPLE[kiBlockSize];

    // Allocate FFT object
    m_pEngineSpectralFwd = new EngineSpectralFwd(true, false, window);

    start(QThread::IdlePriority);
}

WaveSummary::~WaveSummary()
{
    delete windowedSamples;
    delete m_pEngineSpectralFwd;
}

void WaveSummary::enqueue(TrackInfoObject *pTrackInfoObject)
{
    m_qMutex.lock();
    m_qQueue.enqueue(pTrackInfoObject);
    m_qMutex.unlock();
    m_qWait.wakeAll();
}

void WaveSummary::run()
{
	while (1)
    {
        // Check if there is a new track to process in the queue...
        m_qMutex.lock();
        TrackInfoObject *pTrackInfoObject = m_qQueue.dequeue();
        m_qMutex.unlock();

        // If that's not the case...
        if (!pTrackInfoObject)
        {
            // Wait for track to be requested
            m_qWait.wait();
            m_qMutex.lock();
            pTrackInfoObject = m_qQueue.dequeue();
            m_qMutex.unlock();
        }

        //
        // Track processing
        //

        // Check if preview has been generated in the meantime
        QMemArray<char> *p = pTrackInfoObject->getWaveSummary();
        //if (!p || !p->size())
        {
            //
            // Generate summary
            //

            // Open sound file
            SoundSourceProxy *pSoundSource = new SoundSourceProxy(pTrackInfoObject->getLocation());

            // Allocate and reset buffer used to store summary data: max and min amplitude for block, and HFC value
            QMemArray<char> *pData = new QMemArray<char>(kiSummaryBufferSize);
            unsigned int i;
            for (i=0; i<pData->size(); ++i)
                pData->at(i) = 0;

            SAMPLE *pBuffer = new SAMPLE[kiBlockSize*2];

            
            //
            // Extract volume profile
            //
            
            // Length of file in samples
            long liLengthSamples = pSoundSource->length();

            // Seek length used when extracting volume profile
            int iSeekLength = (int)ceilf((float)liLengthSamples/((float)kiSummaryBufferSize/3.));
            
            // Beat is extracted from the middle region of the sound file. Here the 
            // min and max indexes are the boundaries of that region.
            int iBeatLength = min(liLengthSamples, kiBeatBlockNo*kiBlockSize);
            int iBeatBlockLength = iBeatLength/kiBlockSize;
            
            int iBeatPosStart = liLengthSamples/2-iBeatLength/2;
            int iBeatPosEnd = iBeatPosStart+iBeatLength;
  
            // Allocate buffer for first derivative of the PSF vector          
            float *pDPsf = new float[iBeatBlockLength];
            
            long liPos = 0;
            i=0;
            int j = 0;
            while (liPos<liLengthSamples)
            {
                // Read a block of samples
                liPos += pSoundSource->read(kiBlockSize, pBuffer);

                // Check if it's time to extract max and min
                if (liPos>i/3*iSeekLength)
                {
                    // Find min and max value
                    int iMin=0, iMax=0;
                    for (int j=0; j<kiBlockSize; ++j)
                    {
                        if (pBuffer[j]<iMin)
                            iMin = pBuffer[j];
                        if (pBuffer[j]>iMax)
                            iMax = pBuffer[j];
                    }
    
                    // Store max and min amplitude
                    pData->at(i) = (char)max((iMin/256.),-127);
                    pData->at(i+1) = (char)min((iMax/256.),127);
                    pData->at(i+2) = 0;
                    
                    // Store summary in TrackInfoObject
                    QApplication::postEvent(pTrackInfoObject, new WaveSummaryEvent(pData, 0));            
                    
                    i+=3;
                }
                
                // Check if this is the middle region of the file, then extract PSF
                if (liPos>=iBeatPosStart && liPos<iBeatPosEnd)
                {
                    // Mix to mono and window samples
                    for (int m=0; m<kiBlockSize; ++m)
                        windowedSamples[m] = (pBuffer[m*2]+pBuffer[m*2+1])*0.5*windowPtr[m];

                    // Perform FFT
                    m_pEngineSpectralFwd->process(windowedSamples, 0, kiBlockSize);

                    // Get PSF
                    pDPsf[j] = m_pEngineSpectralFwd->getPSF();
                
                    j++;
                }
            }
            
            //
            // Extract beat
            //
            
            // Take derivate of PSF
            for (int i=0; i<iBeatBlockLength-1; ++i)
                pDPsf[i+1] = max(0.,pDPsf[i+1]-pDPsf[i]);
            pDPsf[0] = 0.;
            
            // Construct list of peaks
            PeakList *pPeaks = new PeakList(iBeatBlockLength, pDPsf);
            pPeaks->update(0, iBeatBlockLength);
            
            // Initialize beat probability vector
            ProbabilityVector *bpv = new ProbabilityVector(60.f/histMaxBPM, 60.f/histMinBPM, 1000);
            
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
                    float interval = pPeaks->getDistance(it2,it1)/((float)pSoundSource->getSrate()/float(kiBlockSize));
                    
                    // Update beat probability vector
                    if (interval<60.f/histMinBPM)
                        bpv->add(interval, pDPsf[(*it1).i]*pDPsf[(*it2).i]);
                  
                    if (it2==pPeaks->begin() || interval>=60.f/histMinBPM)
                        bInRange = false;
                    else
                        it2--;
                }
                it1++;
            }           
            
            // Set BPM
            if (!pTrackInfoObject->getBpmConfirm())
                pTrackInfoObject->setBpm(bpv->getBestBpmValue());
            
            delete [] pBuffer;
            delete [] pDPsf;
            delete pPeaks;
            delete bpv;
            delete pSoundSource;

//         qDebug("generate successful for %s",pTrackInfoObject->getFilename().latin1());

        }
    }
}

















