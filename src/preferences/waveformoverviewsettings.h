#pragma once

#include "preferences/usersettings.h"
#include "waveform/overviews/overviewtype.h"

namespace {
const ConfigKey kOverviewTypeCfgKey(
        QStringLiteral("[Waveform]"), QStringLiteral("WaveformOverviewType"));
const ConfigKey kOverviewNormalizedCfgKey(
        QStringLiteral("[Waveform]"), QStringLiteral("OverviewNormalized"));
} // namespace

class WaveformOverviewSettings {
  public:
    WaveformOverviewSettings(UserSettingsPointer pConfig)
            : m_pConfig(pConfig) {
    }

    mixxx::OverviewType getOverviewType() const {
        return m_pConfig->getValue<mixxx::OverviewType>(
                kOverviewTypeCfgKey, mixxx::OverviewType::RGB);
    }

    void setOverviewType(mixxx::OverviewType type) {
        m_pConfig->setValue(kOverviewTypeCfgKey, type);
    }

    bool isOverviewNormalized() const {
        return m_pConfig->getValue<bool>(kOverviewNormalizedCfgKey);
    }

    void setOverviewNormalized(bool normalize) {
        m_pConfig->setValue<bool>(kOverviewNormalizedCfgKey, normalize);
    }

  private:
    UserSettingsPointer m_pConfig;
};
