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
            float *pSpectralBuffer = new float[2048];
            WindowKaiser *pWindow = new WindowKaiser(2048, 6.5);
            EngineSpectralFwd *pSpectral = new EngineSpectralFwd(true, false, pWindow);

            // Block size depends of file size, we want ~100 blocks
            int iBlockSize = pSoundSource->length()/100;
            if (!even(iBlockSize))
                --iBlockSize;

            // Read and store summary
            SAMPLE *pBuffer = new SAMPLE[iBlockSize];
            QMemArray<char> *pData = new QMemArray<char>(300);
            for (unsigned int i=0; i<pData->size(); ++i)
                pData->at(i) = 0;

            int r=pSoundSource->read(iBlockSize, pBuffer);


//         qDebug("len %i req %i, got %i",pSoundSource->length(), iBlockSize, r);
            int no=0;
            while (r==iBlockSize && no<298)
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

            // Store result
//             qDebug("short int min %i, max %i",iMin,iMax);
                pData->at(no) = max(iMin/((iBlockSize/5)*256),-127);
                pData->at(no+1) = min(iMax/((iBlockSize/5)*256),127);

                // Find HFC value
                for (int i=0; i<min(2048, iBlockSize); ++i)
                    pSpectralBuffer[i] = pBuffer[i];
                pSpectral->process(pSpectralBuffer, 0, 2048);

                // Low pass filter HFC
                int lp = (short int)(min(pSpectral->getHFC(),70000)*(256./70000.)-128);
                for (int k=no+1; k>(max(no-9,0)); --k)
                    lp += pData->at(k);
                lp /= 4;

                // Store HFC
                pData->at(no+2) = max(min(lp,127),-127);
                //qDebug("hfc extract %f, store %i",pSpectral->getPSF(), pData->at(no+2));
                no+=3;

                // Store summary in TrackInfoObject
                QApplication::postEvent(pTrackInfoObject, new WaveSummaryEvent(pData, 0));

                r=pSoundSource->read(iBlockSize, pBuffer);
            }

            QValueList<int> *segpoints = new QValueList<int>;
            
            // Check if segmentation points are stored in sound file...
            QValueList<long> *pSegFile = pSoundSource->getCuePoints();
            if (pSegFile)
            {
                qDebug("Found %i points in file",pSegFile->count());
                for (unsigned int i=0; i<pSegFile->count(); ++i)
                    segpoints->append((int)(300.*(float)(*pSegFile->at(i))/(float)pTrackInfoObject->getLength()));
            }
            else
            {
                // Find segmentation points
                for (int i=5; i<298; i=i+3)
                {
                    // Find peaks and depths
                    if ((pData->at(i)>pData->at(i-3) && pData->at(i)>pData->at(i+3))) // ||
                        //(pData->at(i)<pData->at(i-3) && pData->at(i)<pData->at(i+3)))
                        segpoints->append(i);
                }
    
                // Prune segmentation point list
                char threshold = 0;
                while (segpoints->size()>10 && threshold<100)
                {
                    threshold += 10;
    
                    QValueList<int>::iterator it = segpoints->begin();
                    while (it!=segpoints->end())
                    {
                        int i = (*it);
                        if (abs(pData->at(i)-pData->at(i-3))<threshold && abs(pData->at(i)-pData->at(i+3))<threshold)
                        {
                            it =segpoints->remove(it);
                            if (segpoints->size()<=10)
                                break;
                        }
                        else
                            ++it;
                    }
                }
            }
            
            // Store summary in TrackInfoObject
            QApplication::postEvent(pTrackInfoObject, new WaveSummaryEvent(pData, segpoints));

            qDebug("segpoints %i", segpoints->size());

            delete [] pBuffer;
            delete pSpectral;
            delete pWindow;
            delete [] pSpectralBuffer;
            delete pSoundSource;

//         qDebug("generate successful for %s",pTrackInfoObject->getFilename().latin1());

        }
    }
}

















