/***************************************************************************
                          readerextracthfc.cpp  -  description
                             -------------------
    begin                : Tue Mar 18 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
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

#include "readerextracthfc.h"
#include "enginespectralfwd.h"
#include "readerevent.h"
#include "windowkaiser.h"

ReaderExtractHFC::ReaderExtractHFC(ReaderExtract *input, EngineBuffer *pEngineBuffer, int _frameSize, int _frameStep) : ReaderExtract(input, pEngineBuffer, "hfc")
{
    frameSize = _frameSize;
    frameStep = _frameStep;
    frameNo = (input->getBufferSize()/input->getChannels())/frameStep;
    framePerChunk = frameNo/READCHUNK_NO;
    framePerFrameSize = frameSize/frameStep;

    // Allocate and calculate window
    window = new WindowKaiser(frameSize, 6.5);
    windowPtr = window->getWindowPtr();
    readbufferPtr = (CSAMPLE *)input->getBasePtr();

    // Allocate memory for windowed portion of signal
    windowedSamples = new CSAMPLE[frameSize];

    // Allocate FFT object
    m_pEngineSpectralFwd = new EngineSpectralFwd(true, false, window);

    // Allocate HFC and DHFC buffers
    hfc = new CSAMPLE[frameNo];
    dhfc = new CSAMPLE[frameNo];
    for (int i=0; i<frameNo; i++)
    {
        hfc[i] = 0.;
        dhfc[i] = 0.;
    }
}

ReaderExtractHFC::~ReaderExtractHFC()
{
    delete [] hfc;
    delete [] dhfc;
    delete [] windowedSamples;
    delete window;
    delete m_pEngineSpectralFwd;
}

void ReaderExtractHFC::reset()
{
    for (int i=0; i<frameNo; i++)
    {
        hfc[i] = 0.;
        dhfc[i] = 0.;
    }
}

void ReaderExtractHFC::newSource(TrackInfoObject *)
{
    reset();
}


void *ReaderExtractHFC::getBasePtr()
{
    return dhfc;
}

int ReaderExtractHFC::getRate()
{
    return input->getRate()/frameStep;
}

int ReaderExtractHFC::getChannels()
{
    return 1;
}

int ReaderExtractHFC::getBufferSize()
{
    return frameNo;
}

void *ReaderExtractHFC::processChunk(const int _idx, const int start_idx, const int _end_idx, bool, const long signed int)
{
    int end_idx = _end_idx;
    int idx = _idx;
    int frameFrom, frameTo;
    /** From and to frame used when calculating the DHFC */
    int frameFromDHFC, frameToDHFC;

    // Adjust range (circular buffer)
    if (start_idx>=_end_idx)
        end_idx += READCHUNK_NO;
    if (start_idx>_idx)
        idx += READCHUNK_NO;

    // From frame...
    if (idx>start_idx)
    {
        frameFrom = ((((idx%READCHUNK_NO)*framePerChunk)-framePerFrameSize+1)+frameNo)%frameNo;
        frameFromDHFC = (frameFrom-1+frameNo)%frameNo;
    }
    else
    {
        frameFrom = (idx%READCHUNK_NO)*framePerChunk;
        frameFromDHFC = frameFrom;
    }

    // To frame...
    if (idx<end_idx-1)
    {
        frameTo = ((idx+1)%READCHUNK_NO)*framePerChunk;
        frameToDHFC = frameTo;
    }
    else
    {
        frameTo = (((((idx+1)%READCHUNK_NO)*framePerChunk)-framePerFrameSize)+frameNo)%frameNo;
        frameToDHFC = (frameTo-1+frameNo)%frameNo;
    }

//    qDebug("hfc %i-%i",frameFrom,frameTo);

    // Get HFC
    if (frameTo>frameFrom)
    {
        for (int i=frameFrom; i<=frameTo; i++)
        {
            processFftFrame(i);
            hfc[i] = m_pEngineSpectralFwd->getPSF();
        }
//        qDebug("HFC vals 1 : %i",j);
    }
    else
    {
        int i;
        for (i=frameFrom; i<frameNo; i++)
        {
            processFftFrame(i);
            hfc[i] = m_pEngineSpectralFwd->getPSF();
        }
        for (i=0; i<=frameTo; i++)
        {
            processFftFrame(i);
            hfc[i] = m_pEngineSpectralFwd->getPSF();
        }
//        qDebug("HFC vals 2 : %i",j);
    }

//     qDebug("hfc %i: %f", frameFrom, hfc[frameFrom]);

    // Get DHFC, first derivative and HFC, rectified
    //dhfc[(idx*framePerChunk)%frameNo] = hfc[(idx*framePerChunk)%frameNo];

    while (frameToDHFC<frameFromDHFC)
        frameToDHFC+=frameNo;

    for (int i=frameFromDHFC; i<=frameToDHFC; i++)
        dhfc[i%frameNo] = max(0.,hfc[(i+1)%frameNo]-hfc[i%frameNo]);

    // Update vertex buffer by sending an event containing indexes of where to update.
    if (m_pVisualBuffer != 0)
        QApplication::postEvent(m_pVisualBuffer, new ReaderEvent(frameFromDHFC, frameToDHFC, getBufferSize(), getRate()));

    return (void *)&dhfc[frameFromDHFC];
}

void ReaderExtractHFC::processFftFrame(int idx)
{
//    QTextStream stream( &textout );
//    QTextStream stream2( &textout2 );
//    qDebug("fft %i",idx);
    //
    // Window samples
    //
    int inputBufferSize = input->getBufferSize();
    int inputFrameStep = frameStep*input->getChannels();
    int inputFrameSize = frameSize*input->getChannels();

    int inputFramePos = (idx*inputFrameStep+inputBufferSize)%inputBufferSize;
    if (inputFramePos+inputFrameSize < inputBufferSize)
        for (int i=0; i<frameSize; i++)
            windowedSamples[i] = ((readbufferPtr[inputFramePos+(i*2)]+readbufferPtr[inputFramePos+(i*2)+1])/2.)*windowPtr[i];
    else
        for (int i=0; i<frameSize; i++)
            windowedSamples[i] = ((readbufferPtr[(inputFramePos+(i*2))%inputBufferSize] +
                                   readbufferPtr[(inputFramePos+(i*2)+1)%inputBufferSize])/2.)*windowPtr[i]; // To optimize put % outside loop
    // Perform FFT
    m_pEngineSpectralFwd->process(windowedSamples, 0, 0);
}
