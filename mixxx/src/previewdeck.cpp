#include "previewdeck.h"

PreviewDeck::PreviewDeck(QObject* pParent,
                         ConfigObject<ConfigValue> *pConfig,
                         EngineMaster* pMixingEngine,
                         EngineChannel::ChannelOrientation defaultOrientation,
                         QString group) :
        BaseTrackPlayer(pParent, pConfig, pMixingEngine, defaultOrientation,
                group, false, true) {
}

PreviewDeck::~PreviewDeck() {
}
