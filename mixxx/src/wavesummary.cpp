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
        m_qMutex.lock();
        TrackInfoObject *pTrackInfoObject = m_qQueue.dequeue();
        m_qMutex.unlock();

        if (!pTrackInfoObject)
        {
            // Wait for track to be requested
            m_qWait.wait();
            m_qMutex.lock();
            pTrackInfoObject = m_qQueue.dequeue();
            m_qMutex.unlock();
        }

//         qDebug("start generate for %s",pTrackInfoObject->getLocation().latin1());
        //// Check if preview has been generated in the meantime
        //QMemArray<char> *p = pTrackInfoObject->getWaveSummary();
        //if (!p || !p->size())
        {
            // Generate summary
            SoundSourceProxy *pSoundSource = new SoundSourceProxy(pTrackInfoObject->getLocation());
            
            // Objects for extrating HFC values
            const int kiBlockSizeMax = 2048;
            float *pSpectralBuffer = new float[kiBlockSizeMax];
            WindowKaiser *pWindow = new WindowKaiser(kiBlockSizeMax, 6.5);
            EngineSpectralFwd *pSpectral = new EngineSpectralFwd(true, false, pWindow);

            const int kiBlocks = 150;
            
            // Block size depends of file size, we want ~kiBlocks blocks
            int iBlockSize = pSoundSource->length()/kiBlocks;
            if (!even(iBlockSize))
                --iBlockSize;

            // Read and store summary
            SAMPLE *pBuffer = new SAMPLE[iBlockSize];
            QMemArray<char> *pData = new QMemArray<char>(kiBlocks*3);
            for (unsigned int i=0; i<pData->size(); ++i)
                pData->at(i) = 0;

            int r=pSoundSource->read(iBlockSize, pBuffer);


//         qDebug("len %i req %i, got %i",pSoundSource->length(), iBlockSize, r);
            int no=0;
            while (r==iBlockSize && no<kiBlocks*3-2)
            {
                // Find min and max in buffer
                int iMin=0, iMax=0;
                for (int j=0; j<iBlockSize; ++j)
                {
                    if (pBuffer[j]<0)
                        iMin += pBuffer[j];
                    if (pBuffer[j]>0)
                        iMax += pBuffer[j];
                }

                // Scale to range used in visual display
                iMin = max(iMin/((iBlockSize/5)*256),-127);
                iMax = min(iMax/((iBlockSize/5)*256),127);

/*
                // Low pass min and max
                for (int k=no-3; k>=(max(no-9,0)); k-=3)
                {    
                    iMin += pData->at(k);
                    iMax += pData->at(k+1);
                }
                iMin /= 4;
                iMax /= 4;
*/
                
                // Store min & max
                pData->at(no) = iMin;
                pData->at(no+1) = iMax;
                
                // Find HFC value
                for (int i=0; i<min(kiBlockSizeMax, iBlockSize); ++i)
                    pSpectralBuffer[i] = pBuffer[i];
                pSpectral->process(pSpectralBuffer, 0, kiBlockSizeMax);
                float fHfc = pSpectral->getHFC();
                
                // Low pass filter HFC
                const int kiFilterLen = 5;
                int lp = (short int)(min(fHfc,100000)*(256./100000.)-128);
/*
                for (int k=no-3+2; k>=(max(no+2-((kiFilterLen)*3),2)); k-=3)
                    lp += pData->at(k);
                lp /= kiFilterLen;
*/
                
                // Store HFC
                pData->at(no+2) = max(min(lp,127),-127);
                //qDebug("hfc extract %f, store %i",pSpectral->getPSF(), pData->at(no+2));
                no+=3;

                // Store summary in TrackInfoObject
                QApplication::postEvent(pTrackInfoObject, new WaveSummaryEvent(pData, 0));

                r=pSoundSource->read(iBlockSize, pBuffer);
            }

            QValueList<long> *pSegPoints;
            
            // Get segmentation points stored in sound file...
            pSegPoints = pSoundSource->getCuePoints();
            
            long liSampleDuration = pTrackInfoObject->getDuration()*pTrackInfoObject->getSampleRate()*pTrackInfoObject->getChannels();
            
            // If no points are stored in the file, generate some...
            if (!pSegPoints)
            {
                pSegPoints = new QValueList<long>;
                
                // Find segmentation points
                for (int i=3; i<kiBlocks*3; i=i+3)
                {
                    // Find peaks and depths
                    if (abs(pData->at(i)-pData->at(i-3))>1 || abs(pData->at(i+1)-pData->at(i-2))>1)
                        pSegPoints->append(i);
                }
                
                // Prune by removing segment points that are less than 3 index points in distance
                // (remove the one with lowest dA)
                QValueList<long>::iterator it = pSegPoints->begin();
                QValueList<long>::iterator it2 = it;
                ++it2;
                while (it2!=pSegPoints->end())
                {
                    int i1 = (*it);
                    int i2 = (*it2);
                    
                    if (i2-i1<18)
                    {
                        int d1 = max(abs(pData->at(i1)-pData->at(i1-3)),abs(pData->at(i1)-pData->at(i1+3)));
                        int d2 = max(abs(pData->at(i2)-pData->at(i2-3)),abs(pData->at(i2)-pData->at(i2+3)));
   
                        if (d1>d2)
                            it2 = pSegPoints->remove(it2);
                        else
                        {
                            it = pSegPoints->remove(it);
                            it2++;
                        }
                    }
                    else
                    {
                        ++it;
                        ++it2;
                    }
                }
                                
                // Prune segmentation point list by selecting max in derivative of amplitude.
                int iThreshold = 1;
                while (pSegPoints->size()>10)
                {
                    iThreshold += 1;
    
                    QValueList<long>::iterator it = pSegPoints->begin();
                    while (it!=pSegPoints->end())
                    {
                        int i = (*it);
                        if (max(abs(pData->at(i)-pData->at(i-3)),abs(pData->at(i)-pData->at(i+3)))<iThreshold)
                        //if (max(abs(pData->at(i+2)-pData->at(i-3+2)),abs(pData->at(i+2)-pData->at(i+3+2)))<iThreshold)
                        {
                            it = pSegPoints->remove(it);
                            //if (pSegPoints->size()<=10)
                            //    break;
                        }
                        else
                            ++it;
                    }
                }
            }

            // Transform segmentation points to sample values
            for (unsigned int i=0; i<pSegPoints->size(); ++i)
                (*pSegPoints)[i] = (*pSegPoints->at(i))*(liSampleDuration/(kiBlocks*3));
            
            // Store summary in TrackInfoObject
            QApplication::postEvent(pTrackInfoObject, new WaveSummaryEvent(pData, pSegPoints));
            
            delete [] pBuffer;
            delete pSpectral;
            delete pWindow;
            delete [] pSpectralBuffer;
            delete pSoundSource;
        }
    }
}

















