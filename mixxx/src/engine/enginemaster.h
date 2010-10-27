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

#include "engine/engineobject.h"
#include "engine/enginechannel.h"

class EngineWorkerScheduler;
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

    // Get access to the sample buffers. None of these are thread safe. Only to
    // be called by SoundManager.
    const CSAMPLE* getMasterBuffer() const;
    const CSAMPLE* getHeadphoneBuffer() const;
    int numChannels() const;
    const CSAMPLE* getChannelBuffer(unsigned int i) const;

    void process(const CSAMPLE *, const CSAMPLE *pOut, const int iBufferSize);

    // Add an EngineChannel to the mixing engine. This is not thread safe --
    // only call it before the engine has started mixing.
    void addChannel(EngineChannel* pChannel);

    static double gainForOrientation(EngineChannel::ChannelOrientation orientation,
                              double leftGain,
                              double centerGain,
                              double rightGain);

  private:
    QList<EngineChannel*> m_channels;

    CSAMPLE *m_pMaster, *m_pHead;
    QList<CSAMPLE*> m_channelBuffers;

    EngineWorkerScheduler *m_pWorkerScheduler;

    EngineVolume *volume, *head_volume;
    EngineClipping *clipping, *head_clipping;
#ifdef __LADSPA__
    EngineLADSPA *ladspa;
#endif
    EngineVuMeter *vumeter;
    EngineSideChain *sidechain;

    ControlPotmeter *crossfader, *head_mix,
        *m_pBalance, *xFaderCurve, *xFaderCalibration;
};

#endif
