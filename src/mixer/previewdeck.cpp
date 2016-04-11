#include "mixer/previewdeck.h"

PreviewDeck::PreviewDeck(QObject* pParent,
                         UserSettingsPointer pConfig,
                         EngineMaster* pMixingEngine,
                         EffectsManager* pEffectsManager,
                         EngineChannel::ChannelOrientation defaultOrientation,
                         QString group) :
        BaseTrackPlayerImpl(pParent, pConfig, pMixingEngine, pEffectsManager,
                            defaultOrientation, group, false, true) {
}

PreviewDeck::~PreviewDeck() {
}
