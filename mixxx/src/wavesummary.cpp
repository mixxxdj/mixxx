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
		if (!p || p->size()==0)
		{
			// Open sound file
			SoundSourceProxy *pSoundSource = new SoundSourceProxy(pTrackInfoObject);

			// Allocate temp buffer
			SAMPLE *pBuffer = new SAMPLE[kiBlockSize*2];

			// Length of file in samples
			long liLengthSamples = pSoundSource->length();
            long liPos;
			int j;

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
			delete pSoundSource;

			qDebug() << "WaveSummary generation successful for" << pTrackInfoObject->getFilename();

		}
	}
}



















