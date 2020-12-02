#ifndef MIXER_PREVIEWDECK_H
#define MIXER_PREVIEWDECK_H

#include <QByteArrayData>
#include <QString>

#include "engine/channels/enginechannel.h"
#include "mixer/basetrackplayer.h"
#include "preferences/usersettings.h"

class EffectsManager;
class EngineMaster;
class QObject;
class VisualsManager;

class PreviewDeck : public BaseTrackPlayerImpl {
    Q_OBJECT
  public:
    PreviewDeck(QObject* pParent,
            UserSettingsPointer pConfig,
            EngineMaster* pMixingEngine,
            EffectsManager* pEffectsManager,
            VisualsManager* pVisualsManager,
            EngineChannel::ChannelOrientation defaultOrientation,
            const QString& group);
    ~PreviewDeck() override = default;
};

#endif /* MIXER_PREVIEWDECK_H */
