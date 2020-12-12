#include "mixer/sampler.h"

#include "control/controlobject.h"
#include "moc_sampler.cpp"

Sampler::Sampler(QObject* pParent,
        UserSettingsPointer pConfig,
        EngineMaster* pMixingEngine,
        EffectsManager* pEffectsManager,
        VisualsManager* pVisualsManager,
        EngineChannel::ChannelOrientation defaultOrientation,
        const QString& group)
        : BaseTrackPlayerImpl(pParent, pConfig, pMixingEngine, pEffectsManager, pVisualsManager, defaultOrientation, group, /*defaultMaster*/ true,
                  /*defaultHeadphones*/ false,
                  /*primaryDeck*/ false) {
}
