/***************************************************************************
                          visualbuffersignal.h  -  description
                             -------------------
    begin                : Fri Jun 13 2003
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

#ifndef VISUALBUFFERSIGNAL_H
#define VISUALBUFFERSIGNAL_H

#include "visualbuffer.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class VisualBufferSignal : public VisualBuffer  {
public:
    VisualBufferSignal(ReaderExtract *pReaderExtract, EngineBuffer *pEngineBuffer, const char *group);
    ~VisualBufferSignal();
    virtual void update(int iPos, int iLen);
    virtual void draw(GLfloat *p, int iLen, float);

private:
    /** Used in low pass filtering */
    float m_fLastPositive, m_fLastNegative;

};

#endif
