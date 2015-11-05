/***************************************************************************
                          enginevinylsoundemu.h  -  description
                             -------------------
    copyright            : (C) 2007 by Albert Santoni
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

#ifndef ENGINEVINYLSOUNDEMU_H
#define ENGINEVINYLSOUNDEMU_H

#include "util/types.h"
#include "util/defs.h"
#include "engine/engineobject.h"

class ControlObject;

#define NOISE_BUFFER_SIZE 1000

class EngineVinylSoundEmu : public EngineObject {
    Q_OBJECT
  public:
    EngineVinylSoundEmu(QString group);
    virtual ~EngineVinylSoundEmu();

    void setSpeed(double speed);
    void process(CSAMPLE* pInOut, const int iBufferSize);

  private:
    double m_dSpeed;
    double m_dOldSpeed;
    CSAMPLE m_fNoise[NOISE_BUFFER_SIZE];
    int m_iNoisePos;
    CSAMPLE m_crossfadeBuffer[MAX_BUFFER_LEN];
};

#endif
