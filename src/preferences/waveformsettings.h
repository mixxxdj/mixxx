#ifndef PREFERENCES_WAVEFORMSETTINGS_H
#define PREFERENCES_WAVEFORMSETTINGS_H

class WaveformSettings {
  public:
    WaveformSettings(UserSettingsPointer pConfig) : m_pConfig(pConfig) {}

    bool waveformCachingEnabled() const {
        return m_pConfig->getValue<bool>(
                ConfigKey("[Library]", "EnableWaveformCaching"), true);
    }

    void setWaveformCachingEnabled(bool enabled) {
        m_pConfig->setValue<bool>(
                ConfigKey("[Library]", "EnableWaveformCaching"), enabled);
    }

    bool waveformGenerationWithAnalysisEnabled() const {
        return m_pConfig->getValue<bool>(
                ConfigKey("[Library]", "EnableWaveformGenerationWithAnalysis"), true);
    }

    void setWaveformGenerationWithAnalysisEnabled(bool enabled) {
        m_pConfig->setValue<bool>(
                ConfigKey("[Library]", "EnableWaveformGenerationWithAnalysis"), enabled);
    }

  private:
    UserSettingsPointer m_pConfig;
};

#endif /* PREFERENCES_WAVEFORMSETTINGS_H */
