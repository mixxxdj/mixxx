#pragma once

#include <QObject>

#include "mixer/basetrackplayer.h"

class Deck : public BaseTrackPlayerImpl {
    Q_OBJECT
  public:
    Deck(QObject* pParent,
            UserSettingsPointer pConfig,
            EngineMaster* pMixingEngine,
            EffectsManager* pEffectsManager,
            EngineChannel::ChannelOrientation defaultOrientation,
            const ChannelHandleAndGroup& handleGroup);
    ~Deck() override = default;
};
