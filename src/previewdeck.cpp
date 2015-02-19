#include "previewdeck.h"

PreviewDeck::PreviewDeck(QObject* pParent,
                         ConfigObject<ConfigValue> *pConfig,
                         EngineMaster* pMixingEngine,
                         EffectsManager* pEffectsManager,
                         EngineChannel::ChannelOrientation defaultOrientation,
                         const StringAtom& group) :
        BaseTrackPlayerImpl(pParent, pConfig, pMixingEngine, pEffectsManager,
                            defaultOrientation, group, false, true) {
}

PreviewDeck::~PreviewDeck() {
}
