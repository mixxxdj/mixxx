/***************************************************************************
                          readerextract.h  -  description
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

#ifndef READEREXTRACT_H
#define READEREXTRACT_H


/**
  *@author Tue & Ken Haste Andersen
  */

class ReaderExtract
{
public: 
    ReaderExtract(ReaderExtract *_input);
    ~ReaderExtract();

    /** Reset the buffer. This may be used when loading a new track and when seeking */
    virtual void reset() = 0;
    /** Get the chunk size */
    virtual int getChunkSize() = 0;
    /** Get pointer to a given chunk */
    virtual void *getChunkPtr(const int) = 0;
    /** Get sample rate of buffer. This relates to the sample rate of the waveform */
    virtual int getRate() = 0;
    /** Process a given chunk. Returns a pointer to the newly processed chunk */
    virtual void *processChunk(const int) = 0;
protected:
    /** Sample rate of wave signal. Set in ReaderExtractWave */
    static int waverate;
    /** Number of chunks */
    static int chunkNo;
    /** Chunk size */
    int chunkSize;
    /** Pointer to input object */
    ReaderExtract *input;
};

#endif
