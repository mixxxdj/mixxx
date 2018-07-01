#pragma once

#include <QUrl>

#include "broadcast/scrobblingservice.h"
#include "broadcast/networkrequest.h"
#include "broadcast/networkreply.h"
#include "broadcast/networkmanager.h"

class NetworkRequest;

namespace {
    const QUrl ListenBrainzAPIURL("https://api.listenbrainz.org");
}

class ListenBrainzService : public ScrobblingService {
    Q_OBJECT
  public:
    explicit ListenBrainzService(NetworkManager *manager);
    ~ListenBrainzService() = default;
    void setNetworkRequest(NetworkRequest *pRequest);
  public slots:
    void slotBroadcastCurrentTrack(TrackPointer pTrack) override;
    void slotScrobbleTrack(TrackPointer pTrack) override;
    void slotAllTracksPaused() override;
  private slots:
    void slotAPICallFinished(NetworkReply *reply);
  private:
    std::unique_ptr<NetworkManager> m_pNetworkManager;
    std::unique_ptr<NetworkRequest> m_pRequest;
};
