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

#include <qdatetime.h>

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
        if (!p || p->size()==0 || pTrackInfoObject->getBpm()==0)
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
            int iBeatPosEnd = math_min(liLengthSamples, iBeatPosStart+iBeatLength);
  
            // Allocate buffer for first derivative of the PSF vector          
            float *pDPsf = new float[iBeatBlockLength];

            long liPos = pSoundSource->seek(iBeatPosStart);
            liPos += pSoundSource->read(kiBlockSize, pBuffer);
            int j = 0;
            while (liPos<=iBeatPosEnd)
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

            //
            // Extract volume profile
            //
            
            // Allocate and reset buffer used to store summary data: max and min amplitude for block, and HFC value
            QMemArray<char> *pData = new QMemArray<char>(kiSummaryBufferSize);
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
            delete [] pDPsf;
            delete pPeaks;
            delete bpv;
            delete pSoundSource;

            qDebug("generate successful for %s",pTrackInfoObject->getFilename().latin1());

        }
    }
}

















