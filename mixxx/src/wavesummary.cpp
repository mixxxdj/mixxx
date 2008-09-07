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

#include <time.h>
#include <q3memarray.h>
#include <QtDebug>
#include <qapplication.h>
#include <qdatetime.h>

WaveSummary::WaveSummary(ConfigObject<ConfigValue> * _config) : 
    m_bShouldExit(false)
{
    // Store config object
    m_Config = _config;

    start(QThread::IdlePriority);
}

WaveSummary::~WaveSummary()
{
    qDebug() << "~WaveSummary";
    if(running())
        stop();
}

void WaveSummary::enqueue(TrackInfoObject * pTrackInfoObject)
{
    m_qMutex.lock();
    m_qQueue.enqueue(pTrackInfoObject);
    m_qMutex.unlock();
    m_qWait.wakeAll();
}

void WaveSummary::stop() {
    m_bShouldExit = true;
    m_qWait.wakeAll();
    wait();
}


void WaveSummary::run()
{
    while (!m_bShouldExit)
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

            pTrackInfoObject = m_qQueue.dequeue();
            m_qMutex.unlock();
        }
        Q_ASSERT(pTrackInfoObject != NULL);

        //
        // Track processing
        //

        // Open sound file
        SoundSourceProxy * pSoundSource = new SoundSourceProxy(pTrackInfoObject);

        // Check if preview has been generated
        Q3MemArray<char> *p = pTrackInfoObject->getWaveSummary();
        if (!p || p->size()==0) {
            waveformSummaryGen(pTrackInfoObject, pSoundSource);
        }

        if(pTrackInfoObject->getVisualWaveform() == NULL) {
            visualWaveformGen(pTrackInfoObject, pSoundSource);
        }

        delete pSoundSource;
        pSoundSource = NULL;

    }
    qDebug() << "WaveSummary::run() exiting";
}

void WaveSummary::waveformSummaryGen(TrackInfoObject *pTrackInfoObject, SoundSourceProxy *pSoundSource, bool updateUI) {

    // Allocate temp buffer
    SAMPLE * pBuffer = new SAMPLE[kiBlockSize*2];

    // Length of file in samples
    long liLengthSamples = pSoundSource->length();
    long liPos;
    int j, i=0;

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

    pTrackInfoObject->setWaveSummary(pData, 0, updateUI);

    delete [] pBuffer;
    pBuffer = NULL;

    qDebug() << "WaveSummary generation successful for " << pTrackInfoObject->getFilename();

}

void WaveSummary::visualWaveformGen(TrackInfoObject *pTrackInfoObject, SoundSourceProxy *pSoundSource) {

    // More info about this in WaveformRenderer
    // REQUIRE : M * N * Z = F
    // M : DOWNSAMPLES PER PIXEL
    // N : SAMPLES PER DOWNSAMPLE
    // F : SAMPLE RATE OF SONG
    // Z : THE USER SEES 1 SECOND OF DATA IN Z PIXELS
    // => N = F / (MZ)

    int numSamples = pSoundSource->length();
    if (numSamples <= 0) return;
    int f = pSoundSource->getSrate();
    double mz = pTrackInfoObject->getVisualResampleRate();
    double n = double(f) / mz;

    int samplesPerDownsample = n;
    int numDownsamples = numSamples / samplesPerDownsample;

    if(numDownsamples % 2 != 0)
        numDownsamples++;

    // Downsample from curSamples -> numDownsamples

    QVector<float> *downsample = new QVector<float>(numDownsamples);
    float *downsampleVector = downsample->data();
    int i,j;
    int filePos = 0;

    // Set the buffer to zero
    for(i=0;i<numDownsamples;i++) {
        (*downsample)[i] = 0;
    }

    // Allow the visual waveform to display this before we've populated it so
    // that it displays the wave as we work.
    pTrackInfoObject->setVisualWaveform(downsample);

    qDebug() << "WaveSummary: f " << f << " samplesPerDownsample: " << samplesPerDownsample << " downsamples " << numDownsamples << " from " << numSamples;

    // readLen and bufRead need to be divisible by 2
    int bufRead = samplesPerDownsample*2;
    int readLen = bufRead*100;
    int count = 20;

    SAMPLE *pBuffer = new SAMPLE[readLen];

    filePos = pSoundSource->seek(0);
    filePos += pSoundSource->read(readLen, pBuffer);

    int startTime = clock();
    qDebug() << "WaveSummary :: Beginning to downsample waveform.";
    j=0;
    SAMPLE sl,sr;
    SAMPLE maxl=0,maxr=0;

    int samplesAvailable = filePos;

    while(filePos < numSamples && (j+2) < numDownsamples) {

        SAMPLE* bufBase = pBuffer;
        while(samplesAvailable >= bufRead) {
            maxl=0; maxr=0;
            for(i=0; (i+1) < bufRead && (i+1) < count; i+=2) {
                sl = abs(bufBase[i]);
                sr = abs(bufBase[i+1]);

                if(sl > maxl)
                    maxl = sl;

                if(sr > maxr)
                    maxr = sr;
            }

            *(downsampleVector++) = maxl/32768.0;
            j++;

            *(downsampleVector++) = maxr/32768.0;
            j++;

            bufBase += bufRead;
            samplesAvailable -= bufRead;
        }
        int samplesRead = pSoundSource->read(readLen, pBuffer);
        if(samplesRead) {
            samplesAvailable += samplesRead;
            filePos += samplesRead;
        } else {
            /* early EOF return, break out of the loop. */
            qDebug() << "Waveform thread: got early EOF.";
            filePos = numSamples;
        }
    }

    qDebug() << "WaveSummary :: Waveform downsampling finished.";
    startTime = clock() - startTime;
    qDebug() << "WaveSummary :: Generation took " << double(startTime) / CLOCKS_PER_SEC << " seconds";
    delete [] pBuffer;
}
