
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>

#include "broadcast/networkaccessmanager.h"
#include "lastfmsettings.h"

LastFMSettings LastFMSettingsManager::s_latestSettings;

LastFMSettingsManager::LastFMSettingsManager(UserSettingsPointer pSettings,
                                             const LastFMWidgets& widgets)
        :  m_widgets(widgets),
           m_pUserSettings(pSettings),
           m_CPSettingsChanged(kLastFMSettingsChanged),
           apiKey("1047c802fb9bee75d12459f7cee02fea"),
           apiSecret("1e0fcb14744c596ff074ea404e7ecad2"),
           m_networkRequest(rootApiUrl),
           m_pNetworkReply(nullptr) {
    s_latestSettings = getPersistedSettings(pSettings);
    m_authenticated = s_latestSettings.authenticated;
    m_sessionToken = s_latestSettings.sessionToken;
    setUpWidgets();
    connect(widgets.m_pEnabled,&QCheckBox::toggled,
            [this] (bool toggled){
                if (toggled && !s_latestSettings.authenticated) {
                    requestAccessToken();
                }
            }
    );
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

void LastFMSettingsManager::requestAccessToken() {
    QString method = "auth.requestToken";
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("method",method);
    urlQuery.addQueryItem("api_key",apiKey);
    urlQuery.addQueryItem("api_sig",getSignature(method));
    urlQuery.addQueryItem("format","json");
    QUrl url(rootApiUrl);
    url.setQuery(urlQuery);
    qDebug() << "Access token url: " << url;
    m_networkRequest.setUrl(url);
    m_pNetworkReply = NetworkAccessManager::instance()->get(m_networkRequest);
    connect(m_pNetworkReply,&QNetworkReply::finished,
            [this]() {
        QByteArray response = m_pNetworkReply->readAll();
        qDebug() << "Access token response: " << response;
        QJsonObject json = QJsonDocument::fromJson(response).object();
        accessToken = json[QString("token")].toString();
        requestUserAuthorization();
    });
}

void LastFMSettingsManager::requestUserAuthorization() {
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("api_key",apiKey);
    urlQuery.addQueryItem("token",accessToken);
    QUrl url(authenticationUrl);
    url.setQuery(urlQuery);
    qDebug() << "User authorization url: " << url;
    QDesktopServices::openUrl(url);
}


QString LastFMSettingsManager::getSignature(const QString &method, const QString &token) {
    return QString();
}

