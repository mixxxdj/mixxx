#include "broadcast/listenbrainzlistener/listenbrainzservice.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "broadcast/listenbrainzlistener/networkmanager.h"
#include "moc_listenbrainzservice.cpp"
#include "preferences/listenbrainzsettings.h"

namespace {

enum class JsonType {
    NowListening,
    Single
};

const QString kListenBrainzAPIURL = QStringLiteral("https://api.listenbrainz.org/1/submit-listens");
//const QString kMockServerURL = QStringLiteral("http://localhost/cgi-bin/mixxxPostDummy.py");

QByteArray getJSONFromTrack(TrackPointer pTrack, JsonType type) {
    QJsonObject jsonObject;
    QString stringType;
    if (type == JsonType::NowListening) {
        stringType = "playing_now";
    } else {
        stringType = "single";
    }

    QJsonArray payloadArray;
    QJsonObject payloadObject;
    QJsonObject metadataObject;
    QString title = pTrack->getTitle();
    QString artist = pTrack->getArtist();
    metadataObject.insert("artist_name", artist);
    metadataObject.insert("track_name", title);
    payloadObject.insert("track_metadata", metadataObject);
    qint64 timeStamp = QDateTime::currentMSecsSinceEpoch() / 1000;

    if (type == JsonType::Single) {
        payloadObject.insert("listened_at", timeStamp);
    }
    payloadArray.append(payloadObject);
    jsonObject.insert("listen_type", stringType);
    jsonObject.insert("payload", payloadArray);
    QJsonDocument doc(jsonObject);
    return doc.toJson(QJsonDocument::Compact);
}

} // namespace

ListenBrainzService::ListenBrainzService(UserSettingsPointer pSettings)
        : m_request(kListenBrainzAPIURL),
          m_latestSettings(ListenBrainzSettingsManager::getPersistedSettings(pSettings)),
          m_COSettingsChanged(ListenBrainzSettingsManager::kListenBrainzSettingsChanged) {
    connect(&m_manager,
            &QNetworkAccessManager::finished,
            this,
            &ListenBrainzService::slotAPICallFinished);
    connect(&m_COSettingsChanged,
            &ControlPushButton::valueChanged,
            this,
            &ListenBrainzService::slotSettingsChanged);
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
                    pTrack, ListenBrainzJSONFactory::NowListening));
    m_manager.post(m_request, *m_currentJSON);*/
}

void ListenBrainzService::slotScrobbleTrack(TrackPointer pTrack) {
    if (!pTrack || !m_latestSettings.enabled) {
        return;
    }
    m_currentJSON = getJSONFromTrack(pTrack, JsonType::Single);
    m_manager.post(m_request, m_currentJSON);
}

void ListenBrainzService::slotAllTracksPaused() {
}

void ListenBrainzService::slotAPICallFinished(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "API call to ListenBrainz error: "
                   << reply->attribute(
                              QNetworkRequest::HttpStatusCodeAttribute);
    }
    m_currentJSON.clear();
}

void ListenBrainzService::slotSettingsChanged(double value) {
    if (value >= 0) {
        m_latestSettings = ListenBrainzSettingsManager::getLatestSettings();
        if (m_latestSettings.enabled) {
            m_request.setRawHeader("Authorization", "Token " + m_latestSettings.userToken.toUtf8());
        }
    }
}
