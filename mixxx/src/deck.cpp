#include "deck.h"
#include "engine/enginebuffer.h"
#include "engine/enginedeck.h"
#include "engine/enginemaster.h"

Deck::Deck(QObject* pParent,
           ConfigObject<ConfigValue>* pConfig,
           EngineMaster* pMixingEngine,
           EngineChannel::ChannelOrientation defaultOrientation,
           AnalyserQueue* pAnalyserQueue,
           QString group)
        : BaseTrackPlayer(pParent, pConfig, pMixingEngine, defaultOrientation, pAnalyserQueue, group) {

    const char* pSafeGroupName = strdup(getGroup().toAscii().constData());

    EngineDeck* pChannel = new EngineDeck(pSafeGroupName,
                                        pConfig, defaultOrientation);
    EngineBuffer* pEngineBuffer = pChannel->getEngineBuffer();
    pMixingEngine->addChannel(pChannel);
    initiate(pEngineBuffer,pSafeGroupName,pConfig);
}

Deck::~Deck()
{
}

