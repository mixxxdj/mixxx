#include "mixer/sampler.h"

#include "moc_sampler.cpp"

Sampler::Sampler(PlayerManager* pParent,
        UserSettingsPointer pConfig,
        EngineMixer* pMixingEngine,
        EffectsManager* pEffectsManager,
        EngineChannel::ChannelOrientation defaultOrientation,
        const ChannelHandleAndGroup& handleGroup)
        : BaseTrackPlayerImpl(pParent,
                  pConfig,
                  pMixingEngine,
                  pEffectsManager,
                  defaultOrientation,
                  handleGroup,
                  /*defaultMainMix*/ true,
                  /*defaultHeadphones*/ false,
                  /*primaryDeck*/ false) {
}
