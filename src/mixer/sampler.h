#ifndef MIXER_SAMPLER_H
#define MIXER_SAMPLER_H

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
            const QString& group);
    ~Sampler() override = default;
};

#endif /* MIXER_SAMPLER_H */
