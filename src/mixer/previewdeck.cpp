#include "mixer/previewdeck.h"

PreviewDeck::PreviewDeck(QObject* pParent,
                         UserSettingsPointer pConfig,
                         EngineMaster* pMixingEngine,
                         EffectsManager* pEffectsManager,
                         VisualsManager* pVisualsManager,
                         EngineChannel::ChannelOrientation defaultOrientation,
                         QString group) :
        BaseTrackPlayerImpl(pParent, pConfig, pMixingEngine, pEffectsManager,
                pVisualsManager, defaultOrientation, group, false, true) {
}

PreviewDeck::~PreviewDeck() {
}
