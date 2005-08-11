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

#include "qapplication.h"
#include "hercules.h"
#include "controlobject.h"
#include "controleventmidi.h"
#include "midiobject.h"
#include "mathstuff.h"
#include "rotary.h"

Hercules::Hercules() : Input(), m_qRequestLed(5)
{
    m_bCueLeft = false;
    m_bCueRight = false;
    m_bPlayLeft = false;
    m_bPlayRight = false;
    m_bLoopLeft = false;
    m_bLoopRight = false;
    m_bMasterTempoLeft = false;
    m_bMasterTempoRight = false;
    m_bHeadphoneLeft = false;
    m_bHeadphoneRight = false;
    m_bSyncLeft = false;
    m_bSyncRight = false;
    m_iLeftFxMode = 0;
    m_iLeftFxMode = 0;
                 
    
    m_pControlObjectLeftBtnPlay = ControlObject::getControl(ConfigKey("[Channel1]","play"));
    m_pControlObjectRightBtnPlay = ControlObject::getControl(ConfigKey("[Channel2]","play"));
    m_pControlObjectLeftBtnPlayProxy = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","play")));
    m_pControlObjectRightBtnPlayProxy = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","play")));
    
    m_pControlObjectLeftBtnCue = ControlObject::getControl(ConfigKey("[Channel1]","loop"));
    m_pControlObjectRightBtnCue = ControlObject::getControl(ConfigKey("[Channel2]","loop"));
    m_pControlObjectLeftBtnLoopProxy = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","loop")));
    m_pControlObjectRightBtnLoopProxy = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","loop")));
    
    
    m_pControlObjectLeftBeatLoop = ControlObject::getControl(ConfigKey("[Channel1]","beatloop"));
    Q_ASSERT(m_pControlObjectLeftBeatLoop!=0);
    m_pControlObjectRightBeatLoop = ControlObject::getControl(ConfigKey("[Channel2]","beatloop"));
    Q_ASSERT(m_pControlObjectRightBeatLoop!=0);
    
    selectMapping(kqInputMappingHerculesStandard);
        
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
    delete m_pRotaryLeft;
    delete m_pRotaryRight;
}

QStringList Hercules::getMappings()
{
    QStringList mappings;
    mappings << kqInputMappingHerculesStandard << kqInputMappingHerculesInBeat;
    return mappings;
}

