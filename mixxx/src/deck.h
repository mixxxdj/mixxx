#ifndef DECK_H
#define DECK_H

#include "basetrackplayer.h"

class Deck : public BaseTrackPlayer {
    Q_OBJECT
	public:
    Deck(QObject* pParent,
         ConfigObject<ConfigValue> *pConfig,
         EngineMaster* pMixingEngine,
         EngineChannel::ChannelOrientation defaultOrientation,
         AnalyserQueue* pAnalyserQueue,
         QString group);
    virtual ~Deck();
};

#endif
