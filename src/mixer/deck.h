#ifndef MIXER_DECK_H
#define MIXER_DECK_H

#include <QObject>

#include "mixer/basetrackplayer.h"

class Deck : public BaseTrackPlayerImpl {
    Q_OBJECT
  public:
    Deck(QObject* pParent,
         UserSettingsPointer pConfig,
         EngineMaster* pMixingEngine,
         EffectsManager* pEffectsManager,
         VisualsManager* pVisualsManager,
         EngineChannel::ChannelOrientation defaultOrientation,
         const QString& group);
    virtual ~Deck();
};

#endif // MIXER_DECK_H
