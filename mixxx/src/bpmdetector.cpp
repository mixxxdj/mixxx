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

#include "bpmdetect.h"
#include <STTypes.h>

#include <qdatetime.h>

BpmDetector::BpmDetector(ConfigObject<ConfigValue> *_config)
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

void BpmDetector::enqueue(TrackInfoObject *pTrackInfoObject, BpmReceiver *pBpmReceiver)
{
    m_qMutex.lock();
    BpmDetectionPackage* package = new BpmDetectionPackage();
    package->_TrackInfoObject = pTrackInfoObject;
    package->_BpmReceiver = pBpmReceiver;  	
    qDebug() << package;
	m_qQueue.enqueue(package);
	m_qMutex.unlock();
	m_qWait.wakeAll();
}

void BpmDetector::run()
{
	int i = 0;   
    
	while (1)
	{
        TrackInfoObject *pTrackInfoObject = NULL;
        BpmReceiver *pBpmReceiver = NULL;

		// Check if there is a new track to process in the queue...
       
        qDebug() << "Unloading from queue";
		m_qMutex.lock();
		BpmDetectionPackage* package = m_qQueue.dequeue();        
		m_qMutex.unlock();

        if(package != NULL)
        {
            qDebug() << "Found BPM package for " << package->_TrackInfoObject->getTitle();
            pTrackInfoObject = package->_TrackInfoObject;
            pBpmReceiver = package->_BpmReceiver;
            delete package;
        }
        else
		{
            qDebug() << "That was apparently not the case...";
			// Wait for track to be requested
			m_qMutex.lock();
			m_qWait.wait(&m_qMutex);
			//m_qWait.wait(&m_qWaitMutex);

			//m_qMutex.lock();
			package = m_qQueue.dequeue();
            if(package != NULL)
            {
                qDebug() << "Attempt 2 was not null";
                pTrackInfoObject = package->_TrackInfoObject;
                pBpmReceiver = package->_BpmReceiver;
                delete package;
            }  
			m_qMutex.unlock();

            
            
		}
		Q_ASSERT(pTrackInfoObject != NULL);

		//
		// Track processing
		//

        qDebug() << "Moving right along...";            

		// Check if BPM has been detected in the meantime
		if (pTrackInfoObject->getBpmConfirm() == false || pTrackInfoObject->getBpm() == 0.)
		{
       	    #define CHUNKSIZE 4096

        	m_qMutex.lock();
        	int analyzeEntireSong = m_Config->getValueString(ConfigKey("[BPM]","AnalyzeEntireSong")).toInt();
        	m_qMutex.unlock();

        	m_qMutex.lock();
        	int minBpm = m_Config->getValueString(ConfigKey("[BPM]","BPMRangeStart")).toInt();
        	m_qMutex.unlock();

        	m_qMutex.lock();
        	int maxBpm = m_Config->getValueString(ConfigKey("[BPM]","BPMRangeEnd")).toInt();
        	m_qMutex.unlock();
        	

        	SoundSourceProxy *pSoundSource = new SoundSourceProxy(pTrackInfoObject);
        	int16_t data16[ CHUNKSIZE];  // for 16 bit samples
        	int8_t  data8[ CHUNKSIZE ];       // for 8 bit samples
        	soundtouch::SAMPLETYPE samples[ CHUNKSIZE * 2];
        	unsigned int length = 0, read = 0, totalsteps = 0, pos = 0, end = 0;
        	int channels = 2, bits = 16;
        	float frequency = 44100;

        	length = pSoundSource->length();
        	
        	if(analyzeEntireSong < 1)
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

        	totalsteps = ( length / CHUNKSIZE );
        	end = pos + length;

        	if(pTrackInfoObject->getSampleRate())
        	{
        		frequency = pTrackInfoObject->getSampleRate();
        	}
        	if(pTrackInfoObject->getChannels())
        	{
        		channels = pTrackInfoObject->getChannels();
        	}
        	if(pTrackInfoObject->getBitrate())
        	{
        		bits = pTrackInfoObject->getBitrate();
        	}

        	BpmDetect bpmd( channels, ( int ) frequency, maxBpm, minBpm );

        	int cprogress = 0;
        	pSoundSource->seek(pos);
        	do {
        			read = pSoundSource->read(CHUNKSIZE, data16);

        			if(read >= 2)
        			{
        			pos += read;

        				//****************************************************
        				// Replace:
        				//result = FMOD_Sound_ReadData( sound, data16, CHUNKSIZE, &read );

        				//****************************************************
        				for ( unsigned int i = 0; i < read ; i++ ) {
        					int16_t test = data16[i];
        					if(test > 0)
        					{
        						test = 0;
        					}
        					samples[ i ] = ( float ) data16[ i ] / 32768;
        				}
        				bpmd.inputSamples( samples, read / ( channels ) );
        			
        				cprogress++;
        				if ( cprogress % 250 == 0 ) {
        					if(pBpmReceiver)
                            {
                                m_qMutex.lock();
                                pBpmReceiver->setProgress(pTrackInfoObject, cprogress/10);
                                m_qMutex.unlock();                               
                            }
        				}
        			}
        	} while (read == CHUNKSIZE && pos <= length);

        	float BPM = bpmd.getBpm();
        	if ( BPM != 0. ) {
        		BPM = Correct_BPM( BPM, maxBpm, minBpm );
        		pTrackInfoObject->setBpm(BPM);
        		pTrackInfoObject->setBpmConfirm();
                if(pBpmReceiver){
                    pBpmReceiver->setComplete(pTrackInfoObject, false, BPM);
                }
                qDebug() << "BPM detection successful for" << pTrackInfoObject->getFilename();
        		delete pSoundSource;
        		continue;
        	}

        	qDebug("BPM detection failed the first time. Trying old version.");
        	
        	// *********************************************************************
        	// At this point the new BPM detection failed to extract a BPM. So,
        	// we will fallback to the old detection algorithm in hopes of obtaining
        	// a usable BPM estimation. If this also fails, then it is likely there
        	// isn't a way with existing BPM detection algorithms to accurately 
        	// estimate the BPM for this song. In that case the user will need to
        	// manually enter the bpm or tap it.
        	// *********************************************************************


        	// Allocate temp buffer
        			SAMPLE *pBuffer = new SAMPLE[kiBlockSize*2];

        	// Beat is extracted from the middle region of the sound file. Here the 
        	// min and max indexes are the boundaries of that region.
        	int iBeatLength = math_min(length, kiBeatBlockNo*kiBlockSize);
        	int iBeatBlockLength = iBeatLength/(kiBlockSize/2);
        	int iBeatPosStart = math_max(0,length/2-iBeatLength/2);

        	if(iBeatPosStart %2 != 0)
        	{
        		//Bug Fix: above formula allows iBeatPosStart
        		//to be odd (which is illegal)
        		iBeatPosStart--;
        	}
        	int iBeatPosEnd = math_min(length, iBeatPosStart+iBeatLength);


        	// Allocate buffer for first derivative of the PSF vector          
        	float *pDPsf = new float[iBeatBlockLength];

        	long liPos = pSoundSource->seek(iBeatPosStart);
        	liPos += pSoundSource->read(kiBlockSize, pBuffer);
        	int j = 0;
        	while (liPos<iBeatPosEnd)
        	{
        		// Mix to mono, rectangular window
        		for (int m=0; m<kiBlockSize; ++m)
        			windowedSamples[m] = (pBuffer[m*2]+pBuffer[m*2+1])*0.5; //*windowPtr[m];

        		// Perform FFT
        		m_pEngineSpectralFwd->process(windowedSamples, 0, kiBlockSize);

        		// Get PSF
        		pDPsf[j] = m_pEngineSpectralFwd->getPSF();

        		j++;

        		// Read a new block of samples
        		liPos += pSoundSource->read(kiBlockSize, pBuffer);

        		//qDebug("liPos, iBeatPosEnd: %i, %i", liPos, iBeatPosEnd);
        	}

        	int i;
        	// Take derivate of PSF
        	for (i=0; i<iBeatBlockLength-1; ++i)
        		pDPsf[i+1] = math_max(0.,pDPsf[i+1]-pDPsf[i]);
        	pDPsf[0] = 0.;

        	// Construct list of peaks
        	PeakList *pPeaks = new PeakList(iBeatBlockLength, pDPsf);
        	pPeaks->update(0, iBeatBlockLength);

        	// Initialize beat probability vector
        	ProbabilityVector *bpv = new ProbabilityVector(60.f/maxBpm, 60.f/minBpm, kiBeatBins);

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
        			if (interval<60.f/minBpm)
        				bpv->add(interval, pDPsf[(*it1).i]*pDPsf[(*it2).i]);

        			if (it2==pPeaks->begin() || interval>=60.f/minBpm)
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
	}
}


