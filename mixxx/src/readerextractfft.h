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
    ReaderExtractFFT(ReaderExtract *input, int _specNo, WindowKaiser *window);
    ~ReaderExtractFFT();
    void *processChunk(const int idx);


//    void notify(double) {};
//    void update(int specFrom, int specTo);
private:
    int specNo;
    QPtrList<EngineSpectralFwd> specList;
    CSAMPLE *hfc;    
};

#endif
