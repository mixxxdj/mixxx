#pragma once

#include "preferences/usersettings.h"
#include "waveform/waveform.h"

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

    Waveform::Sampling waveformSamplingFunction() const {
        return m_pConfig->getValue(
                ConfigKey("[Waveform]", "waveform_sampling_function"), Waveform::Sampling::MAX);
    }

    void setWaveformSamplingFunction(Waveform::Sampling sampling) {
        m_pConfig->setValue(
                ConfigKey("[Waveform]", "waveform_sampling_function"), sampling);
    }

  private:
    UserSettingsPointer m_pConfig;
};
