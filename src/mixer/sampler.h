#pragma once

#include "mixer/basetrackplayer.h"

class Sampler : public BaseTrackPlayerImpl {
    Q_OBJECT
  public:
    Sampler(PlayerManager* pParent,
            UserSettingsPointer pConfig,
            EngineMaster* pMixingEngine,
            EffectsManager* pEffectsManager,
            EngineChannel::ChannelOrientation defaultOrientation,
            GroupHandle groupHandle);
    ~Sampler() override = default;
};
