/***************************************************************************
                          hercules.cpp  -  description
                             -------------------
    begin                : Tue Feb 22 2005
    copyright            : (C) 2005 by Tue Haste Andersen
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

#include "hercules.h"
#include "controlobject.h"
#include "controleventmidi.h"
#include "qapplication.h"
#include "midiobject.h"
#include "mathstuff.h"
#include "rotary.h"

Hercules::Hercules() : Input()
{
    m_pControlObjectLeftTreble = ControlObject::getControl(ConfigKey("[Channel1]","filterHigh"));
    m_pControlObjectLeftMiddle = ControlObject::getControl(ConfigKey("[Channel1]","filterMid"));
    m_pControlObjectLeftBass = ControlObject::getControl(ConfigKey("[Channel1]","filterLow"));
    m_pControlObjectLeftVolume = ControlObject::getControl(ConfigKey("[Channel1]","volume"));
    m_pControlObjectLeftPitch = ControlObject::getControl(ConfigKey("[Channel1]","rate"));
    m_pControlObjectLeftJog = ControlObject::getControl(ConfigKey("[Channel1]","wheel"));
    m_pControlObjectLeftBtnHeadphone = ControlObject::getControl(ConfigKey("[Channel1]","pfl"));
    m_pControlObjectLeftBtnPitchBendMinus = ControlObject::getControl(ConfigKey("[Channel1]","rate_perm_down_small"));
    m_pControlObjectLeftBtnPitchBendPlus = ControlObject::getControl(ConfigKey("[Channel1]","rate_perm_up_small"));
    m_pControlObjectLeftBtnTrackPrev = ControlObject::getControl(ConfigKey("[Channel1]","PrevTrack"));
    m_pControlObjectLeftBtnTrackNext = ControlObject::getControl(ConfigKey("[Channel1]","NextTrack"));
    m_pControlObjectLeftBtnCue = ControlObject::getControl(ConfigKey("[Channel1]","cue_goto"));
    m_pControlObjectLeftBtnPlay = ControlObject::getControl(ConfigKey("[Channel1]","play"));
    m_pControlObjectLeftBtnAutobeat = ControlObject::getControl(ConfigKey("[Channel1]","beatsync"));
    m_pControlObjectLeftBtnMasterTempo = new ControlObject(ConfigKey("[Channel1]","HerculesMaster"));
    m_pControlObjectLeftBtn1 = new ControlObject(ConfigKey("[Channel1]","Hercules1"));
    m_pControlObjectLeftBtn2 = new ControlObject(ConfigKey("[Channel1]","Hercules2"));
    m_pControlObjectLeftBtn3 = new ControlObject(ConfigKey("[Channel1]","Hercules3"));
    m_pControlObjectLeftBtnFx = new ControlObject(ConfigKey("[Channel1]","Hercules4"));
    
    m_pControlObjectRightTreble = ControlObject::getControl(ConfigKey("[Channel2]","filterHigh"));
    m_pControlObjectRightMiddle = ControlObject::getControl(ConfigKey("[Channel2]","filterMid"));
    m_pControlObjectRightBass = ControlObject::getControl(ConfigKey("[Channel2]","filterLow"));
    m_pControlObjectRightVolume = ControlObject::getControl(ConfigKey("[Channel2]","volume"));
    m_pControlObjectRightPitch = ControlObject::getControl(ConfigKey("[Channel2]","rate"));
    m_pControlObjectRightJog = ControlObject::getControl(ConfigKey("[Channel2]","wheel"));
    m_pControlObjectRightBtnHeadphone = ControlObject::getControl(ConfigKey("[Channel2]","pfl"));
    m_pControlObjectRightBtnPitchBendMinus = ControlObject::getControl(ConfigKey("[Channel2]","rate_perm_down_small"));
    m_pControlObjectRightBtnPitchBendPlus = ControlObject::getControl(ConfigKey("[Channel2]","rate_perm_up_small"));
    m_pControlObjectRightBtnTrackPrev = ControlObject::getControl(ConfigKey("[Channel2]","PrevTrack"));
    m_pControlObjectRightBtnTrackNext = ControlObject::getControl(ConfigKey("[Channel2]","NextTrack"));
    m_pControlObjectRightBtnCue = ControlObject::getControl(ConfigKey("[Channel2]","cue_goto"));
    m_pControlObjectRightBtnPlay = ControlObject::getControl(ConfigKey("[Channel2]","play"));
    m_pControlObjectRightBtnAutobeat = ControlObject::getControl(ConfigKey("[Channel2]","beatsync"));
    m_pControlObjectRightBtnMasterTempo = new ControlObject(ConfigKey("[Channel2]","HerculesMaster"));
    m_pControlObjectRightBtn1 = new ControlObject(ConfigKey("[Channel2]","Hercules1"));
    m_pControlObjectRightBtn2 = new ControlObject(ConfigKey("[Channel2]","Hercules2"));
    m_pControlObjectRightBtn3 = new ControlObject(ConfigKey("[Channel2]","Hercules3"));
    m_pControlObjectRightBtnFx = new ControlObject(ConfigKey("[Channel2]","Hercules4"));

    m_pControlObjectCrossfade = ControlObject::getControl(ConfigKey("[Master]","crossfader"));
    
    m_pRequestLed = new QSemaphore(5);
    
    m_pRotaryLeft = new Rotary();
    m_pRotaryRight = new Rotary();
}

Hercules::~Hercules()
{
    if (running())
    {
        terminate();
        wait();
    }
    delete m_pRequestLed;
    delete m_pRotaryLeft;
    delete m_pRotaryRight;
}

void Hercules::led()
{
    m_pRequestLed->tryAccess(1);
}
