#include "mixer/sampler.h"

#include "controlobject.h"

Sampler::Sampler(QObject* pParent,
                 UserSettingsPointer pConfig,
                 EngineMaster* pMixingEngine,
                 EffectsManager* pEffectsManager,
                 EngineChannel::ChannelOrientation defaultOrientation,
                 QString group) :
        BaseTrackPlayerImpl(pParent, pConfig, pMixingEngine, pEffectsManager,
                            defaultOrientation, group, true, false) {
}

Sampler::~Sampler() {
}
