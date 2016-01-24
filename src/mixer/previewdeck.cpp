#include "mixer/previewdeck.h"

PreviewDeck::PreviewDeck(QObject* pParent,
                         UserSettingsPointer pConfig,
                         std::shared_ptr<EngineMaster> pMixingEngine,
                         std::shared_ptr<EffectsManager> pEffectsManager,
                         EngineChannel::ChannelOrientation defaultOrientation,
                         QString group) :
        BaseTrackPlayerImpl(pParent, pConfig, pMixingEngine, pEffectsManager,
                            defaultOrientation, group, false, true) {
}

PreviewDeck::~PreviewDeck() {
}
