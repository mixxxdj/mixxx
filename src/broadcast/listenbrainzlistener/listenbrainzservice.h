#pragma once

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>

#include "broadcast/listenbrainzlistener/networkmanager.h"
#include "broadcast/listenbrainzlistener/networkreply.h"
#include "broadcast/listenbrainzlistener/networkrequest.h"
#include "broadcast/scrobblingservice.h"
#include "control/controlpushbutton.h"
#include "preferences/listenbrainzsettings.h"

class NetworkRequest;

/// Handles the Network kommunication with Listenbrainz
class ListenBrainzService : public ScrobblingService {
    Q_OBJECT
  public:
    explicit ListenBrainzService(UserSettingsPointer pSettings);
  public slots:
    void slotBroadcastCurrentTrack(TrackPointer pTrack) override;
    void slotScrobbleTrack(TrackPointer pTrack) override;
    void slotAllTracksPaused() override;
  private slots:
    void slotAPICallFinished(QNetworkReply* reply);
    void slotSettingsChanged(double value);

  private:
    QNetworkRequest m_request;
    QNetworkAccessManager m_manager;
    ListenBrainzSettings m_latestSettings;
    ControlPushButton m_COSettingsChanged;
    QByteArray m_currentJSON;
};
