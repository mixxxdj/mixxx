/***************************************************************************
                          readerextractfft.cpp  -  description
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

#include "readerextractfft.h"
#include "enginespectralfwd.h"
#include "windowkaiser.h"
#include "configobject.h"

ReaderExtractFFT::ReaderExtractFFT(ReaderExtract *input, int _specNo, WindowKaiser *window) : ReaderExtract(input)
{
    specNo = _specNo;

    // Allocate list of EngineSpectralFwd objects, corresponding to one object for each
    // stepsize throughout the readbuffer of EngineBuffer
    specList.setAutoDelete(TRUE);

    hfc = new CSAMPLE[specNo];
    for (int i=0; i<specNo; i++)
    {
        specList.append(new EngineSpectralFwd(true,false,window));
        hfc[i] = 0.;
    }
}

ReaderExtractFFT::~ReaderExtractFFT()
{
}

/*
void ReaderExtractFFT::update(int specFrom, int specTo)
{
    if (specTo>specFrom)
        for (int i=specFrom; i<specTo; i++)
            process(i);    
    else
    {
        for (int i=specFrom; i<specNo; i++)
            process(i);
        for (int i=0; i<specTo; i++)
            process(i);
    }
}
*/

void *ReaderExtractFFT::processChunk(const int idx)
{
//    specList.at(idx)->process(input->getWindowPtr(idx),0);
    hfc[idx] = specList.at(idx)->getHFC();    
    //qDebug("hfc: %f",hfc[idx]);

    return 0;
}

