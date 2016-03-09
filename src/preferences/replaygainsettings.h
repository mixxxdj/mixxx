#ifndef PREFERENCES_REPLAYGAINSETTINGS_H
#define PREFERENCES_REPLAYGAINSETTINGS_H

#include "preferences/usersettings.h"

class ReplayGainSettings {
  public:
    ReplayGainSettings(UserSettingsPointer pConfig);

    int getInitialReplayGainBoost();
    void setInitialReplayGainBoost(int value);
    int getInitialDefaultBoost();
    void setInitialDefaultBoost(int value);
    bool getReplayGainEnabled();
    void setReplayGainEnabled(bool value);
    bool getReplayGainAnalyserEnabled();
    void setReplayGainAnalyserEnabled(bool value);
    int getReplayGainAnalyserVersion();
    void setReplayGainAnalyserVersion(int value);
    bool getReplayGainReanalyze();
    void setReplayGainReanalyze(bool value);

  private:
    // Pointer to config object
    UserSettingsPointer m_pConfig;
};

#endif /* PREFERENCES_REPLAYGAINSETTINGS_H */
