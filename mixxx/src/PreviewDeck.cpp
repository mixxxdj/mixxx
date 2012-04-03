#include "PreviewDeck.h"
#include "engine/enginebuffer.h"
#include "engine/enginePreviewDeck.h"
#include "engine/enginemaster.h"

PreviewDeck::PreviewDeck(QObject* pParent,
                 ConfigObject<ConfigValue>* pConfig,
                 EngineMaster* pMixingEngine,
                 EngineChannel::ChannelOrientation defaultOrientation,
                 QString group)
        : BaseTrackPlayer(pParent, pConfig, pMixingEngine, defaultOrientation, NULL, group) {

    const char* pSafeGroupName = strdup(getGroup().toAscii().constData());

    EnginePreviewDeck* pChannel = new EnginePreviewDeck(pSafeGroupName,
                                        pConfig, defaultOrientation);
    EngineBuffer* pEngineBuffer = pChannel->getEngineBuffer();
    pMixingEngine->addChannel(pChannel);
    initiate(pEngineBuffer,pSafeGroupName,pConfig);
}

PreviewDeck::~PreviewDeck()
{
}
