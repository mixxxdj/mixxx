#include "mixer/sampler.h"

#include "control/controlobject.h"

Sampler::Sampler(QObject* pParent,
                 UserSettingsPointer pConfig,
                 std::shared_ptr<EngineMaster> pMixingEngine,
                 std::shared_ptr<EffectsManager> pEffectsManager,
                 EngineChannel::ChannelOrientation defaultOrientation,
                 QString group) :
        BaseTrackPlayerImpl(pParent, pConfig, pMixingEngine, pEffectsManager,
                            defaultOrientation, group, true, false) {
}

Sampler::~Sampler() {
}
