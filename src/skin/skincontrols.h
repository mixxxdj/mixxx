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
    ControlPushButton m_showPreparationWindow;
    ControlPushButton m_showMixer;
    ControlPushButton m_showSettings;
    ControlPushButton m_showSpinnies;
    ControlPushButton m_showVinylControl;
};
