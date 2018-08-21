
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>
#include <QMessageBox>
#include <algorithm>
#include <QCryptographicHash>

#include "broadcast/networkaccessmanager.h"
#include "lastfmsettings.h"

LastFMSettings LastFMSettingsManager::s_latestSettings;

LastFMSettingsManager::LastFMSettingsManager(UserSettingsPointer pSettings,
                                             const LastFMWidgets &widgets,
                                             QWidget *parent)
        :  m_widgets(widgets),
           m_pUserSettings(pSettings),
           m_CPSettingsChanged(kLastFMSettingsChanged),
           m_networkRequest(rootApiUrl),
           m_pNetworkReply(nullptr),
           m_parent(parent) {
    s_latestSettings = getPersistedSettings(pSettings);
    m_authenticated = s_latestSettings.authenticated;
    m_sessionToken = s_latestSettings.sessionToken;
    setUpWidgets();
    connect(widgets.m_pEnabled,&QCheckBox::toggled,
            [this] (bool toggled){
                if (toggled && !s_latestSettings.authenticated) {
                    presentAuthorizationDialog();
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

void LastFMSettingsManager::presentAuthorizationDialog() {
    QString text = "Would you like to open a new tab in your browser"
                   " to authorize Mixxx to submit the metadata of the tracks"
                   " you listen to Last.FM?";
    QMessageBox question(m_parent);
    question.setInformativeText(text);
    question.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    int result = question.exec();
    if (result == QMessageBox::Yes) {
        requestAccessToken();
        text = "Have you authorized Mixxx?";
        question.setInformativeText(text);
        result = question.exec();
        if (result == QMessageBox::Yes) {
            requestSessionToken();
        }
        else {
            m_widgets.m_pEnabled->setChecked(false);
        }
    }
    else {
        m_widgets.m_pEnabled->setChecked(false);
    }
}

void LastFMSettingsManager::requestAccessToken() {
    QString method = "auth.getToken";
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("method",method);
    urlQuery.addQueryItem("api_key",apiKey);
    urlQuery.addQueryItem("api_sig", getSignature({qMakePair(QString("api_key"),apiKey),
                                                   qMakePair(QString("method"),method),
                                                   qMakePair(QString("format"),QString("json"))}));
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
        accessToken = json["token"].toString();
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


QString LastFMSettingsManager::getSignature(const QList<QPair<QString, QString>> &parameters) {
    QString joinedParams;
    QList<QPair<QString,QString>> sorted(parameters);
    std::sort(sorted.begin(),sorted.end());
    for (const QPair<QString,QString>& argument : sorted) {
        joinedParams += argument.first + argument.second;
    }
    joinedParams += apiSecret;
    QByteArray digest =
            QCryptographicHash::hash(joinedParams.toUtf8(),QCryptographicHash::Md5);
    qDebug() << "Signature digest: " << digest;
    return QString::fromUtf8(digest.toHex());
}

void LastFMSettingsManager::requestSessionToken() {
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("api_key",apiKey);
    urlQuery.addQueryItem("token",accessToken);
    urlQuery.addQueryItem("method","auth.getSession");
    urlQuery.addQueryItem("format","json");
    urlQuery.addQueryItem("api_sig",getSignature({qMakePair(QString("api_key"),apiKey),
                                                  qMakePair(QString("token"),accessToken),
                                                  qMakePair(QString("method"),QString("auth.getSession")),
                                                  qMakePair(QString("format"),QString("json"))}));
    QUrl url(rootApiUrl);
    url.setQuery(urlQuery);
    qDebug() << "Session token URL: " << url;
    m_networkRequest.setUrl(url);
    m_pNetworkReply = NetworkAccessManager::instance()->get(m_networkRequest);
    connect(m_pNetworkReply,&QNetworkReply::finished,
            [this]() {
        QByteArray response = m_pNetworkReply->readAll();
        qDebug() << "Session token response: " << response;
        QJsonObject json = QJsonDocument::fromJson(response).object();
        m_sessionToken = json["session"].toObject()["key"].toString();
        qDebug() << "Session token string: " << m_sessionToken;
        m_authenticated = true;
    });
}

