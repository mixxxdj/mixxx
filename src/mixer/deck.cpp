#include "mixer/deck.h"

Deck::Deck(QObject* pParent,
           UserSettingsPointer pConfig,
           EngineMaster* pMixingEngine,
           EffectsManager* pEffectsManager,
           EngineChannel::ChannelOrientation defaultOrientation,
           const QString& group) :
        BaseTrackPlayerImpl(pParent, pConfig, pMixingEngine, pEffectsManager,
                            defaultOrientation, group, true, false) {
}

Deck::~Deck() {
}
