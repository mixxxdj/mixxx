#pragma once

#include "mixer/basetrackplayer.h"

class Sampler : public BaseTrackPlayerImpl {
    Q_OBJECT
  public:
    Sampler(PlayerManager* pParent,
            UserSettingsPointer pConfig,
            EngineMixer* pMixingEngine,
            EffectsManager* pEffectsManager,
            EngineChannel::ChannelOrientation defaultOrientation,
            const ChannelHandleAndGroup& handleGroup);
    ~Sampler() override = default;
};
