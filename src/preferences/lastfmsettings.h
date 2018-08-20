#pragma once

#include <QObject>
#include <QCheckBox>

#include "preferences/usersettings.h"
#include "control/controlproxy.h"

namespace {
    const ConfigKey kLastFMEnabled("[Livemetadata]","LastFMEnabled");
    const ConfigKey kLastFMAuthenticated("[Livemetadata]","LastFMAuthenticated");
    const ConfigKey kLastFMSessionToken("[Livemetadata]","LastFMSessionToken");
    const ConfigKey kLastFMSettingsChanged("[Livemetadata]","LastFMSettingsChanged");
    const bool defaultLastFMEnabled = false;
    const bool defaultLastFMAuthenticated = false;
    const QString defaultLastFMSessionToken = "NoToken";
}

struct LastFMWidgets {
    QCheckBox *m_pEnabled;
};

struct LastFMSettings {
    bool enabled, authenticated;
    QString sessionToken;
};

class LastFMSettingsManager : public QObject {
    Q_OBJECT
  public:
    LastFMSettingsManager(UserSettingsPointer pSettings, const LastFMWidgets& widgets);
    static LastFMSettings getPersistedSettings(UserSettingsPointer pSettings);
    static LastFMSettings getLatestSettings();
    void applySettings();
    void cancelSettings();
    void setSettingsToDefault();
  private:
    void setUpWidgets();
    bool settingsDifferent();
    bool settingsCorrect();
    void updateLatestSettingsAndNotify();
    void persistSettings();
    void resetSettingsToDefault();

    LastFMWidgets m_widgets;
    UserSettingsPointer m_pUserSettings;
    static LastFMSettings s_latestSettings;
    ControlProxy m_CPSettingsChanged;

    bool m_authenticated;
    QString m_sessionToken;
};

