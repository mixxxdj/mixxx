#include "deck.h"

Deck::Deck(QObject* pParent,
           ConfigObject<ConfigValue>* pConfig,
           EngineMaster* pMixingEngine,
           EngineChannel::ChannelOrientation defaultOrientation,
           AnalyserQueue* pAnalyserQueue,
           QString group)
        : BaseTrackPlayer(pParent, pConfig, pMixingEngine, defaultOrientation, pAnalyserQueue, group) {

}

Deck::~Deck()
{
}

