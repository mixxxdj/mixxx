/***************************************************************************
                          readerextracthfc.cpp  -  description
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

#include "readerextracthfc.h"
#include "enginespectralfwd.h"

ReaderExtractHFC::ReaderExtractHFC(ReaderExtract *input, int frameSize, int frameStep) : ReaderExtract(input)
{
    frameNo = READBUFFERSIZE/frameStep;
    framePerChunk = frameNo/READCHUNK_NO;
   
    hfc = new CSAMPLE[frameNo];
    specList = (QPtrList<EngineSpectralFwd> *)input->getBasePtr();
}

ReaderExtractHFC::~ReaderExtractHFC()
{
    delete [] hfc;
}

void ReaderExtractHFC::reset()
{
}

void *ReaderExtractHFC::getBasePtr()
{
    return hfc;
}

int ReaderExtractHFC::getRate()
{
    return 0;
}

void *ReaderExtractHFC::processChunk(const int idx, const int start_idx, const int end_idx)
{
    for (int i=idx*framePerChunk; i<idx*(framePerChunk+1); i++)
    {
        int i2 = i%frameNo;
        hfc[i2] = specList->at(i2)->getHFC();
    }
        
    return (void *)&hfc[idx];
}
