/***************************************************************************
                          enginedeck.h  -  description
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

#ifndef ENGINEDECK_H
#define ENGINEDECK_H

#include "circularbuffer.h"
#include "controlpushbutton.h"
#include "engine/engineobject.h"
#include "engine/enginechannel.h"
#include "configobject.h"

#include "soundmanagerutil.h"

class EngineBuffer;
class EnginePregain;
class EngineBuffer;
class EngineFilterBlock;
class EngineClipping;
class EngineFlanger;
class EngineVuMeter;
class EngineVinylSoundEmu;
class ControlPushButton;

class EngineDeck : public EngineChannel, public AudioDestination {
    Q_OBJECT
  public:
    EngineDeck(const char *group, ConfigObject<ConfigValue>* pConfig,
               EngineChannel::ChannelOrientation defaultOrientation = CENTER);
    virtual ~EngineDeck();

    virtual void process(const CSAMPLE *pInput, const CSAMPLE *pOutput, const int iBufferSize);

    // TODO(XXX) This hack needs to be removed.
    virtual EngineBuffer* getEngineBuffer();

    virtual bool isActive();

    // Begin vinyl passthrough methods

    // This is called by SoundManager whenever there are new samples from the
    // deck to be processed
    virtual void receiveBuffer(AudioInput input, const short *pBuffer, unsigned int nFrames);

    // Called by SoundManager whenever the passthrough input is connected to a
    // soundcard input.
    virtual void onInputConnected(AudioInput input);

    // Called by SoundManager whenever the passthrough input is disconnected from
    // a soundcard input.
    virtual void onInputDisconnected(AudioInput input);

    // Return whether or not passthrough is active
    bool isPassthroughActive();

  public slots:
    void slotPassingToggle(double v);

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    EngineBuffer* m_pBuffer;
    EngineClipping* m_pClipping;
    EngineFilterBlock* m_pFilter;
    EngineFlanger* m_pFlanger;
    EnginePregain* m_pPregain;
    EngineVinylSoundEmu* m_pVinylSoundEmu;
    EngineVuMeter* m_pVUMeter;

    // Begin vinyl passthrough fields
    ControlPushButton* m_pPassing;
    CSAMPLE* m_pConversionBuffer;
    CircularBuffer<CSAMPLE> m_sampleBuffer;
    bool m_bPassthroughIsActive;
    bool m_bPassthroughWasActive;
};

#endif
