#include "preferences/replaygainsettings.h"

#include "track/track.h"

namespace {
const char* kConfigKey = "[ReplayGain]";

const char* kInitialReplayGainBoost = "InitialReplayGainBoost";
const char* kInitialDefaultBoost = "InitialDefaultBoost";
// WARNING: Do not fix the "analyser" spelling here since user config files
// contain these strings.
const char* kReplayGainAnalyzerEnabled = "ReplayGainAnalyserEnabled";
const char* kReplayGainAnalyzerVersion = "ReplayGainAnalyserVersion";
const char* kReplayGainReanalyze = "ReplayGainReanalyze";

const char* kReplayGainEnabled = "ReplayGainEnabled";

const int kInitialDefaultBoostDefault = -6;
} // anonymous namespace

ReplayGainSettings::ReplayGainSettings(UserSettingsPointer pConfig)
    : m_pConfig(pConfig) {
}

int ReplayGainSettings::getInitialReplayGainBoost() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kInitialReplayGainBoost), 0);
}

void ReplayGainSettings::setInitialReplayGainBoost(int value) {
    m_pConfig->set(ConfigKey(kConfigKey, kInitialReplayGainBoost),
            ConfigValue(value));
}

int ReplayGainSettings::getInitialDefaultBoost() const {
    return m_pConfig->getValue(ConfigKey(kConfigKey, kInitialDefaultBoost),
            kInitialDefaultBoostDefault);
}

void ReplayGainSettings::setInitialDefaultBoost(int value) {
    m_pConfig->set(ConfigKey(kConfigKey, kInitialDefaultBoost),
                ConfigValue(value));
}

bool ReplayGainSettings::getReplayGainEnabled() const {
    return m_pConfig->getValue(
        ConfigKey(kConfigKey, kReplayGainEnabled), true);
}

void ReplayGainSettings::setReplayGainEnabled(bool value) {
    if (value) {
        m_pConfig->set(ConfigKey(kConfigKey, kReplayGainEnabled), ConfigValue(1));
    } else {
        m_pConfig->set(ConfigKey(kConfigKey, kReplayGainEnabled), ConfigValue(0));
    }
}

bool ReplayGainSettings::getReplayGainAnalyzerEnabled() const {
    return m_pConfig->getValue(
        ConfigKey(kConfigKey, kReplayGainAnalyzerEnabled), true);
}

void ReplayGainSettings::setReplayGainAnalyzerEnabled(bool value) {
    m_pConfig->set(ConfigKey(kConfigKey, kReplayGainAnalyzerEnabled),
                ConfigValue(value));
}

int ReplayGainSettings::getReplayGainAnalyzerVersion() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kReplayGainAnalyzerVersion), 2);
}

void ReplayGainSettings::setReplayGainAnalyzerVersion(int value) {
    m_pConfig->set(ConfigKey(kConfigKey, kReplayGainAnalyzerVersion),
            ConfigValue(value));
}

bool ReplayGainSettings::getReplayGainReanalyze() const {
    return m_pConfig->getValueString(
        ConfigKey(kConfigKey, kReplayGainReanalyze)).toInt() > 0;
}

void ReplayGainSettings::setReplayGainReanalyze(bool value) {
    m_pConfig->set(ConfigKey(kConfigKey, kReplayGainReanalyze),
                ConfigValue(value));
}

bool ReplayGainSettings::isAnalyzerEnabled(int version) const {
    return getReplayGainAnalyzerEnabled()
            && (version == getReplayGainAnalyzerVersion());
}

bool ReplayGainSettings::isAnalyzerDisabled(int version, TrackPointer tio) const {
    if (isAnalyzerEnabled(version)) {
        if (getReplayGainReanalyze()) {
            // ignore stored replay gain
            return false;
        }
        return tio->getReplayGain().hasRatio();
    }
    // not enabled, pretend we have already a stored value.
    return true;
}
