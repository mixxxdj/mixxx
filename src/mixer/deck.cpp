#include "mixer/deck.h"

Deck::Deck(QObject* pParent,
        UserSettingsPointer pConfig,
        EngineMaster* pMixingEngine,
        MacroRecorder* pMacroRecorder,
        EffectsManager* pEffectsManager,
        VisualsManager* pVisualsManager,
        EngineChannel::ChannelOrientation defaultOrientation,
        const ChannelHandleAndGroup& handleGroup)
        : BaseTrackPlayerImpl(pParent,
                  pConfig,
                  pMixingEngine,
                  pMacroRecorder,
                  pEffectsManager,
                  pVisualsManager,
                  defaultOrientation,
                  handleGroup,
                  /*defaultMaster*/ true,
                  /*defaultHeadphones*/ false,
                  /*primaryDeck*/ true) {
}
