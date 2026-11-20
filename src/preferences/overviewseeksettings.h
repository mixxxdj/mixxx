#pragma once

#include "preferences/usersettings.h"

/// Settings for seeking with the waveform overview of a playing deck.
/// Read by DlgPrefDeck and WOverview.
class OverviewSeekSettings {
  public:
    OverviewSeekSettings(UserSettingsPointer pConfig)
            : m_pConfig(pConfig) {
    }

    /// Keep the beat in the bar when clicking the overview of a playing deck
    DEFINE_PREFERENCE_HELPERS(KeepBarPhase, bool, "[Controls]", "OverviewSeekKeepBarPhase", true);

  private:
    UserSettingsPointer m_pConfig;
};
