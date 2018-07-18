#include "broadcast/listenbrainzlistener/listenbrainzservice.h"

#include <QJsonObject>

#include "broadcast/listenbrainzlistener/listenbrainzjsonfactory.h"
#include "broadcast/listenbrainzlistener/networkmanager.h"
#include "moc_listenbrainzservice.cpp"
#include "preferences/listenbrainzsettings.h"

ListenBrainzService::ListenBrainzService(UserSettingsPointer pSettings)
        : m_request(ListenBrainzAPIURL),
          m_latestSettings(ListenBrainzSettingsManager::getPersistedSettings(pSettings)),
          m_COSettingsChanged(kListenBrainzSettingsChanged) {
    connect(&m_manager, &QNetworkAccessManager::finished, this, &ListenBrainzService::slotAPICallFinished);
    connect(&m_COSettingsChanged, &ControlPushButton::valueChanged, this, &ListenBrainzService::slotSettingsChanged);
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (m_latestSettings.enabled) {
        m_request.setRawHeader("Authorization", "Token " + m_latestSettings.userToken.toUtf8());
    }
}

void ListenBrainzService::slotBroadcastCurrentTrack(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
    /*if (!pTrack || !m_latestSettings.enabled)
        return;
    m_currentJSON = new QByteArray(
            ListenBrainzJSONFactory::getJSONFromTrack(
                    pTrack,ListenBrainzJSONFactory::NowListening));
    m_manager.post(m_request,*m_currentJSON);*/
}

void ListenBrainzService::slotScrobbleTrack(TrackPointer pTrack) {
    if (!pTrack || !m_latestSettings.enabled)
        return;
    m_currentJSON =
            ListenBrainzJSONFactory::getJSONFromTrack(
                    pTrack, ListenBrainzJSONFactory::Single);
    m_manager.post(m_request, m_currentJSON);
}

void ListenBrainzService::slotAllTracksPaused() {
}

void ListenBrainzService::slotAPICallFinished(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "API call to ListenBrainz error: " << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    }
    m_currentJSON.clear();
}

void ListenBrainzService::slotSettingsChanged(double value) {
    if (value) {
        m_latestSettings = ListenBrainzSettingsManager::getLatestSettings();
        if (m_latestSettings.enabled) {
            m_request.setRawHeader("Authorization", "Token " + m_latestSettings.userToken.toUtf8());
        }
    }
}
