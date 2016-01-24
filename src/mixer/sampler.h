#ifndef MIXER_SAMPLER_H
#define MIXER_SAMPLER_H

#include "effects/effectsmanager.h"
#include "engine/enginemaster.h"
#include "mixer/basetrackplayer.h"

class Sampler : public BaseTrackPlayerImpl {
    Q_OBJECT
  public:
    Sampler(QObject* pParent,
            UserSettingsPointer pConfig,
            std::shared_ptr<EngineMaster> pMixingEngine,
            std::shared_ptr<EffectsManager> pEffectsManager,
            EngineChannel::ChannelOrientation defaultOrientation,
            QString group);
    virtual ~Sampler();
};

#endif /* MIXER_SAMPLER_H */
