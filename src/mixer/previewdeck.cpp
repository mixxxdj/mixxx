#include "mixer/previewdeck.h"

#include "moc_previewdeck.cpp"

PreviewDeck::PreviewDeck(PlayerManager* pParent,
        UserSettingsPointer pConfig,
        EngineMaster* pMixingEngine,
        EffectsManager* pEffectsManager,
        EngineChannel::ChannelOrientation defaultOrientation,
        GroupHandle groupHandle)
        : BaseTrackPlayerImpl(pParent,
                  pConfig,
                  pMixingEngine,
                  pEffectsManager,
                  defaultOrientation,
                  groupHandle,
                  /*defaultMaster*/ false,
                  /*defaultHeadphones*/ true,
                  /*primaryDeck*/ false) {
}
