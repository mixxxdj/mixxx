/***************************************************************************
                          enginedelay.h  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
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

#ifndef ENGINEDELAY_H
#define ENGINEDELAY_H

#include "engine/engineobject.h"

class ControlPotmeter;

const int kiMaxDelay = 20000;

class EngineDelay : public EngineObject {
    Q_OBJECT
  public:
    EngineDelay(const char* group);
    virtual ~EngineDelay();

    void process(const CSAMPLE* pIn, CSAMPLE* pOut, const int iBufferSize);

  public slots:
    void slotDelayChanged(double);

  private:
    ControlPotmeter* m_pDelayPot;
    CSAMPLE* m_pDelayBuffer;
    int m_iDelayPos;
    int m_iDelay;
};

#endif
