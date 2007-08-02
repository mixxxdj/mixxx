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

#ifdef __EXPERIMENTAL_BPM__
#include "bpmdetect.h"
#include <STTypes.h>
#endif

#include <qdatetime.h>

WaveSummary::WaveSummary(ConfigObject<ConfigValue> *_config)
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

WaveSummary::~WaveSummary()
{
	terminate();
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
	int i = 0;
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
			m_qMutex.lock();
			m_qWait.wait(&m_qMutex);
			//m_qWait.wait(&m_qWaitMutex);

			//m_qMutex.lock();
			pTrackInfoObject = m_qQueue.dequeue();
			m_qMutex.unlock();
		}
		Q_ASSERT(pTrackInfoObject != NULL);

		//
		// Track processing
		//

		// Check if preview has been generated in the meantime
		Q3MemArray<char> *p = pTrackInfoObject->getWaveSummary();
		if (!p || p->size()==0 || pTrackInfoObject->getBpmConfirm() == false)
		{
			// Open sound file
			SoundSourceProxy *pSoundSource = new SoundSourceProxy(pTrackInfoObject);

			// Allocate temp buffer
			SAMPLE *pBuffer = new SAMPLE[kiBlockSize*2];

			// Length of file in samples
			long liLengthSamples = pSoundSource->length();


			//
			// Extract beat
			//

			// Beat is extracted from the middle region of the sound file. Here the 
			// min and max indexes are the boundaries of that region.
			int iBeatLength = math_min(liLengthSamples, kiBeatBlockNo*kiBlockSize);
			int iBeatBlockLength = iBeatLength/(kiBlockSize/2);
			int iBeatPosStart = math_max(0,liLengthSamples/2-iBeatLength/2);
			if(iBeatPosStart %2 != 0)
			{
				//Bug Fix: above formula allows iBeatPosStart
				//to be odd (which is illegal)
				iBeatPosStart--;
			}
			int iBeatPosEnd = math_min(liLengthSamples, iBeatPosStart+iBeatLength);

			//***********************************************************
			// Experimental Bpm Implementation [GSOC]
			//***********************************************************		
			extractBeat(pTrackInfoObject);  
			long liPos;
			int j;

#ifndef __EXPERIMENTAL_BPM__

			// Allocate buffer for first derivative of the PSF vector          
			float *pDPsf = new float[iBeatBlockLength];

			liPos = pSoundSource->seek(iBeatPosStart);
			liPos += pSoundSource->read(kiBlockSize, pBuffer);
			j = 0;
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

			// Take derivate of PSF
			for (i=0; i<iBeatBlockLength-1; ++i)
				pDPsf[i+1] = math_max(0.,pDPsf[i+1]-pDPsf[i]);
			pDPsf[0] = 0.;

			// Construct list of peaks
			PeakList *pPeaks = new PeakList(iBeatBlockLength, pDPsf);
			pPeaks->update(0, iBeatBlockLength);

			// Initialize beat probability vector
			ProbabilityVector *bpv = new ProbabilityVector(60.f/histMaxBPM, 60.f/histMinBPM, kiBeatBins);

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


			// Update BPM value in TrackInfoObject
			if (!pTrackInfoObject->getBpmConfirm()) {
				pTrackInfoObject->setBpm(bpv->getBestBpmValue());

				float conf = bpv->getBestBpmConfidence() / (float)1e10;

				// FIXME: This is in the wrong file
				if (conf > 1000.0f || conf < 0.0f) {
					// Something went pretty wrong there...
					conf = 0.0f;
				}

				conf = math_max(0., math_min(1., conf));

				if (conf > 0.75f)
					pTrackInfoObject->setBpmConfirm(true);
			}

#endif


			//
			// Extract volume profile
			//

			// Allocate and reset buffer used to store summary data: max and min amplitude for block, and HFC value
			Q3MemArray<char> *pData = new Q3MemArray<char>(kiSummaryBufferSize);
			for (i=0; i<pData->size(); ++i)
				pData->at(i) = 0;

			// Seek length used when extracting volume profile
			int iSeekLength = (int)ceilf((float)liLengthSamples/((float)kiSummaryBufferSize/3.));
			if (iSeekLength%2!=0)
				iSeekLength--;

			liPos = pSoundSource->seek(0);
			liPos += pSoundSource->read(kiBlockSize, pBuffer);
			i=0;
			j = 0;

			while (liPos<liLengthSamples && i<kiSummaryBufferSize-2)
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
				pData->at(i) = (char)math_max((iMin/256.),-127);
				pData->at(i+1) = (char)math_min((iMax/256.),127);
				pData->at(i+2) = 0;

				i+=3;

				// Seek to new pos
				liPos = pSoundSource->seek(iSeekLength*(i/3));

				// Read a new block of samples
				liPos += pSoundSource->read(kiBlockSize, pBuffer);
			}

			pTrackInfoObject->setWaveSummary(pData, 0);

			delete [] pBuffer;
