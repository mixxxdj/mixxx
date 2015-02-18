#ifndef DECK_H
#define DECK_H

#include <QObject>

#include "basetrackplayer.h"

class Deck : public BaseTrackPlayerImpl {
    Q_OBJECT
  public:
    Deck(QObject* pParent,
         ConfigObject<ConfigValue>* pConfig,
         EngineMaster* pMixingEngine,
         EffectsManager* pEffectsManager,
         EngineChannel::ChannelOrientation defaultOrientation,
         const StringAtom& group);
    virtual ~Deck();
};

#endif // DECK_H
