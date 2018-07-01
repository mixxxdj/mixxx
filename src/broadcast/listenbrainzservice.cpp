#include "broadcast/listenbrainzservice.h"

#include <QJsonObject>

#include "broadcast/listenbrainzjsonfactory.h"
#include "broadcast/networkmanager.h"
#include "moc_listenbrainzservice.cpp"

ListenBrainzService::ListenBrainzService(NetworkManager *manager)
        :  m_pNetworkManager(manager),
           m_pRequest(new QtNetworkRequest(ListenBrainzAPIURL)) {
       connect(m_pNetworkManager.get(),&NetworkManager::finished,
               this,&ListenBrainzService::slotAPICallFinished);
}

void ListenBrainzService::setNetworkRequest(NetworkRequest *pRequest) {
    m_pRequest.reset(pRequest);
}

void ListenBrainzService::slotBroadcastCurrentTrack(TrackPointer pTrack) {
    if (!pTrack)
        return;
    QByteArray object =
            ListenBrainzJSONFactory::getJSONFromTrack(
                    pTrack,ListenBrainzJSONFactory::NowListening);

    m_pNetworkManager->post(m_pRequest.get(),object);
}

void ListenBrainzService::slotScrobbleTrack(TrackPointer pTrack) {
}

void ListenBrainzService::slotAllTracksPaused() {
}

void ListenBrainzService::slotAPICallFinished(NetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "API call to ListenBrainz error: " << reply->getHttpError();
    }
}






