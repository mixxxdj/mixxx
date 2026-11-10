#pragma once
#include "control/controlpushbutton.h"

/// Skin controls that can be use in controller mappings.
class SkinControls {
  public:
    SkinControls();

  private:
    ControlPushButton m_showEffectRack;
    ControlPushButton m_showLibraryCoverArt;
    ControlPushButton m_showMicrophones;
    ControlPushButton m_showPreviewDecks;
    ControlPushButton m_showSamplers;
    ControlPushButton m_show4EffectUnits;
    ControlPushButton m_showCoverArt;
    ControlPushButton m_showMaximizedLibrary;
    ControlPushButton m_showMixer;
    ControlPushButton m_showSettings;
    ControlPushButton m_showSpinnies;
    ControlPushButton m_showVinylControl;
    ControlPushButton m_hightlightInputAllow;
    ControlPushButton m_hightlightMixerChannel1;
    ControlPushButton m_hightlightMixerChannel2;
    ControlPushButton m_hightlightMixerChannel3;
    ControlPushButton m_hightlightMixerChannel4;
    ControlPushButton m_hightlightDeckChannel1;
    ControlPushButton m_hightlightDeckChannel2;
    ControlPushButton m_hightlightDeckChannel3;
    ControlPushButton m_hightlightDeckChannel4;
    ControlPushButton m_hightlightWaveformChannel1;
    ControlPushButton m_hightlightWaveformChannel2;
    ControlPushButton m_hightlightWaveformChannel3;
    ControlPushButton m_hightlightWaveformChannel4;
};
