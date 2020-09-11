#pragma once

#include "mixer/basetrackplayer.h"

class Sampler : public BaseTrackPlayerImpl {
    Q_OBJECT
  public:
    Sampler(QObject* pParent,
            UserSettingsPointer pConfig,
            EngineMaster* pMixingEngine,
            EffectsManager* pEffectsManager,
            VisualsManager* pVisualsManager,
            EngineChannel::ChannelOrientation defaultOrientation,
            const ChannelHandleAndGroup& handleGroup);
    ~Sampler() override = default;
};
