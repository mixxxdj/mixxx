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

#include <qstring.h>

class VisualChannel;
class TrackInfoObject;
class EngineBuffer;

/**
  * Abstract class for feature extraction. Each ReaderExtract object sets up its own buffer
  * containing READCHUNK_NO chunks. Each chunk containing a number of samples relative to the
  * ReaderExtractWave object. processChunk is called whenever a chunk needs to be updated.
  *
  *@author Tue Haste Andersen
  */

class ReaderExtract
{
public:
    ReaderExtract(ReaderExtract *_input, EngineBuffer *pEngineBuffer, QString qsVisualSignalType);

    virtual ~ReaderExtract();
    /** Called when a new sound source is loaded */
    virtual void newSource(TrackInfoObject *) = 0;
    /** Reset the buffer. This is when seeking */
    virtual void reset() = 0;
    /** Get pointer to start of buffer array, pointer to list or however data are stored */
    virtual void *getBasePtr() = 0;
    /** Get sample rate of buffer. This relates to the sample rate of the waveform */
    virtual int getRate() = 0;
    /** Get number of channels in the buffer. Most ReaderExtract classes average down to one channel */
    virtual int getChannels() = 0;
    /** Get buffer size. Dependent on number of channels */
    virtual int getBufferSize() = 0;
    /** Process a given chunk at chunk idx i. start_idx and end_idx gives the indexes of the the chunks
      * at the update boundaries of the buffer. Returns a pointer to the newly processed chunk */
    virtual void *processChunk(const int idx, const int start_idx, const int end_idx, bool backwards, const long signed int filepos_start) = 0;
protected:
    /** Pointer to input object */
    ReaderExtract *input;
    /** Pointer to EngineBuffer object */
    EngineBuffer *m_pEngineBuffer;
    /** Holds visual signal type */
    QString m_qsVisualDataType;
};

#endif
