/***************************************************************************
                          soundbuffer.h  -  description
                             -------------------
    begin                : Thu Feb 6 2003
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

#ifndef SOUNDBUFFER_H
#define SOUNDBUFFER_H

#include "defs.h"
#include "monitor.h"
#include "soundsource.h"
#include "enginepreprocess.h"
#include "windowkaiser.h"
#include <qevent.h>
#include <qmutex.h>

class SignalVertexBuffer;

/**
  *@author Tue & Ken Haste Andersen
  */

class SoundBuffer {
public: 
    SoundBuffer(QMutex *_enginelock, int _chunkSize, int _chunkNo, int windowSize, int _stepSize);
    ~SoundBuffer();
    void setSoundSource(SoundSource *_file);
    void setSignalVertexBuffer(SignalVertexBuffer *_signalVertexBuffer);
    void getchunk(CSAMPLE rate);
    /** Seek to a new play position. Returns positon actually seeked to */
    long int seek(long int new_playpos);
    CSAMPLE *getChunkPtr(int chunkIdx);
    CSAMPLE *getWindowPtr(int windowIdx);
    /** Return sample rate of buffer */
    int getRate();
    /** Return size of samples read at each update signal (USER:1001) */
    int getChunkSize();

    /** The buffer where the samples are read into */
    CSAMPLE *read_buffer;

    /** The first read sample in the file, currently in read_buffer. This should only be
      * accessed while holding the enginelock from the reader class. */
    long int filepos_start;
    /** The last read sample in the file, currently in read_buffer. This should only be
      * accessed while holding the enginelock from the reader class. */
    long int filepos_end;

private:
    /** Pointer to enginelock mutex in the Reader class */
    QMutex *enginelock;
    
    SAMPLE *temp;

    /** Buffer start and end position */
    int bufferpos_start, bufferpos_end;
    /** Pointer to SignalVertexBuffer associated with this buffer */
    SignalVertexBuffer *signalVertexBuffer;

    /** Pointer to window and windowed samples of signal */
    WindowKaiser *window;
    CSAMPLE *windowedSamples;
    /** Pointer to pre-processing object */
    EnginePreProcess *preprocess;

    SoundSource *file;
    int chunkSize;
    int chunkNo;
    int stepSize;
    int windowPerChunk;
    int windowNo;
};

#endif
