#pragma once

#include <QObject>

#include "mixer/basetrackplayer.h"

class Deck : public BaseTrackPlayerImpl {
    Q_OBJECT
  public:
    Deck(PlayerManager* pParent,
            UserSettingsPointer pConfig,
            EngineMaster* pMixingEngine,
            EffectsManager* pEffectsManager,
            EngineChannel::ChannelOrientation defaultOrientation,
            GroupHandle groupHandle);
    ~Deck() override = default;
};
