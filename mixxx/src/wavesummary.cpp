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
#include "windowkaiser.h"
#include "wavesegmentation.h"
#include <qmemarray.h>
#include <qapplication.h>

WaveSummary::WaveSummary()
{
    start(QThread::IdlePriority);
}

WaveSummary::~WaveSummary()
{
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

        // Length of track in samples
        //long liSamplesPerSecond = pTrackInfoObject->getSampleRate()*pTrackInfoObject->getChannels();
        //qDebug("sample per second %li",liSamplesPerSecond);
        
//         qDebug("start generate for %s",pTrackInfoObject->getLocation().latin1());
        
		//// Check if preview has been generated in the meantime
        QMemArray<char> *p = pTrackInfoObject->getWaveSummary();
        //if (!p || !p->size())
        {
            //
            // Generate summary
            //

            // Open sound file
            SoundSourceProxy *pSoundSource = new SoundSourceProxy(pTrackInfoObject->getLocation());

            // Length of file in samples
            long liLengthSamples = pSoundSource->length();

            // Block and hop size
            int iHopSize = liLengthSamples/(kiSummaryBufferSize/3);
            if (!even(iHopSize))
                iHopSize--;
            int iBlockSize = iHopSize;
                
// qDebug("block size %i",iBlockSize);

//          qDebug("len %i samples %i ch %i srate %i",liSamplesPerSecond, liLengthSeconds,pTrackInfoObject->getChannels(),pTrackInfoObject->getSampleRate());
            
            // Allocate and reset buffer used to store summary data: max and min amplitude for block, and HFC value
            QMemArray<char> *pData = new QMemArray<char>(kiSummaryBufferSize);
            unsigned int i;
            for (i=0; i<pData->size(); ++i)
                pData->at(i) = 0;

            SAMPLE *pBuffer = new SAMPLE[iBlockSize];
            long int pos = 0;
            
            for (i=0; i<pData->size()-2; i+=3)
            {
                // Read a block of samples
                int iRead = pSoundSource->read(iBlockSize, pBuffer);

//                 qDebug("wanted %i, got %i",iBlockSize,iRead);
                
                // Seek to new position
                pos += iHopSize;
                if (iHopSize!=iBlockSize)
                    pSoundSource->seek(pos);
            
                // Find min and max value
                int iMin=0, iMax=0;
                for (int j=0; j<iBlockSize; ++j)
                {
                    if (pBuffer[j]<iMin)
                        iMin = pBuffer[j];
                    if (pBuffer[j]>iMax)
                        iMax = pBuffer[j];
                }
                
                // Store max and min amplitude
                pData->at(i) = max((iMin/256.),-127);
                pData->at(i+1) = min((iMax/256.),127);
                pData->at(i+2) = 0;
            
                // Store summary in TrackInfoObject
                QApplication::postEvent(pTrackInfoObject, new WaveSummaryEvent(pData, 0));            
            }
            qDebug("read %i, len %i",pos, liLengthSamples);
                
                            
            
/*
            // Initialize objects for extrating PSF values
            float *pSpectralBuffer = new float[kiBlockSize];
            WindowKaiser *pWindow = new WindowKaiser(kiBlockSize, 6.5);
            EngineSpectralFwd *pSpectral = new EngineSpectralFwd(true, false, pWindow);

			// Prepare segmentation psf
			int sr=pSoundSource->getSrate();
			int sStepSizeFrames=sr*kfFeatureStepSize;
			int sStepSizeSamples=sStepSizeFrames*2;
			int sStepsCount=pSoundSource->length()/sStepSizeSamples;
			int FileSizeSteps=(pSoundSource->length()-1)/sStepSizeSamples+1;

			
			double *sPSF_vector = new double[sStepsCount];


			bool BlockAhead=false;

			int BufferSize=2*(4*sStepSizeSamples+kiBlockSize);

			// Allocate temporary wave buffer
			SAMPLE *pBuffer = new SAMPLE[BufferSize*2];
            
			// Allocate and reset buffer used to store summary data: max and min amplitude for block, and HFC value
			QMemArray<char> *pData = new QMemArray<char>(kiSummaryBufferSize);
            unsigned int i;
			for (i=0; i<pData->size(); ++i)
                pData->at(i) = 0;

            // Read a block of samples
			int r=pSoundSource->read(BufferSize, pBuffer);
			int Cursor=0;

//         qDebug("len %i req %i, got %i",pSoundSource->length(), iStepSize, r);

			int no=0; 
			int sI=0;
			int Other=0;
            while (r==BufferSize && sI<FileSizeSteps)
            {
                // Find min and max in buffer
				bool DoSummary=(no<(long)sI*(long)300/(long)FileSizeSteps);
				if (DoSummary)
				{
					int iMin=0, iMax=0;
					for (int j=0; j<BufferSize; ++j)
					{
						if (pBuffer[j]<0)
							iMin += pBuffer[j];
						if (pBuffer[j]>0)
							iMax += pBuffer[j];
					}

                    // Store max and min amplitude
                    pData->at(no) = max(iMin/((BufferSize/5)*256),-127);
                    pData->at(no+1) = min(iMax/((BufferSize/5)*256),127);
				}

                // Find HFC value
                for (i=0; i<kiBlockSize; ++i)
				{
                    pSpectralBuffer[i] = pBuffer[Other * BufferSize + Cursor];
					if(++Cursor == BufferSize)
					{
						Other = 1 - Other;
						if (BlockAhead)
							BlockAhead=false;
						else
							r=pSoundSource->read(BufferSize, &pBuffer[Other*BufferSize]);
						Cursor=0;
					}
				}
                pSpectral->process(pSpectralBuffer, 0, kiBlockSize);
				//pSpectral->process(&pBuffer[i], 0, kiBlockSize);
				float psf=pSpectral->getPSF();
				sPSF_vector[sI]=psf;
                
                 //qDebug("PSF[%i]=%f",sI,psf);
                
                // Low pass filter HFC
				if(DoSummary)
				{
					int lp = (short int)(min(psf,70000)*(256./70000.)-128);
					for (int k=no+1; k>(max(no-9,0)); --k)
						lp += pData->at(k);
					lp /= 4;

					// Store HFC
					pData->at(no+2) = max(min(lp,127),-127);
					//qDebug("hfc extract %f, store %i",pSpectral->getPSF(), pData->at(no+2));
					no+=3;
					// Store summary in TrackInfoObject
					QApplication::postEvent(pTrackInfoObject, new WaveSummaryEvent(pData, 0));
				}
				sI++;
				Cursor = (Cursor+sStepSizeSamples-kiBlockSize) ;
				if (Cursor<0)
				{
					Cursor += BufferSize;
					Other = 1 - Other;
					BlockAhead=true;
				}
            }
            
            // Find segmentation points
			WaveSegmentation *seg=new WaveSegmentation();
			float sgPoints[30];
			int segCount=seg->Process(sPSF_vector,sStepsCount,sgPoints,30);
            QValueList<long> *segpoints = new QValueList<long>;
            for (i=0; i<segCount; i++)
            {
                    segpoints->append((long)sgPoints[i]*liSamplesPerSecond);
					qDebug("Segment boundary at %d seconds (%li samples)",(int)sgPoints[i],(long)sgPoints[i]*liSamplesPerSecond);

            }
            
            // Store summary in TrackInfoObject
            QApplication::postEvent(pTrackInfoObject, new WaveSummaryEvent(pData, segpoints));
*/
            
            

            //qDebug("segpoints %i", segpoints->size());

            delete [] pBuffer;
            delete pSoundSource;
//            delete pSpectral;
//             delete pWindow;
//             delete [] pSpectralBuffer;

//         qDebug("generate successful for %s",pTrackInfoObject->getFilename().latin1());

        }
    }
}

















