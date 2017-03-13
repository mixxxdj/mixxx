/***************************************************************************
                          enginebufferscale.h  -  description
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

#ifndef ENGINEBUFFERSCALEDUMMY_H
#define ENGINEBUFFERSCALEDUMMY_H

#include <QObject>

#include "util/types.h"
#include "engine/enginebufferscale.h"

class ReadAheadManager;

class EngineBufferScaleDummy : public EngineBufferScale {
  public:
    EngineBufferScaleDummy(ReadAheadManager* pReadAheadManager);
    virtual ~EngineBufferScaleDummy();

    /** Called from EngineBuffer when seeking, to ensure the buffers are flushed */
    void clear();
    /** Scale buffer */
    CSAMPLE* getScaled(unsigned long buf_size);

  private:
    ReadAheadManager* m_pReadAheadManager;
};

#endif
