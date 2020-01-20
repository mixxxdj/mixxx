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

#include <QScopedPointer>

#include "preferences/usersettings.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "engine/engineobject.h"
#include "engine/channels/enginechannel.h"
#include "util/circularbuffer.h"

#include "soundio/soundmanagerutil.h"

class EngineBuffer;
class EnginePregain;
class EngineBuffer;
class EngineMaster;
class EngineVuMeter;
class EngineEffectsManager;
class ControlPushButton;

class EngineDeck : public EngineChannel, public AudioDestination {
    Q_OBJECT
  public:
    EngineDeck(const ChannelHandleAndGroup& handle_group,
            UserSettingsPointer pConfig,
            EngineMaster* pMixingEngine,
            EffectsManager* pEffectsManager,
            EngineChannel::ChannelOrientation defaultOrientation,
            bool primaryDeck);
    virtual ~EngineDeck();

    virtual void process(CSAMPLE* pOutput, const int iBufferSize);
    virtual void collectFeatures(GroupFeatureState* pGroupFeatures) const;
    virtual void postProcess(const int iBufferSize);

    // TODO(XXX) This hack needs to be removed.
    virtual EngineBuffer* getEngineBuffer();

    virtual bool isActive();

    // This is called by SoundManager whenever there are new samples from the
    // configured input to be processed. This is run in the callback thread of
    // the soundcard this AudioDestination was registered for! Beware, in the
    // case of multiple soundcards, this method is not re-entrant but it may be
    // concurrent with EngineMaster processing.
    virtual void receiveBuffer(const AudioInput& input,
            const CSAMPLE* pBuffer,
            unsigned int nFrames);

    // Called by SoundManager whenever the passthrough input is connected to a
    // soundcard input.
    virtual void onInputConfigured(const AudioInput& input);

    // Called by SoundManager whenever the passthrough input is disconnected
    // from a soundcard input.
    virtual void onInputUnconfigured(const AudioInput& input);

    // Return whether or not passthrough is active
    bool isPassthroughActive() const;

  signals:
    void noPassthroughInputConfigured();

  public slots:
    void slotPassingToggle(double v);
    void slotPassthroughChangeRequest(double v);

  private:
    UserSettingsPointer m_pConfig;
    EngineBuffer* m_pBuffer;
    EnginePregain* m_pPregain;

    // Begin vinyl passthrough fields
    QScopedPointer<ControlObject> m_pInputConfigured;
    ControlPushButton* m_pPassing;
    bool m_bPassthroughIsActive;
    bool m_bPassthroughWasActive;
    bool m_wasActive;
};

#endif
