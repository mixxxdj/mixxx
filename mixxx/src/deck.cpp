#include "deck.h"

Deck::Deck(QObject* pParent,
           ConfigObject<ConfigValue>* pConfig,
           EngineMaster* pMixingEngine,
           EngineChannel::ChannelOrientation defaultOrientation,
           QString group)
        : BaseTrackPlayer(pParent, pConfig, pMixingEngine, defaultOrientation, group) {

}

Deck::~Deck()
{
}

