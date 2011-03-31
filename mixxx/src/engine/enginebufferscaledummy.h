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

#include "defs.h"
#include <QObject>
#include "engine/enginebufferscale.h"

class ReadAheadManager;

class EngineBufferScaleDummy : public EngineBufferScale
{
public:
    EngineBufferScaleDummy(ReadAheadManager* pReadAheadManager);
    ~EngineBufferScaleDummy();

    /** Set base tempo, ie. normal playback speed. */
    void setBaseRate(double dBaseRate);
    /** Set tempo */
    double setTempo(double dTempo);
    /** Get new playpos after call to scale() */
    double getNewPlaypos();
    /** Called from EngineBuffer when seeking, to ensure the buffers are flushed */
    void clear();
    /** Scale buffer */
    CSAMPLE* scale(double playpos,
                   unsigned long buf_size,
                   CSAMPLE* pBase,
                   unsigned long iBaseLength);

private:
    ReadAheadManager* m_pReadAheadManager;

};

#endif
