#include "controlobjectthread.h"
#include "looprecorderdeck.h"

LoopRecorderDeck::LoopRecorderDeck(QObject* pParent,
                         ConfigObject<ConfigValue> *pConfig,
                         EngineMaster* pMixingEngine,
                         EngineChannel::ChannelOrientation defaultOrientation,
                         QString group) :
        BaseTrackPlayer(pParent, pConfig, pMixingEngine, defaultOrientation,
                group, true, false) {

    m_pRepeat = new ControlObjectThread(group,"repeat");
    //m_pRepeat->slotSet(1.0);
}

LoopRecorderDeck::~LoopRecorderDeck() {
    delete m_pRepeat;
}
