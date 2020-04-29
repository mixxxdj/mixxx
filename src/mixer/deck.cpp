#include "mixer/deck.h"

Deck::Deck(QObject* pParent,
           UserSettingsPointer pConfig,
           EngineMaster* pMixingEngine,
           EffectsManager* pEffectsManager,
           VisualsManager* pVisualsManager,
           EngineChannel::ChannelOrientation defaultOrientation,
           const QString& group) :
        BaseTrackPlayerImpl(pParent, pConfig, pMixingEngine, pEffectsManager,
                pVisualsManager, defaultOrientation, group, true, false) {
}

Deck::~Deck() {
}
