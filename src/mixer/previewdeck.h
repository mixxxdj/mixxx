#ifndef MIXER_PREVIEWDECK_H
#define MIXER_PREVIEWDECK_H

#include "effects/effectsmanager.h"
#include "engine/enginemaster.h"
#include "mixer/basetrackplayer.h"

class PreviewDeck : public BaseTrackPlayerImpl {
    Q_OBJECT
  public:
    PreviewDeck(QObject* pParent,
                UserSettingsPointer pConfig,
                std::shared_ptr<EngineMaster> pMixingEngine,
                std::shared_ptr<EffectsManager> pEffectsManager,
                EngineChannel::ChannelOrientation defaultOrientation,
                QString group);
    virtual ~PreviewDeck();
};

#endif /* MIXER_PREVIEWDECK_H */
