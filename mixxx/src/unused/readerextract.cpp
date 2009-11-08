/***************************************************************************
                          readerextract.cpp  -  description
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

#include "readerextract.h"

ReaderExtract::ReaderExtract(ReaderExtract * _input, EngineBuffer * pEngineBuffer, QString qsVisualDataType)
{
    m_qsVisualDataType = qsVisualDataType;
    input = _input;
    m_pEngineBuffer = pEngineBuffer;
}

ReaderExtract::~ReaderExtract()
{
}

