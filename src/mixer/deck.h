#pragma once

#include <QObject>

#include "mixer/basetrackplayer.h"

class Deck : public BaseTrackPlayerImpl {
    Q_OBJECT
  public:
    Deck(QObject* pParent,
            UserSettingsPointer pConfig,
            EngineMaster* pMixingEngine,
            MacroRecorder* pMacroRecorder,
            EffectsManager* pEffectsManager,
            VisualsManager* pVisualsManager,
            EngineChannel::ChannelOrientation defaultOrientation,
            const QString& group);
    ~Deck() override = default;
};
