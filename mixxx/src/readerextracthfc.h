/***************************************************************************
                          readerextracthfc.h  -  description
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

#ifndef READEREXTRACTHFC_H
#define READEREXTRACTHFC_H

#include "readerextract.h"
#include "defs.h"
#include <qptrlist.h>

class EngineSpectralFwd;
class WindowKaiser;

/**
  * Calculates the High Frequency Content as used in ReaderExtractBeat.
  *
  *@author Tue Haste Andersen
  */

class ReaderExtractHFC : public ReaderExtract
{
public: 
    ReaderExtractHFC(ReaderExtract *input, int frameSize, int frameStep);
    ~ReaderExtractHFC();
    void reset();
    void newSource(TrackInfoObject *);
    void *getBasePtr();
    int getRate();
    int getChannels();
    int getBufferSize();
    void *processChunk(const int idx, const int start_idx, const int end_idx, bool);
private:
    void processFftFrame(int idx);
    
    int frameNo;
    int framePerChunk, framePerFrameSize;
    int frameSize, frameStep;
    /** Pointer to window and windowed samples of signal */
    WindowKaiser *window;
    /** Pointer to samples containing one windowed frame of samples */
    CSAMPLE *windowedSamples;
    /** Pointer to array containing window */
    CSAMPLE *windowPtr;
    /** Pointer to read_buffer from ReaderExtractWave */
    CSAMPLE *readbufferPtr;
    /** Array of hfc and first derivative of hfc */
    CSAMPLE *hfc, *dhfc;
    EngineSpectralFwd *m_pEngineSpectralFwd;
};

#endif
