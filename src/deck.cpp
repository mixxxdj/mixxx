#include "deck.h"

Deck::Deck(QObject* pParent,
           ConfigObject<ConfigValue>* pConfig,
           EngineMaster* pMixingEngine,
           EffectsManager* pEffectsManager,
           EngineChannel::ChannelOrientation defaultOrientation,
           QString group) :
        BaseTrackPlayer(pParent, pConfig, pMixingEngine, pEffectsManager,
                        defaultOrientation, group, true, false) {
}

Deck::~Deck() {
}
