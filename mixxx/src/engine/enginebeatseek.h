/***************************************************************************
                          enginebeatseek.h  -  description
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

#ifndef ENGINEBEATSEEK_H
#define ENGINEBEATSEEK_H

#include "engineobject.h"

/**
  *@author Tue Haste Andersen
  */

class EngineBeatSeek : public EngineObject  
{
public:
    EngineBeatSeek(const char *group);
    ~EngineBeatSeek();
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
private:
    CSAMPLE *m_pTemp1, *m_pTemp2, *m_pTemp3;
};

#endif
