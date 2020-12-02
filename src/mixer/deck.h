#ifndef MIXER_DECK_H
#define MIXER_DECK_H

#include <QByteArrayData>
#include <QObject>
#include <QString>

#include "engine/channels/enginechannel.h"
#include "mixer/basetrackplayer.h"
#include "preferences/usersettings.h"

class EffectsManager;
class EngineMaster;
class QObject;
class VisualsManager;

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
    ~Deck() override = default;
};

#endif // MIXER_DECK_H
