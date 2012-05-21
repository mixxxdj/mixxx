#ifndef PREVIEWDECK_H
#define PREVIEWDECK_H

#include "basetrackplayer.h"

class PreviewDeck : public BaseTrackPlayer {
    Q_OBJECT
    public:
    //reimplement to use enginePreviewDeck instead of engineDeck
    PreviewDeck(QObject* pParent,
            ConfigObject<ConfigValue> *pConfig,
            EngineMaster* pMixingEngine,
            EngineChannel::ChannelOrientation defaultOrientation,
            QString group);
    virtual ~PreviewDeck();
};

#endif
