#include "mixer/sampler.h"

#include "control/controlobject.h"

Sampler::Sampler(QObject* pParent,
                 UserSettingsPointer pConfig,
                 EngineMaster* pMixingEngine,
                 EffectsManager* pEffectsManager,
                 VisualsManager* pVisualsManager,
                 EngineChannel::ChannelOrientation defaultOrientation,
                 QString group) :
        BaseTrackPlayerImpl(pParent, pConfig, pMixingEngine, pEffectsManager,
                pVisualsManager, defaultOrientation, group, true, false) {
}

Sampler::~Sampler() {
}
