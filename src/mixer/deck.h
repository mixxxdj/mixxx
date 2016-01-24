#ifndef MIXER_DECK_H
#define MIXER_DECK_H

#include <QObject>

#include "effects/effectsmanager.h"
#include "engine/enginemaster.h"
#include "mixer/basetrackplayer.h"

class Deck : public BaseTrackPlayerImpl {
    Q_OBJECT
  public:
    Deck(QObject* pParent,
         UserSettingsPointer pConfig,
         std::shared_ptr<EngineMaster> pMixingEngine,
         std::shared_ptr<EffectsManager> pEffectsManager,
         EngineChannel::ChannelOrientation defaultOrientation,
         const QString& group);
    virtual ~Deck();
};

#endif // MIXER_DECK_H
