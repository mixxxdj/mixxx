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
    frameNo = input->getBufferSize(); ///frameStep;
    framePerChunk = frameNo/READCHUNK_NO;
   
    hfc = new CSAMPLE[frameNo];
    dhfc = new CSAMPLE[frameNo];
    specList = (QPtrList<EngineSpectralFwd> *)input->getBasePtr();
}

ReaderExtractHFC::~ReaderExtractHFC()
{
    delete [] hfc;
    delete [] dhfc;
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
    return input->getRate();
}

int ReaderExtractHFC::getChannels()
{
    return input->getChannels();
}

int ReaderExtractHFC::getBufferSize()
{
    return input->getBufferSize();
}

void *ReaderExtractHFC::processChunk(const int idx, const int start_idx, const int end_idx)
{
    for (int i=idx*framePerChunk; i<(idx+1)*framePerChunk; i++)
    {
        int i2 = i%frameNo;
        hfc[i2] = specList->at(i2)->getHFC();
    }

    dhfc[(idx*framePerChunk)%frameNo] = hfc[(idx*framePerChunk)%frameNo];
    for (i=idx*framePerChunk+1; i<(idx+1)*framePerChunk; i++)
    {
        int i2 = i%frameNo;
        dhfc[i2] = hfc[i2]-hfc[i2-i];
//        qDebug("hfc(%i) %f",i2,dhfc[i2]);
    }    
        
    return (void *)&dhfc[idx];
}
