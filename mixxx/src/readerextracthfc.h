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

//#include <qfile.h>

class EngineSpectralFwd;
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
    void *getBasePtr();
    int getRate();
    int getChannels();
    int getBufferSize();
    void *processChunk(const int idx, const int start_idx, const int end_idx, bool);
private:
    int frameNo;
    int framePerChunk;
    /** Array of hfc and first derivative of hfc */
    CSAMPLE *hfc, *dhfc;
    QPtrList<EngineSpectralFwd> *specList;

    //QFile textout;    
};

#endif
