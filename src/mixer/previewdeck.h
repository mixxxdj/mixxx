#ifndef MIXER_PREVIEWDECK_H
#define MIXER_PREVIEWDECK_H

#include "mixer/basetrackplayer.h"

class PreviewDeck : public BaseTrackPlayerImpl {
    Q_OBJECT
  public:
    PreviewDeck(QObject* pParent,
                ConfigObject<ConfigValue> *pConfig,
                EngineMaster* pMixingEngine,
                EffectsManager* pEffectsManager,
                EngineChannel::ChannelOrientation defaultOrientation,
                QString group);
    virtual ~PreviewDeck();
};

#endif /* MIXER_PREVIEWDECK_H */
