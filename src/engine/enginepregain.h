/***************************************************************************
                          enginepregain.h  -  description
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

#ifndef ENGINEPREGAIN_H
#define ENGINEPREGAIN_H

#include "engine/engineobject.h"
#include "controlobject.h"
#include "util/performancetimer.h"

class ControlAudioTaperPot;
class ControlPotmeter;
class ControlObject;

class EnginePregain : public EngineObject {
  public:
    EnginePregain(QString group);
    virtual ~EnginePregain();

    void process(CSAMPLE* pInOut, const int iBufferSize);

  private:
    float m_fPrevGain;
    ControlAudioTaperPot* m_pPotmeterPregain;
    ControlObject* m_pTotalGain;
    ControlObject* m_pControlReplayGain;
    ControlObject* m_pPassthroughEnabled;
    static ControlPotmeter* s_pReplayGainBoost;
    static ControlObject* s_pEnableReplayGain;
    bool m_bSmoothFade;
    PerformanceTimer m_timer;
};

#endif
