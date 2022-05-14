#pragma once

#include <QObject>

#include "mixer/basetrackplayer.h"

class Stem : public BaseTrackPlayerImpl {
    Q_OBJECT
  public:
    Stem(PlayerManager* pParent,
            UserSettingsPointer pConfig,
            EngineMaster* pMixingEngine,
            EffectsManager* pEffectsManager,
            EngineChannel::ChannelOrientation defaultOrientation,
            const ChannelHandleAndGroup& handleGroup);
    ~Stem() override = default;
  public slots:
    void slotStemPlay(TrackPointer pTrack);

  private:
    QString stemName;
};
