#include "lastfmsettings.h"

LastFMSettingsManager::LastFMSettingsManager(UserSettingsPointer pSettings,
                                             const LastFMWidgets& widgets)
        :  m_widgets(widgets),
           m_pUserSettings(pSettings),
           m_CPSettingsChanged(kLastFMSettingsChanged) {
    s_latestSettings = getPersistedSettings(pSettings);
    m_authenticated = s_latestSettings.authenticated;
    m_sessionToken = s_latestSettings.sessionToken;
    setUpWidgets();
}

LastFMSettings LastFMSettingsManager::getPersistedSettings(UserSettingsPointer pSettings) {
    LastFMSettings ret;
    ret.enabled = pSettings->getValue(kLastFMEnabled,defaultLastFMEnabled);
    ret.authenticated = pSettings->getValue(kLastFMAuthenticated,defaultLastFMAuthenticated);
    ret.sessionToken = pSettings->getValue(kLastFMSessionToken,defaultLastFMSessionToken);
    return ret;
}

void LastFMSettingsManager::setUpWidgets() {
    m_widgets.m_pEnabled->setChecked(s_latestSettings.enabled);
}

LastFMSettings LastFMSettingsManager::getLatestSettings() {
    return s_latestSettings;
}

void LastFMSettingsManager::applySettings() {
    if (settingsDifferent() && settingsCorrect()) {
        updateLatestSettingsAndNotify();
        persistSettings();
    }
}

bool LastFMSettingsManager::settingsDifferent() {
    return s_latestSettings.enabled != m_widgets.m_pEnabled->isChecked();
}

bool LastFMSettingsManager::settingsCorrect() {
    return true;
}

void LastFMSettingsManager::updateLatestSettingsAndNotify() {
    s_latestSettings.enabled = m_widgets.m_pEnabled->isChecked();
    s_latestSettings.authenticated = m_authenticated;
    s_latestSettings.sessionToken = m_sessionToken;
    m_CPSettingsChanged.set(true);
}

void LastFMSettingsManager::persistSettings() {
    m_pUserSettings->setValue(kLastFMEnabled,s_latestSettings.enabled);
    m_pUserSettings->setValue(kLastFMAuthenticated,s_latestSettings.authenticated);
    m_pUserSettings->setValue(kLastFMSessionToken,s_latestSettings.sessionToken);
}

void LastFMSettingsManager::cancelSettings() {
    setUpWidgets();
}

void LastFMSettingsManager::setSettingsToDefault() {
    resetSettingsToDefault();
    setUpWidgets();
}


void LastFMSettingsManager::resetSettingsToDefault() {
    s_latestSettings.enabled = defaultLastFMEnabled;
    s_latestSettings.authenticated = defaultLastFMAuthenticated;
    s_latestSettings.sessionToken = defaultLastFMSessionToken;
}
