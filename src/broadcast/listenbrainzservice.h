#pragma once

#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include "control/controlpushbutton.h"
#include "preferences/listenbrainzsettings.h"
#include "broadcast/scrobblingservice.h"
#include "broadcast/networkrequest.h"
#include "broadcast/networkreply.h"
#include "broadcast/networkmanager.h"

class NetworkRequest;

namespace {
    const QUrl ListenBrainzAPIURL("https://api.listenbrainz.org/1/submit-listens");
    const QUrl MockServerURL("http://localhost/cgi-bin/mixxxPostDummy.py");
}

class ListenBrainzService : public ScrobblingService {
    Q_OBJECT
  public:
    explicit ListenBrainzService(UserSettingsPointer pSettings);
  public slots:
    void slotBroadcastCurrentTrack(TrackPointer pTrack) override;
    void slotScrobbleTrack(TrackPointer pTrack) override;
    void slotAllTracksPaused() override;
  private slots:
    void slotAPICallFinished(QNetworkReply *reply);
    void slotSettingsChanged(double value);
  private:
    QNetworkRequest m_request;
    QNetworkAccessManager m_manager;
    ListenBrainzSettings m_latestSettings;
    ControlPushButton m_COSettingsChanged;
    QByteArray *m_pCurrentJSON;
};
