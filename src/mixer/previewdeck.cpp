#include "mixer/previewdeck.h"

#include "moc_previewdeck.cpp"

PreviewDeck::PreviewDeck(PlayerManager* pParent,
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
                  /*defaultMainMix*/ false,
                  /*defaultHeadphones*/ true,
                  /*primaryDeck*/ false) {
}
