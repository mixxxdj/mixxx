#ifndef PREVIEWDECK_H
#define PREVIEWDECK_H

#include "basetrackplayer.h"

class PreviewDeck : public BaseTrackPlayer {
    Q_OBJECT
  public:
    PreviewDeck(QObject* pParent,
                ConfigObject<ConfigValue> *pConfig,
                EngineMaster* pMixingEngine,
                EngineChannel::ChannelOrientation defaultOrientation,
                QString group);
    virtual ~PreviewDeck();
};

#endif /* PREVIEWDECK_H */
