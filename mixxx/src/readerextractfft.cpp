/***************************************************************************
                          readerextractfft.cpp  -  description
                             -------------------
    begin                : Mon Feb 3 2003
    copyright            : (C) 2003 by Tue and Ken Haste Andersen
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "readerextractfft.h"
#include "enginespectralfwd.h"
#include "windowkaiser.h"
#include "configobject.h"

ReaderExtractFFT::ReaderExtractFFT(ReaderExtract *input, int _frameSize, int _frameStep) : ReaderExtract(input)
{
    // These values are relative this buffer. Input buffer can have another number of channels and thus a different
    // buffer size than this class.
    frameSize = _frameSize;
    frameStep = _frameStep;
    frameNo = (input->getBufferSize()/input->getChannels())/frameStep;
    framePerChunk = frameNo/READCHUNK_NO;
    
    // Allocate and calculate window
    window = new WindowKaiser(frameSize, 6.5);
    windowPtr = window->getWindowPtr();
    readbufferPtr = (CSAMPLE *)input->getBasePtr();
    
    // Allocate memory for windowed portion of signal
    windowedSamples = new CSAMPLE[frameSize];

    // Allocate list of EngineSpectralFwd objects, corresponding to one object for each
    // stepsize throughout the readbuffer of EngineBuffer
    specList.setAutoDelete(TRUE);

    for (int i=0; i<frameNo; i++)
        specList.append(new EngineSpectralFwd(true,false,window));
}

ReaderExtractFFT::~ReaderExtractFFT()
{
    delete window;
    delete [] windowedSamples;

    // Delete list
    specList.clear();
}

void ReaderExtractFFT::reset()
{
}
    
void *ReaderExtractFFT::getBasePtr()
{
    return (void *)&specList;
}

int ReaderExtractFFT::getRate()
{
    return input->getRate()/frameStep;
}

int ReaderExtractFFT::getChannels()
{
    return 1;
}

int ReaderExtractFFT::getBufferSize()
{
    return frameNo; //input->getBufferSize()/input->getChannels();
}

void *ReaderExtractFFT::processChunk(const int idx, const int start_idx, const int end_idx)
{
    int frameFrom  = idx*framePerChunk;
    int frameTo    = (idx+1)*framePerChunk; //(frameFrom+framePerChunk)%frameNo;

//    qDebug("no %i, from %i ,to %i",frameNo,frameFrom,frameTo);
    
    if (frameTo>frameFrom)
        for (int i=frameFrom; i<frameTo; i++)
            processFrame(i);
    else
    {
        for (int i=frameFrom; i<frameNo; i++)
            processFrame(i);
        for (i=0; i<frameTo; i++)
            processFrame(i);
    }

    return 0;
}

void ReaderExtractFFT::processFrame(int idx)
{
//    qDebug("fft %i",idx);
    //
    // Window samples
    //
    int inputBufferSize = input->getBufferSize();
    int inputFrameStep = frameStep*input->getChannels();
    int inputFrameSize = frameSize*input->getChannels();

    int inputFramePos = (idx*inputFrameStep-inputFrameSize/2+inputBufferSize)%inputBufferSize;
    if (inputFramePos+inputFrameSize < inputBufferSize)
        for (int i=0; i<frameSize; i++)
            windowedSamples[i] = (readbufferPtr[inputFramePos+(i*2)]+readbufferPtr[inputFramePos+(i*2)+1])/2.;
    else
        for (int i=0; i<frameSize; i++)
            windowedSamples[i] = (readbufferPtr[(inputFramePos+(i*2))%inputBufferSize] +
                                  readbufferPtr[(inputFramePos+(i*2)+1)%inputBufferSize])/2.; // To optimize put % outside loop
    
    // Perform FFT
    specList.at(idx)->process(windowedSamples,0);

//    hfc[idx] = specList.at(idx)->getHFC();
}
