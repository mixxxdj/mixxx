#include "mixer/previewdeck.h"

#include "moc_previewdeck.cpp"

PreviewDeck::PreviewDeck(QObject* pParent,
        UserSettingsPointer pConfig,
        EngineMaster* pMixingEngine,
        EffectsManager* pEffectsManager,
        VisualsManager* pVisualsManager,
        EngineChannel::ChannelOrientation defaultOrientation,
        const QString& group)
        : BaseTrackPlayerImpl(pParent, pConfig, pMixingEngine, pEffectsManager, pVisualsManager, defaultOrientation, group, /*defaultMaster*/ false,
                  /*defaultHeadphones*/ true,
                  /*primaryDeck*/ false) {
}
