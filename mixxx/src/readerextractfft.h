/***************************************************************************
                          readerextractfft.h  -  description
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

#ifndef READEREXTRACTFFT_H
#define READEREXTRACTFFT_H

#include "readerextract.h"
#include "defs.h"
#include <qptrlist.h>

class EngineSpectralFwd;
class WindowKaiser;

/**
  * FFT processing of wave buffer.
  *
  *@author Tue and Ken Haste Andersen
  */

class ReaderExtractFFT : public ReaderExtract
{
public:
    ReaderExtractFFT(ReaderExtract *input, int _frameSize, int _frameStep);
    ~ReaderExtractFFT();
    void reset();
    void *getBasePtr();
    int getRate();
    int getChannels();
    int getBufferSize();
    void *processChunk(const int idx, const int start_idx, const int end_idx, bool);
private:
    void processFrame(int idx);

    /** Pointer to window and windowed samples of signal */
    WindowKaiser *window;
    /** Pointer to samples containing one windowed frame of samples */
    CSAMPLE *windowedSamples;
    /** Pointer to array containing window */
    CSAMPLE *windowPtr;
    /** Pointer to read_buffer from ReaderExtractWave */
    CSAMPLE *readbufferPtr;
    /** Frame size */
    int frameSize;
    /** Number of frames */
    int frameNo;
    /** Step size */
    int frameStep;
    /** Frames per chunk */
    int framePerChunk;
    /** List of pointers to spectrums */
    QPtrList<EngineSpectralFwd> specList;
};

#endif
