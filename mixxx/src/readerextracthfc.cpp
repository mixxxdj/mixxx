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

ReaderExtractHFC::ReaderExtractHFC(ReaderExtract *input, int frameSize, int frameStep) : ReaderExtract(input, "signal")
{
    frameNo = input->getBufferSize(); ///frameStep;
    framePerChunk = frameNo/READCHUNK_NO;
    framePerFrameSize = frameSize/frameStep;
    
    hfc = new CSAMPLE[frameNo];
    dhfc = new CSAMPLE[frameNo];
    for (int i=0; i<frameNo; i++)
    {
        hfc[i] = 0.;
        dhfc[i] = 0.;
    }
    
    specList = (QPtrList<EngineSpectralFwd> *)input->getBasePtr();

//    textout.setName("hfc.txt");
//    textout.open( IO_WriteOnly );
}

ReaderExtractHFC::~ReaderExtractHFC()
{
    delete [] hfc;
    delete [] dhfc;
}

void ReaderExtractHFC::reset()
{
    for (int i=0; i<frameNo; i++)
    {
        hfc[i] = 0.;
        dhfc[i] = 0.;
    }
}

void *ReaderExtractHFC::getBasePtr()
{
    return dhfc;
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

void *ReaderExtractHFC::processChunk(const int _idx, const int start_idx, const int _end_idx, bool)
{
//    QTextStream stream( &textout );

    int end_idx = _end_idx;
    int idx = _idx;
    int frameFrom, frameTo;
    /** From and to frame used when calculating the DHFC */
    int frameFromDHFC, frameToDHFC;

    // Adjust range (circular buffer)
    if (start_idx>=_end_idx)
        end_idx += READCHUNK_NO;
    if (start_idx>_idx)
        idx += READCHUNK_NO;

    // From frame...
    if (idx>start_idx)
    {
        frameFrom = ((((idx%READCHUNK_NO)*framePerChunk)-framePerFrameSize+1)+frameNo)%frameNo;
        frameFromDHFC = (frameFrom-1+frameNo)%frameNo;
    }
    else
    {
        frameFrom = (idx%READCHUNK_NO)*framePerChunk;
        frameFromDHFC = frameFrom;
    }
        
    // To frame...
    if (idx<end_idx-1)
    {
        frameTo = ((idx+1)%READCHUNK_NO)*framePerChunk;
        frameToDHFC = frameTo;
    }
    else
    {
        frameTo = (((((idx+1)%READCHUNK_NO)*framePerChunk)-framePerFrameSize)+frameNo)%frameNo;
        frameToDHFC = (frameTo-1+frameNo)%frameNo;
    }
    
//    qDebug("hfc %i-%i",frameFrom,frameTo);
        
    // Get HFC
//    int j = 0;
    if (frameTo>frameFrom)
    {
        for (int i=frameFrom; i<=frameTo; i++)
        {
            hfc[i] = specList->at(i)->getHFC();
//            stream << hfc[i] << "\n";
//            j++;
        }
//        qDebug("HFC vals 1 : %i",j);
    }
    else
    {
        int i;
        for (i=frameFrom; i<frameNo; i++)
        {
            hfc[i] = specList->at(i)->getHFC();
//            stream << hfc[i] << "\n";
//            j++;
        }
        for (i=0; i<=frameTo; i++)
        {
            hfc[i] = specList->at(i)->getHFC();
//            stream << hfc[i] << "\n";
//            j++;
        }
//        qDebug("HFC vals 2 : %i",j);
    }

//    stream << "\n";
//    textout.flush();

    // Get DHFC, first derivative and HFC, rectified
    //dhfc[(idx*framePerChunk)%frameNo] = hfc[(idx*framePerChunk)%frameNo];

    while (frameTo>frameToDHFC)
        frameToDHFC+=frameNo;

    for (int i=frameFromDHFC; i<=frameToDHFC; i++)
        dhfc[i%frameNo] = max(0.,hfc[(i+1)%frameNo]-hfc[i%frameNo]);
        
    return (void *)&dhfc[frameFromDHFC];
}
