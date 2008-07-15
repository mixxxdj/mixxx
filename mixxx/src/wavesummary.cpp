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

#include <q3memarray.h>
#include <QtDebug>
#include <qapplication.h>
#include <qdatetime.h>

WaveSummary::WaveSummary(ConfigObject<ConfigValue> * _config)
{
    // Store config object
    m_Config = _config;
    
    start(QThread::IdlePriority);
}

WaveSummary::~WaveSummary()
{
    terminate();
}

void WaveSummary::enqueue(TrackInfoObject * pTrackInfoObject)
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
        TrackInfoObject * pTrackInfoObject = m_qQueue.dequeue();
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

        // Open sound file
        SoundSourceProxy * pSoundSource = new SoundSourceProxy(pTrackInfoObject);

        // Check if preview has been generated in the meantime
        Q3MemArray<char> *p = pTrackInfoObject->getWaveSummary();
        if (!p || p->size()==0)
        {

            // Allocate temp buffer
            SAMPLE * pBuffer = new SAMPLE[kiBlockSize*2];

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
            pBuffer = NULL;            

            qDebug() << "WaveSummary generation successful for " << pTrackInfoObject->getFilename();
        }

        if(pTrackInfoObject->getVisualWaveform() == NULL) {
            visualWaveformGen(pTrackInfoObject, pSoundSource);
        }

        delete pSoundSource;
        pSoundSource = NULL;

    }
}

void WaveSummary::visualWaveformGen(TrackInfoObject *pTrackInfoObject, SoundSourceProxy *pSoundSource) {
    
    int numSamples = pSoundSource->length();
    int sampleRate = pSoundSource->getSrate();
    
    int desiredSecondsToDisplay = 1; // MEH!    
    int samplesPerWindow = desiredSecondsToDisplay * sampleRate; // x seconds/window * y stereo samples / second = xy stereo samples / window
    int width = pTrackInfoObject->getVisualResampleRate();
    
    samplesPerWindow += (samplesPerWindow % width);
    int samplesPerPixel = samplesPerWindow / width; // xy stereo samples per window / z pixels per window = xy/z stereo samples per pixel

    if(samplesPerPixel % 2 != 0)
        samplesPerPixel--;

    //samplesPerPixel *= 2; // stereo samples per pixel * 2 mono samples / 1 stereo sample = mono samples per pixel

    int curSamples = numSamples + (numSamples % samplesPerPixel);
    int resultSamples = curSamples / samplesPerPixel; // total samples / samples per pixel = total pixels (downsampled stereo samples)

    if(resultSamples % 2 != 0)
        resultSamples++;

    QVector<float> *downsample = new QVector<float>(resultSamples);
    int i,j;
    int filePos = 0;

    // Set the buffer to zero
    for(i=0;i<resultSamples;i++) {
        (*downsample)[i] = 0;
    }


    pTrackInfoObject->setVisualResampleRate(double(sampleRate)/samplesPerPixel);
    // Allow the visual waveform to display this before we've populated it so
    // that it displays the wave as we work.
    pTrackInfoObject->setVisualWaveform(downsample);
    
    qDebug() << "Samplerate " << sampleRate << " Samples per pixel: " << samplesPerPixel << " downsamples " << resultSamples << " from " << numSamples;

    int bufRead = samplesPerPixel*2;

    if(bufRead % 2 != 0)
        bufRead++;

    SAMPLE *pBuffer = new SAMPLE[bufRead];

    filePos = pSoundSource->seek(0);
    filePos += pSoundSource->read(bufRead, pBuffer);

    qDebug() << "WaveSummary :: Beginning to downsample waveform.";
    j=0;
    SAMPLE sl,sr;
    SAMPLE maxl=0,maxr=0,minl=0,minr=0;
    float gmr=0, gml=0;

    int doWaveformDecay = 0;
    
    while(filePos < numSamples && (j+2) < resultSamples) {
        maxl=0;minl=0;
        maxr=0;minr=0;
        
        for(i=0;(i+1)<bufRead;i+=2) {

            sl = pBuffer[i];
            sr = pBuffer[i+1];

            if(sl > maxl)
                maxl = sl;
            if(sl < minl)
                minl = sl;
            if(sr > maxr)
                maxr = sr;
            if(sr < minr)
                minr = sr;
        }

        float fDecay = 0.99f;
        float max, min;

        max = math_max(abs(maxr),abs(minr));
        min = math_max(abs(maxl),abs(minl));
        //max = math_max(maxr,maxl);
        //min = -math_min(minr,minl);

        float temp = gml * fDecay;
        if(!doWaveformDecay || max > temp)
            gml = max;
        else
            gml = temp;

        temp = gmr * fDecay;
        if(!doWaveformDecay || min > temp)
            gmr = min;
        else
            gmr = temp;
           
        (*downsample)[j] = gml/32768.0;
        j++;

        (*downsample)[j] = gmr/32768.0;
        j++;
        
        filePos += pSoundSource->read(bufRead, pBuffer);
    }
    
    qDebug() << "WaveSummary :: Waveform downsampling finished.";

    delete [] pBuffer;
}
