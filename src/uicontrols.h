#pragma once
#include "control/controlpushbutton.h"

/// UI controls that can be use in controller mappings.
class UIControls {
  public:
    UIControls();

  private:
    ControlPushButton m_effectRack1Show;
    ControlPushButton m_libraryShowCoverArt;
    ControlPushButton m_microphoneShowMicrophone;
    ControlPushButton m_previewDeckShowPreviewDeck;
    ControlPushButton m_samplersShowSamplers;
    ControlPushButton m_skinShow4EffectUnits;
    ControlPushButton m_skinShowCoverArt;
    ControlPushButton m_skinShowMaximizedLibrary;
    ControlPushButton m_skinShowMixer;
    ControlPushButton m_skinShowSettings;
    ControlPushButton m_skinShowSpinnies;
    ControlPushButton m_vinylControlShowVinylControl;
};
