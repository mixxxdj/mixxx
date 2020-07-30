#include "mixer/previewdeck.h"

PreviewDeck::PreviewDeck(QObject* pParent,
        UserSettingsPointer pConfig,
        EngineMaster* pMixingEngine,
        EffectsManager* pEffectsManager,
        VisualsManager* pVisualsManager,
        EngineChannel::ChannelOrientation defaultOrientation,
        const ChannelHandleAndGroup& handle_group)
        : BaseTrackPlayerImpl(pParent,
                  pConfig,
                  pMixingEngine,
                  pEffectsManager,
                  pVisualsManager,
                  defaultOrientation,
                  handle_group,
                  /*defaultMaster*/ false,
                  /*defaultHeadphones*/ true,
                  /*primaryDeck*/ false) {
}