void Hercules::selectMapping(QString qMapping)
{
    m_qMapping = qMapping;

    if (qMapping==kqInputMappingHerculesInBeat)
    {
        m_pControlObjectLeftPitch = ControlObject::getControl(ConfigKey("[Channel1]","volume"));
        m_pControlObjectRightPitch = ControlObject::getControl(ConfigKey("[Channel2]","volume"));
        m_pControlObjectLeftBtnAutobeat = ControlObject::getControl(ConfigKey("[Channel1]","beatsync"));
        m_pControlObjectRightBtnAutobeat = ControlObject::getControl(ConfigKey("[Channel2]","beatsync"));
        m_pControlObjectLeftBtnPitchBendMinus = 0;
        m_pControlObjectRightBtnPitchBendMinus = 0;
        m_pControlObjectLeftBtnPitchBendPlus = 0;
        m_pControlObjectRightBtnPitchBendPlus = 0;
        m_pControlObjectLeftBtnMasterTempo = ControlObject::getControl(ConfigKey("[Channel1]","rate"));
        m_pControlObjectRightBtnMasterTempo = ControlObject::getControl(ConfigKey("[Channel2]","rate"));
        m_pControlObjectLeftVolume = ControlObject::getControl(ConfigKey("[Channel1]","pregain"));
        m_pControlObjectRightVolume = ControlObject::getControl(ConfigKey("[Channel2]","pregain"));
        m_pControlObjectCrossfade = ControlObject::getControl(ConfigKey("[Master]","rate"));
    
        changeJogMode(m_iLeftFxMode, m_iRightFxMode);

        m_pControlObjectLeftBeatLoop->queueFromThread(1.);
        m_pControlObjectRightBeatLoop->queueFromThread(1.);
    }
    else
    {
        m_pControlObjectLeftPitch = ControlObject::getControl(ConfigKey("[Channel1]","rate"));
        m_pControlObjectRightPitch = ControlObject::getControl(ConfigKey("[Channel2]","rate"));
        m_pControlObjectLeftBtnAutobeat = 0;
        m_pControlObjectRightBtnAutobeat = 0;
        m_pControlObjectLeftBtnPitchBendMinus = ControlObject::getControl(ConfigKey("[Channel1]","rate_perm_down_small"));
        m_pControlObjectRightBtnPitchBendMinus = ControlObject::getControl(ConfigKey("[Channel2]","rate_perm_down_small"));
        m_pControlObjectLeftBtnPitchBendPlus = ControlObject::getControl(ConfigKey("[Channel1]","rate_perm_up_small"));
        m_pControlObjectRightBtnPitchBendPlus = ControlObject::getControl(ConfigKey("[Channel2]","rate_perm_up_small"));
        m_pControlObjectLeftBtnMasterTempo = 0;
        m_pControlObjectRightBtnMasterTempo = 0;
        m_pControlObjectLeftVolume = ControlObject::getControl(ConfigKey("[Channel1]","volume"));
        m_pControlObjectRightVolume = ControlObject::getControl(ConfigKey("[Channel2]","volume"));
        m_pControlObjectCrossfade = ControlObject::getControl(ConfigKey("[Master]","crossfader"));
        
        changeJogMode();
    
        m_pControlObjectLeftBeatLoop->queueFromThread(0.);
        m_pControlObjectRightBeatLoop->queueFromThread(0.);
    }

    // Generic mappings
    m_pControlObjectLeftBtnTrackPrev = ControlObject::getControl(ConfigKey("[Channel1]","cue_set"));
    m_pControlObjectRightBtnTrackPrev = ControlObject::getControl(ConfigKey("[Channel2]","cue_set"));
    m_pControlObjectLeftBtnTrackNext = ControlObject::getControl(ConfigKey("[Channel1]","cue_preview"));
    m_pControlObjectRightBtnTrackNext = ControlObject::getControl(ConfigKey("[Channel2]","cue_preview"));
    m_pControlObjectLeftTreble = ControlObject::getControl(ConfigKey("[Channel1]","filterHigh"));
    m_pControlObjectRightTreble = ControlObject::getControl(ConfigKey("[Channel2]","filterHigh"));
    m_pControlObjectLeftMiddle = ControlObject::getControl(ConfigKey("[Channel1]","filterMid"));
    m_pControlObjectRightMiddle = ControlObject::getControl(ConfigKey("[Channel2]","filterMid"));
    m_pControlObjectLeftBass = ControlObject::getControl(ConfigKey("[Channel1]","filterLow"));
    m_pControlObjectRightBass = ControlObject::getControl(ConfigKey("[Channel2]","filterLow"));
    m_pControlObjectLeftBtnHeadphone = ControlObject::getControl(ConfigKey("[Channel1]","pfl"));
    m_pControlObjectRightBtnHeadphone = ControlObject::getControl(ConfigKey("[Channel2]","pfl"));
    
    m_pControlObjectLeftBtn1 = new ControlObject(ConfigKey("[Channel1]","Hercules1"));
    m_pControlObjectLeftBtn2 = new ControlObject(ConfigKey("[Channel1]","Hercules2"));
    m_pControlObjectLeftBtn3 = new ControlObject(ConfigKey("[Channel1]","Hercules3"));
    m_pControlObjectLeftBtnFx = new ControlObject(ConfigKey("[Channel1]","Hercules4"));
    
    m_pControlObjectRightBtn1 = new ControlObject(ConfigKey("[Channel2]","Hercules1"));
    m_pControlObjectRightBtn2 = new ControlObject(ConfigKey("[Channel2]","Hercules2"));
    m_pControlObjectRightBtn3 = new ControlObject(ConfigKey("[Channel2]","Hercules3"));
    m_pControlObjectRightBtnFx = new ControlObject(ConfigKey("[Channel2]","Hercules4"));
}

void Hercules::changeJogMode(int iLeftFxMode, int iRightFxMode)
{
    switch (iLeftFxMode)
    {
    case 0:
        m_pControlObjectLeftJog = ControlObject::getControl(ConfigKey("[Channel1]","wheel"));
        break;
    case 1:
        m_pControlObjectLeftJog = ControlObject::getControl(ConfigKey("[Channel1]","temporalShapeRate"));
        break;
    case 2:
        m_pControlObjectLeftJog = ControlObject::getControl(ConfigKey("[Channel1]","temporalPhaseRate"));
        break;
    }
    
    switch (iRightFxMode)
    {
    case 0:
        m_pControlObjectRightJog = ControlObject::getControl(ConfigKey("[Channel2]","wheel"));
        break;
    case 1:
        m_pControlObjectRightJog = ControlObject::getControl(ConfigKey("[Channel2]","temporalShapeRate"));
        break;
    case 2:
        m_pControlObjectRightJog = ControlObject::getControl(ConfigKey("[Channel2]","temporalPhaseRate"));
        break;
    }
}        


void Hercules::led()
{
    m_qRequestLed.tryAccess(1);
}
