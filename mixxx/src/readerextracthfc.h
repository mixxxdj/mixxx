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
/**
  *@author Tue & Ken Haste Andersen
  */

class ReaderExtractHFC : public ReaderExtract
{
public: 
    ReaderExtractHFC(ReaderExtract *input, int frameSize, int frameStep);
    ~ReaderExtractHFC();
    void reset();
    void *getBasePtr();
    int getRate();
    void *processChunk(const int idx, const int start_idx, const int end_idx);
private:
    int frameNo;
    int framePerChunk;
    CSAMPLE *hfc;
    QPtrList<EngineSpectralFwd> *specList;
};

#endif
