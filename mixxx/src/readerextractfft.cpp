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
    frameSize = _frameSize;
    frameStep = _frameStep;
    frameNo = (READBUFFERSIZE)/frameStep;
    framePerChunk = frameNo/READCHUNK_NO;
    
    // Allocate and calculate window
    window = new WindowKaiser(frameSize, 6.5);
    windowPtr = window->getWindowPtr();
    readbufferPtr = (CSAMPLE *)input->getChunkPtr(0);
    
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
    
void *ReaderExtractFFT::getChunkPtr(const int idx)
{
    return 0;
}

int ReaderExtractFFT::getRate()
{
    return 0;
}

void *ReaderExtractFFT::processChunk(const int idx, const int start_idx, const int end_idx)
{
    int frameFrom  = idx*framePerChunk;
    int frameTo    = (frameFrom+framePerChunk)%frameNo;

    qDebug("no %i, from %i ,to %i",frameNo,frameFrom,frameTo);
    
    if (frameTo>frameFrom)
        for (int i=frameFrom; i<frameTo; i++)
            processFrame(i);
    else
    {
        for (int i=frameFrom; i<frameNo; i++)
            processFrame(i);
        for (int i=0; i<frameTo; i++)
            processFrame(i);
    }

    return 0;
}

void ReaderExtractFFT::processFrame(int idx)
{
    //
    // Window samples
    //
    int framePos = (idx*frameStep-frameSize/2+READBUFFERSIZE)%READBUFFERSIZE;
    if (framePos+frameSize < READBUFFERSIZE)
        for (int i=framePos; i<framePos+frameSize; i++)
            windowedSamples[i-framePos] = readbufferPtr[i];
    else
        for (int i=0; i<frameSize; i++)
            windowedSamples[i] = readbufferPtr[(framePos+i)%READBUFFERSIZE]; // To optimize put % outside loop
    
    // Perform FFT
    specList.at(idx)->process(windowedSamples,0);

//    hfc[idx] = specList.at(idx)->getHFC();
}
