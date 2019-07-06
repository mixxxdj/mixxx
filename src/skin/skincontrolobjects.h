//
// Created by Ferran Pujol Camins on 2019-07-06.
//

#pragma once

#include "control/controlobject.h"
#include "control/controlpushbutton.h"

// This class holds skin-related COs that outlive the currently loaded skin
class SkinControlObjects {
  public:
    SkinControlObjects()
            : m_coSkinLoaded(ConfigKey("[Skin]", "reloaded")),
            m_coFallbackCueColorId(ConfigKey("[Skin]", "fallback_cue_color_id")) {

    }
  private:
    ControlPushButton m_coSkinLoaded;
    ControlObject m_coFallbackCueColorId;
};
