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

ReaderExtractFFT::ReaderExtractFFT(ReaderExtract *input, int _frameSize, int _frameStep) : ReaderExtract(input, "fft")
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


//    textout.setName("fftabs.txt");
//    textout.open( IO_WriteOnly );
//    textout2.setName("fftwave.txt");
//    textout2.open( IO_WriteOnly );
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

void *ReaderExtractFFT::processChunk(const int _idx, const int start_idx, const int _end_idx, bool)
{
    int end_idx = _end_idx;
    int idx = _idx;
    int frameFrom, frameTo;

//    qDebug("start %i, end %i, curr %i",start_idx, end_idx, idx);

    // Adjust range (circular buffer)
    if (start_idx>=_end_idx)
        end_idx += READCHUNK_NO;
    if (start_idx>_idx)
        idx += READCHUNK_NO;

    // From frame...
    if (idx>start_idx)
        frameFrom = ((((idx%READCHUNK_NO)*framePerChunk)-(frameSize/frameStep)+1)+frameNo)%frameNo;
    else
        frameFrom = (idx%READCHUNK_NO)*framePerChunk;

    // To frame...
    if (idx<end_idx-1)
        frameTo = ((idx+1)%READCHUNK_NO)*framePerChunk;
    else
        frameTo = (((((idx+1)%READCHUNK_NO)*framePerChunk)-(frameSize/frameStep))+frameNo)%frameNo;
        
//    qDebug("no %i, from %i ,to %i",frameNo,frameFrom,frameTo);
    
    if (frameTo>frameFrom)
        for (int i=frameFrom; i<=frameTo; i++)
            processFrame(i);
    else
    {
        int i;    
        for (i=frameFrom; i<frameNo; i++)
            processFrame(i);
        for (i=0; i<=frameTo; i++)
            processFrame(i);
    }

    return 0;
}

void ReaderExtractFFT::processFrame(int idx)
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

//    qDebug("windowing %i-%i",inputFramePos, (inputFramePos+inputFrameSize)%inputBufferSize);

    
//    // Write wave to text file
//    for (int i=0; i<frameSize; i++)
//        stream2 << windowedSamples[i] << " ";
//    stream2 << "\n";

    // Perform FFT
    CSAMPLE *tmp = specList.at(idx%frameNo)->process(windowedSamples,0);
//    qDebug("frame %i",idx%frameNo);
//    // Write FFT to text file
//    for (int i=0; i<frameSize/2; i++)
//        stream << tmp[i] << " ";
//    stream << "\n";
    
//    textout.flush();
//    textout2.flush();
}
