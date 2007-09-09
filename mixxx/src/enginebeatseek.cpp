/***************************************************************************
                          enginebeatseek.cpp  -  description
                             -------------------
    begin                : Fri Nov 5 2004
    copyright            : (C) 2004 by Tue Haste Andersen
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

#include "enginebeatseek.h"

EngineBeatSeek::EngineBeatSeek(const char * group)
{
    m_pTemp1 = new CSAMPLE[MAX_BUFFER_LEN];
    m_pTemp2 = new CSAMPLE[MAX_BUFFER_LEN];
    m_pTemp3 = new CSAMPLE[MAX_BUFFER_LEN];
}

EngineBeatSeek::~EngineBeatSeek()
{
}

void EngineBeatSeek::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
}

