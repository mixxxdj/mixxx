/***************************************************************************
                          enginebufferscale.cpp  -  description
                             -------------------
    begin                : Sun Apr 13 2003
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

#include "engine/enginebufferscale.h"

EngineBufferScale::EngineBufferScale()
{
    m_dTempo = 1.;
    buffer = new CSAMPLE[MAX_BUFFER_LEN];
    new_playpos = 0;
}

EngineBufferScale::~EngineBufferScale()
{
    delete [] buffer;
}

double EngineBufferScale::getNewPlaypos()
{
    return new_playpos;
}
