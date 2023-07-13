#include "mixer/previewdeck.h"

#include "moc_previewdeck.cpp"

PreviewDeck::PreviewDeck(PlayerManager* pParent,
        UserSettingsPointer pConfig,
        EngineMaster* pMixingEngine,
        EffectsManager* pEffectsManager,
        EngineChannel::ChannelOrientation defaultOrientation,
        const ChannelHandleAndGroup& handleGroup)
        : BaseTrackPlayerImpl(pParent,
                  std::move(pConfig),
                  pMixingEngine,
                  pEffectsManager,
                  defaultOrientation,
                  handleGroup,
                  /*defaultMaster*/ false,
                  /*defaultHeadphones*/ true,
                  /*primaryDeck*/ false) {
}
