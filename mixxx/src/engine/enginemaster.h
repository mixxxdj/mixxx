/***************************************************************************
                          enginemaster.h  -  description
                             -------------------
    begin                : Sun Apr 28 2002
    copyright            : (C) 2002 by 
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

#ifndef ENGINEMASTER_H
#define ENGINEMASTER_H

#include "engineobject.h"

class EngineBuffer;
class EngineVolume;
class EngineChannel;
class EngineClipping;
class EngineFlanger;
#ifdef __LADSPA__
class EngineLADSPA;
#endif
#ifdef __FXUNITS__
class EngineEffectsUnits;
#endif
class EngineVuMeter;
class ControlPotmeter;
class ControlPushButton;
class ControlObject;
class EngineVinylSoundEmu;
class EngineSideChain;

// class EngineBufferMasterRate;

/**
  *@author Tue and Ken Haste Andersen
  */

class EngineMaster : public EngineObject
{
public:
    EngineMaster(ConfigObject<ConfigValue> *_config,
                 EngineBuffer *buffer1, EngineBuffer *buffer2,
                 EngineChannel *, EngineChannel *, const char *group);
    ~EngineMaster();
    /** Reconfigures the EngineBufferScaleSRC objects with the sound quality written in the config database */
    void setPitchIndpTimeStretch(bool b);
    const CSAMPLE* getMasterBuffer();
    const CSAMPLE* getHeadphoneBuffer();
    void process(const CSAMPLE *, const CSAMPLE *pOut, const int iBufferSize);
private:
	void xfadeGains(FLOAT_TYPE *gain1, FLOAT_TYPE *gain2, double xfadePosition, double transform, double calibration);

    EngineBuffer *buffer1, *buffer2;
    EngineChannel *channel1, *channel2;
    EngineVolume *volume, *head_volume; 
    EngineClipping *clipping, *head_clipping;
    EngineFlanger *flanger;
#ifdef __LADSPA__
    EngineLADSPA *ladspa;
#endif
#ifdef __FXUNITS__
    EngineEffectsUnits *fxunits;
#endif
    EngineVuMeter *vumeter;
    EngineVolume *volume1, *volume2;
    EngineVuMeter *vumeter1, *vumeter2;
    EngineVinylSoundEmu *vinylsound1, *vinylsound2;
    EngineSideChain *sidechain;

//     EngineBufferMasterRate *m_pEngineBufferMasterRate;
        
//     ControlObject *m_pControlObjectHeadphoneMute;
    ControlPotmeter *crossfader, *head_mix, *m_pBalance, *xFaderCurve, *xFaderCalibration;
    ControlPushButton *pfl1, *pfl2, *flanger1, *flanger2, *transform1, *transform2;
    CSAMPLE *m_pMaster, *m_pHead, *m_pTemp1, *m_pTemp2;
    bool master1, master2;
};

#endif
