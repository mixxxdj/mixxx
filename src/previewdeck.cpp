#include "previewdeck.h"

PreviewDeck::PreviewDeck(QObject* pParent,
                         ConfigObject<ConfigValue> *pConfig,
                         EngineMaster* pMixingEngine,
                         EffectsManager* pEffectsManager,
                         EngineChannel::ChannelOrientation defaultOrientation,
                         QString group) :
        BaseTrackPlayer(pParent, pConfig, pMixingEngine, pEffectsManager,
                        defaultOrientation, group, false, true) {
}

PreviewDeck::~PreviewDeck() {
}
