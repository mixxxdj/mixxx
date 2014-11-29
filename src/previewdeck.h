#ifndef PREVIEWDECK_H
#define PREVIEWDECK_H

#include "basetrackplayer.h"

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

#endif /* PREVIEWDECK_H */
