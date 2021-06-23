#pragma once

#include "preferences/usersettings.h"
#include "track/track_decl.h"

class ReplayGainSettings {
  public:
    ReplayGainSettings(UserSettingsPointer pConfig);

    int getInitialReplayGainBoost() const;
    void setInitialReplayGainBoost(int value);
    int getInitialDefaultBoost() const;
    void setInitialDefaultBoost(int value);
    bool getReplayGainEnabled() const;
    void setReplayGainEnabled(bool value);
    bool getReplayGainAnalyzerEnabled() const;
    void setReplayGainAnalyzerEnabled(bool value);
    int getReplayGainAnalyzerVersion() const;
    void setReplayGainAnalyzerVersion(int value);
    bool getReplayGainReanalyze() const;
    void setReplayGainReanalyze(bool value);

    bool isAnalyzerEnabled(int version) const;
    bool isAnalyzerDisabled(int version, TrackPointer tio) const;

  private:
    // Pointer to config object
    UserSettingsPointer m_pConfig;
};
