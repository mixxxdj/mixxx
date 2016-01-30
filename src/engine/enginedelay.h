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
#include "preferences/usersettings.h"

class ControlPotmeter;
class ControlObjectSlave;

class EngineDelay : public EngineObject {
    Q_OBJECT
  public:
    EngineDelay(const char* group, ConfigKey delayControl);
    virtual ~EngineDelay();

    void process(CSAMPLE* pInOut, const int iBufferSize);

  public slots:
    void slotDelayChanged();

  private:
    ControlPotmeter* m_pDelayPot;
    ControlObjectSlave* m_pSampleRate;
    CSAMPLE* m_pDelayBuffer;
    int m_iDelayPos;
    int m_iDelay;
};

#endif
