/***************************************************************************
                          enginepreprocess.cpp  -  description
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

#include "enginepreprocess.h"
#include "configobject.h"

EnginePreProcess::EnginePreProcess()
{
    buffer = new CSAMPLE[MAX_BUFFER_LEN];
}

EnginePreProcess::~EnginePreProcess()
{
    delete [] buffer;
}

CSAMPLE *EnginePreProcess::process(const CSAMPLE *source, const int buf_size)
{
    return buffer;
}
