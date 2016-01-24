#include "mixer/deck.h"

Deck::Deck(QObject* pParent,
           UserSettingsPointer pConfig,
           std::shared_ptr<EngineMaster> pMixingEngine,
           std::shared_ptr<EffectsManager> pEffectsManager,
           EngineChannel::ChannelOrientation defaultOrientation,
           const QString& group) :
        BaseTrackPlayerImpl(pParent, pConfig, pMixingEngine, pEffectsManager,
                            defaultOrientation, group, true, false) {
}

Deck::~Deck() {
}
