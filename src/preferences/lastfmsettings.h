#pragma once

#include <QObject>
#include <QCheckBox>
#include <QNetworkRequest>

#include "preferences/usersettings.h"
#include "control/controlproxy.h"

class QNetworkReply;

namespace {
    const ConfigKey kLastFMEnabled("[Livemetadata]","LastFMEnabled");
    const ConfigKey kLastFMAuthenticated("[Livemetadata]","LastFMAuthenticated");
    const ConfigKey kLastFMSessionToken("[Livemetadata]","LastFMSessionToken");
    const ConfigKey kLastFMSettingsChanged("[Livemetadata]","LastFMSettingsChanged");
    const bool defaultLastFMEnabled = false;
    const bool defaultLastFMAuthenticated = false;
    const QString defaultLastFMSessionToken = "NoToken";
    const QUrl rootApiUrl("http://ws.audioscrobbler.com/2.0/");
    const QUrl authenticationUrl("http://www.last.fm/api/auth/");
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

    void requestAccessToken();
    void requestUserAuthorization();

    QString getSignature(const QString& method,const QString& token = QString());

    LastFMWidgets m_widgets;
    UserSettingsPointer m_pUserSettings;
    static LastFMSettings s_latestSettings;
    ControlProxy m_CPSettingsChanged;
    QString apiKey, apiSecret, accessToken;
    QNetworkRequest m_networkRequest;
    QNetworkReply *m_pNetworkReply;

    bool m_authenticated;
    QString m_sessionToken;
};

