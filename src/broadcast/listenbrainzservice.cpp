#include "broadcast/listenbrainzservice.h"

#include <QJsonObject>

#include "preferences/listenbrainzsettings.h"
#include "broadcast/listenbrainzjsonfactory.h"
#include "broadcast/networkmanager.h"
#include "moc_listenbrainzservice.cpp"

ListenBrainzService::ListenBrainzService(UserSettingsPointer pSettings)
        :  m_request(ListenBrainzAPIURL),
           m_latestSettings(ListenBrainzSettingsManager::getPersistedSettings(pSettings)),
           m_COSettingsChanged(kListenBrainzSettingsChanged),
           m_pCurrentJSON(nullptr) {
    connect(&m_manager,&QNetworkAccessManager::finished,
            this,&ListenBrainzService::slotAPICallFinished);
    connect(&m_COSettingsChanged,&ControlPushButton::valueChanged,
            this,&ListenBrainzService::slotSettingsChanged);
    m_request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    if (m_latestSettings.enabled) {
        m_request.setRawHeader("Authorization","Token " + m_latestSettings.userToken.toUtf8());
    }
}


void ListenBrainzService::slotBroadcastCurrentTrack(TrackPointer pTrack) {
    if (!pTrack || !m_latestSettings.enabled)
        return;
    m_pCurrentJSON = new QByteArray(
            ListenBrainzJSONFactory::getJSONFromTrack(
                    pTrack,ListenBrainzJSONFactory::NowListening));
    m_manager.post(m_request,*m_pCurrentJSON);
}

void ListenBrainzService::slotScrobbleTrack(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
}

void ListenBrainzService::slotAllTracksPaused() {
}

void ListenBrainzService::slotAPICallFinished(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "API call to ListenBrainz error: " <<
                   reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    }
    delete m_pCurrentJSON;
    m_pCurrentJSON = nullptr;
}

void ListenBrainzService::slotSettingsChanged(double value) {
    if (value) {
        m_latestSettings = ListenBrainzSettingsManager::getLatestSettings();
        if (m_latestSettings.enabled) {
            m_request.setRawHeader("Authorization","Token " + m_latestSettings.userToken.toUtf8());
        }
    }
}






