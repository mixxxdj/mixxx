/***************************************************************************
                          readerextractwave.h  -  description
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

#ifndef READEREXTRACTWAVE_H
#define READEREXTRACTWAVE_H

#include "readerextract.h"
#include "defs.h"
#include "monitor.h"
#include "soundsource.h"
#include <qevent.h>
#include <qmutex.h>

class SignalVertexBuffer;
class ReaderExtractFFT;

/**
  *@author Tue & Ken Haste Andersen
  */

class ReaderExtractWave : ReaderExtract {
public: 
    ReaderExtractWave(QMutex *_enginelock);
    ~ReaderExtractWave();
    void reset();
    void *getChunkPtr(const int idx);
    int getRate();
    void *processChunk(const int, const int, const int);
    void setSoundSource(SoundSource *_file);
    void setSignalVertexBuffer(SignalVertexBuffer *_signalVertexBuffer);
    /** Read a new chunk into the readbuffer: */
    void getchunk(CSAMPLE rate);
    /** Seek to a new play position. Returns positon actually seeked to */
    long int seek(long int new_playpos);

    /** The first read sample in the file, currently in read_buffer. This should only be
      * accessed while holding the enginelock from the reader class. */
    long int filepos_start;
    /** The last read sample in the file, currently in read_buffer. This should only be
      * accessed while holding the enginelock from the reader class. */
    long int filepos_end;
private:
    /** Pointer to enginelock mutex in the Reader class */
    QMutex *enginelock;
    /** Temporary buffer for the raw samples */
    SAMPLE *temp;
    /** Buffer start and end position */
    int bufferpos_start, bufferpos_end;
    /** Pointer to SignalVertexBuffer associated with this buffer */
    SignalVertexBuffer *signalVertexBuffer;
    /** Pointer to the sound source */
    SoundSource *file;
    int stepSize;
    int windowPerChunk;
    int windowNo;
    /** The buffer where the samples are read into */
    CSAMPLE *read_buffer;
    /** Pointer to FFT extractor */
    ReaderExtractFFT *readerfft;
};

#endif