#ifndef __EXPERIMENTAL_BPM__
			delete [] pDPsf;
			delete pPeaks;
			delete bpv;
#endif
			delete pSoundSource;

			qDebug() << "WaveSummary generation successful for" << pTrackInfoObject->getFilename();

		}
	}
}

void WaveSummary::extractBeat(TrackInfoObject *pTrackInfoObject)
{
#ifdef __EXPERIMENTAL_BPM__
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
	soundtouch::SAMPLETYPE samples[ CHUNKSIZE];
	unsigned int length = 0, read = 0, totalsteps = 0, pos = 0, end = 0;
	int channels = 2, bits = 16;
	float frequency = 44100;

	if(pTrackInfoObject->getType() == "ogg")
	{
		length = pTrackInfoObject->getLength();
	}
	else if(pTrackInfoObject->getType() == "mp3")
	{
		length = pSoundSource->length();
	}
	else
	{
		length = pSoundSource->length();
	}
	
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

	BPMDetect bpmd( channels, ( int ) frequency, maxBpm, minBpm );

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
				for ( unsigned int i = 0; i < read/2 ; i++ ) {
					int16_t test = data16[i];
					if(test > 0)
					{
						test = 0;
					}
					samples[ i ] = ( float ) data16[ i ] / 32768;
				}
				bpmd.inputSamples( samples, read / ( 2 * channels ) );
			
				cprogress++;
				if ( cprogress % 250 == 0 ) {
					/// @todo printing status (cprogress/totalsteps)
				}
			}
	} while ( /*result == FMOD_OK &&*/ read == CHUNKSIZE );

	float BPM = bpmd.getBpm();
	if ( BPM != 0. ) {
		BPM = Correct_BPM( BPM, maxBpm, minBpm );
		pTrackInfoObject->setBpm(BPM);
		pTrackInfoObject->setBpmConfirm();
		delete pSoundSource;
		return;
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

	// Update BPM value in TrackInfoObject
	if (!pTrackInfoObject->getBpmConfirm()) {
		pTrackInfoObject->setBpm(bpv->getBestBpmValue());

		float conf = bpv->getBestBpmConfidence() / (float)1e10;

		// FIXME: This is in the wrong file
		if (conf > 1000.0f || conf < 0.0f) {
			// Something went pretty wrong there...
			conf = 0.0f;
		}

		conf = math_max(0., math_min(1., conf));

		if (conf > 0.75f)
		{
			pTrackInfoObject->setBpmConfirm(true);
		}
		else
		{
			// Let the user know we can't accurately detect the BPM and that they'd better tap it.
			qDebug("BPM detection failed!");
		}
	}

	delete [] pBuffer;
	delete [] pDPsf;
	delete pPeaks;
	delete bpv;
	delete pSoundSource;


#endif
}


















