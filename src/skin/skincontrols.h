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
    ControlPushButton m_showWaveforms;
    ControlPushButton m_showHotcues;
    ControlPushButton m_show8Hotcues;
    ControlPushButton m_showIntroOutroCues;
    ControlPushButton m_showLoopControls;
    ControlPushButton m_showBeatjumpControls;
    ControlPushButton m_showRateControls;
    ControlPushButton m_showRateControlButtons;
    ControlPushButton m_showKeyControls;
    ControlPushButton m_showEqKnobs;
    ControlPushButton m_showEqKillButtons;
    ControlPushButton m_showXfader;
    ControlPushButton m_showMainHeadMixer;
    ControlPushButton m_equal4deckWaveforms;
    ControlPushButton m_timingShiftButtons;
    ControlPushButton m_showSuperKnobs;
    ControlPushButton m_showSamplerFx;
};
