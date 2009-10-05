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
class EngineVuMeter;
class ControlPotmeter;
class ControlPushButton;
class ControlObject;
class EngineVinylSoundEmu;
class EngineSideChain;

class EngineMaster : public EngineObject {
public:
    EngineMaster(ConfigObject<ConfigValue>* pConfig, const char* pGroup);
    virtual ~EngineMaster();

    // Reconfigures the EngineBufferScaleSRC objects with the sound quality
    // written in the config database
    void setPitchIndpTimeStretch(bool b);

    // Get access to the sample buffers. None of these are thread safe. Only to
    // be called by SoundManager.
    const CSAMPLE* getMasterBuffer();
    const CSAMPLE* getHeadphoneBuffer();
    int numChannels();
    const CSAMPLE* getChannelBuffer(int i);

    void process(const CSAMPLE *, const CSAMPLE *pOut, const int iBufferSize);

    // Add an EngineChannel to the mixing engine. This is not thread safe --
    // only call it before the engine has started mixing.
    void addChannel(EngineChannel* pChannel);

  private:
    QList<EngineChannel*> m_channels;

    CSAMPLE *m_pMaster, *m_pHead;
    QList<CSAMPLE*> m_channelBuffers;

    EngineVolume *volume, *head_volume;
    EngineClipping *clipping, *head_clipping;
    EngineFlanger *flanger;
#ifdef __LADSPA__
    EngineLADSPA *ladspa;
#endif
    EngineVuMeter *vumeter;
    EngineSideChain *sidechain;

    ControlPotmeter *crossfader, *head_mix,
        *m_pBalance, *xFaderCurve, *xFaderCalibration;
    ControlPushButton *flanger1, *flanger2;
};

#endif
