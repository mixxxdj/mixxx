#include "mixer/previewdeck.h"

#include "moc_previewdeck.cpp"

PreviewDeck::PreviewDeck(QObject* pParent,
        UserSettingsPointer pConfig,
        EngineMaster* pMixingEngine,
        EffectsManager* pEffectsManager,
        VisualsManager* pVisualsManager,
        EngineChannel::ChannelOrientation defaultOrientation,
        const ChannelHandleAndGroup& handleGroup)
        : BaseTrackPlayerImpl(pParent,
                  pConfig,
                  pMixingEngine,
                  pEffectsManager,
                  pVisualsManager,
                  defaultOrientation,
                  handleGroup,
                  /*defaultMaster*/ false,
                  /*defaultHeadphones*/ true,
                  /*primaryDeck*/ false) {
}
