#include "sampler.h"

#include "controlobjectthreadmain.h"
#include "controlobject.h"

Sampler::Sampler(QObject* pParent,
                 ConfigObject<ConfigValue>* pConfig,
                 EngineMaster* pMixingEngine,
                 EffectsManager* pEffectsManager,
                 EngineChannel::ChannelOrientation defaultOrientation,
                 QString group) :
        BaseTrackPlayer(pParent, pConfig, pMixingEngine, pEffectsManager,
                        defaultOrientation, group, true, false) {
}

Sampler::~Sampler() {
}
