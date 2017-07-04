#include "mixer/sampler.h"

#include "control/controlobject.h"

// Samplers are the same as decks except they cannot be routed to effects.
// Each level of the effects system (EffectProcessor, EngineEffect, and
// EngineEffectChain) maintain state for every input channel going to both
// the master and headphone outputs. Some effects require a buffer for every
// combination of input and output channel, which requires a lot of memory.
// Not allowing samplers to be routed to effects allows for using lots of
// samplers with minimal impact on memory use.
Sampler::Sampler(QObject* pParent,
                 UserSettingsPointer pConfig,
                 EngineMaster* pMixingEngine,
                 EffectsManager* pEffectsManager,
                 EngineChannel::ChannelOrientation defaultOrientation,
                 QString group) :
        BaseTrackPlayerImpl(pParent, pConfig, pMixingEngine, pEffectsManager,
                            defaultOrientation, group, true, false, false) {
}

Sampler::~Sampler() {
}
