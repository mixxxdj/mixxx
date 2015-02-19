#include "sampler.h"

#include "controlobject.h"

Sampler::Sampler(QObject* pParent,
                 ConfigObject<ConfigValue>* pConfig,
                 EngineMaster* pMixingEngine,
                 EffectsManager* pEffectsManager,
                 EngineChannel::ChannelOrientation defaultOrientation,
                 const StringAtom& group) :
        BaseTrackPlayerImpl(pParent, pConfig, pMixingEngine, pEffectsManager,
                            defaultOrientation, group, true, false) {
}

Sampler::~Sampler() {
}
