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
#include "controlobjectthread.h"
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

    m_pControlObjectLeftBtnPlay = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","play")));
    m_pControlObjectRightBtnPlay = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","play")));
    m_pControlObjectLeftBtnPlayProxy = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","play")));
    m_pControlObjectRightBtnPlayProxy = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","play")));

    m_pControlObjectLeftBtnCue = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","cue_cdj")));
    m_pControlObjectRightBtnCue = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","cue_cdj")));
    m_pControlObjectLeftBtnLoopProxy = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","loop")));
    m_pControlObjectRightBtnLoopProxy = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","loop")));


    m_pControlObjectLeftBeatLoop = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","beatloop")));
    Q_ASSERT(m_pControlObjectLeftBeatLoop!=0);
    m_pControlObjectRightBeatLoop = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","beatloop")));
    Q_ASSERT(m_pControlObjectRightBeatLoop!=0);

    m_pControlObjectLeftBtnHeadphoneProxy = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","pfl")));
    m_pControlObjectRightBtnHeadphoneProxy = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","pfl")));

    m_pControlObjectLeftVuMeter = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","VuMeter")));
    Q_ASSERT(m_pControlObjectLeftVuMeter != 0);
    m_pControlObjectRightVuMeter = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","VuMeter")));
    Q_ASSERT(m_pControlObjectRightVuMeter != 0);

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

    //FIXME: delete all the ControlObjectThreadMain objects!
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

    //FIXME: delete the ControlObjectThread objects before creating "new" ones.

    if (qMapping==kqInputMappingHerculesInBeat)
    {
        m_pControlObjectLeftPitch = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","volume")));
        m_pControlObjectRightPitch = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","volume")));
        m_pControlObjectLeftBtnAutobeat = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","beatsync")));
        m_pControlObjectRightBtnAutobeat = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","beatsync")));
        m_pControlObjectLeftBtnPitchBendMinus = 0;
        m_pControlObjectRightBtnPitchBendMinus = 0;
        m_pControlObjectLeftBtnPitchBendPlus = 0;
        m_pControlObjectRightBtnPitchBendPlus = 0;
        m_pControlObjectLeftBtnMasterTempo = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","rate")));
        m_pControlObjectRightBtnMasterTempo = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","rate")));
        m_pControlObjectLeftVolume = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","pregain")));
        m_pControlObjectRightVolume = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","pregain")));
        m_pControlObjectCrossfade = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Master]","rate")));
        changeJogMode(m_iLeftFxMode, m_iRightFxMode);

        m_pControlObjectLeftBeatLoop->slotSet(1.);
        m_pControlObjectRightBeatLoop->slotSet(1.);
    }
    else
    {
        m_pControlObjectLeftPitch = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","rate")));
        m_pControlObjectRightPitch = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","rate")));
        m_pControlObjectLeftBtnAutobeat = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","beatsync")));
        m_pControlObjectRightBtnAutobeat = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","beatsync")));
        m_pControlObjectLeftBtnPitchBendMinus = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","rate_perm_down_small")));
        m_pControlObjectRightBtnPitchBendMinus = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","rate_perm_down_small")));
        m_pControlObjectLeftBtnPitchBendPlus = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","rate_perm_up_small")));
        m_pControlObjectRightBtnPitchBendPlus = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","rate_perm_up_small")));
        m_pControlObjectLeftBtnMasterTempo = 0;
        m_pControlObjectRightBtnMasterTempo = 0;
        m_pControlObjectLeftVolume = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","volume")));
        m_pControlObjectRightVolume = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","volume")));
        m_pControlObjectCrossfade = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Master]","crossfader")));

        changeJogMode();

        m_pControlObjectLeftBeatLoop->slotSet(0.);
        m_pControlObjectRightBeatLoop->slotSet(0.);
    }

    // Generic mappings
    m_pControlObjectLeftBtnTrackPrev = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","PrevTrack")));
    m_pControlObjectRightBtnTrackPrev = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","PrevTrack")));
    m_pControlObjectLeftBtnTrackNext = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","NextTrack")));
    m_pControlObjectRightBtnTrackNext = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","NextTrack")));
    m_pControlObjectLeftTreble = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","filterHigh")));
    m_pControlObjectRightTreble = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","filterHigh")));
    m_pControlObjectLeftMiddle = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","filterMid")));
    m_pControlObjectRightMiddle = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","filterMid")));
    m_pControlObjectLeftBass = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","filterLow")));
    m_pControlObjectRightBass = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","filterLow")));
    m_pControlObjectLeftBtnHeadphone = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","pfl")));
    m_pControlObjectRightBtnHeadphone = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","pfl")));

    m_pControlObjectLeftBtn1 = new ControlObjectThread(new ControlObject(ConfigKey("[Channel1]","Hercules1")));
    m_pControlObjectLeftBtn2 = new ControlObjectThread(new ControlObject(ConfigKey("[Channel1]","Hercules2")));
    m_pControlObjectLeftBtn3 = new ControlObjectThread(new ControlObject(ConfigKey("[Channel1]","Hercules3")));
    m_pControlObjectLeftBtn123[0] = m_pControlObjectLeftBtn1;
    m_pControlObjectLeftBtn123[1] = m_pControlObjectLeftBtn2;
    m_pControlObjectLeftBtn123[2] = m_pControlObjectLeftBtn3;
    m_pControlObjectLeftBtnFx = new ControlObjectThread(new ControlObject(ConfigKey("[Channel1]","Hercules4")));

    m_pControlObjectRightBtn1 = new ControlObjectThread(new ControlObject(ConfigKey("[Channel2]","Hercules1")));
    m_pControlObjectRightBtn2 = new ControlObjectThread(new ControlObject(ConfigKey("[Channel2]","Hercules2")));
    m_pControlObjectRightBtn3 = new ControlObjectThread(new ControlObject(ConfigKey("[Channel2]","Hercules3")));
    m_pControlObjectRightBtn123[0] = m_pControlObjectRightBtn1;
    m_pControlObjectRightBtn123[1] = m_pControlObjectRightBtn2;
    m_pControlObjectRightBtn123[2] = m_pControlObjectRightBtn3;
    m_pControlObjectRightBtnFx = new ControlObjectThread(new ControlObject(ConfigKey("[Channel2]","Hercules4")));

    m_pControlObjectMainVolume = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Master]","volume")));
    m_pControlObjectBalance = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Master]","balance")));
    m_pControlObjectGainA = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","pregain")));
    m_pControlObjectGainB = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","pregain")));

    m_pControlObjectUp = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Playlist]","SelectPrevTrack")));
    m_pControlObjectDown = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Playlist]","SelectNextTrack")));
    m_pControlObjectLoadDeckA = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","LoadSelectedTrack")));
    m_pControlObjectLoadDeckB = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","LoadSelectedTrack")));

    m_pControlObjectLeftKillHigh = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","filterHighKill")));
    m_pControlObjectRightKillHigh = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","filterHighKill")));
    m_pControlObjectLeftKillMid = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","filterMidKill")));
    m_pControlObjectRightKillMid = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","filterMidKill")));
    m_pControlObjectLeftKillBass = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","filterLowKill")));
    m_pControlObjectRightKillBass = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","filterLowKill")));

    scratchMode = false;
    m_pControlObjectLeftScratch = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","scratch")));
    m_pControlObjectRightScratch = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","scratch")));
}

void Hercules::changeJogMode(int iLeftFxMode, int iRightFxMode)
{
    //FIXME: delete the ControlObjectThread objects before creating "new" ones.

    switch (iLeftFxMode)
    {
    case 0:
        m_pControlObjectLeftJog = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","wheel")));
        break;
    case 1:
        m_pControlObjectLeftJog = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","temporalShapeRate")));
        break;
    case 2:
        m_pControlObjectLeftJog = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","temporalPhaseRate")));
        break;
    }

    switch (iRightFxMode)
    {
    case 0:
        m_pControlObjectRightJog = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","wheel")));
        break;
    case 1:
        m_pControlObjectRightJog = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","temporalShapeRate")));
        break;
    case 2:
        m_pControlObjectRightJog = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","temporalPhaseRate")));
        break;
    }
}


void Hercules::led()
{
    m_qRequestLed.tryAccess(1);
}
