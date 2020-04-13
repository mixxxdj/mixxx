#pragma once

#include <QList>
#include <QMap>
#include <QSet>
#include <QUrlQuery>
#include <QUuid>

#include "musicbrainz/musicbrainz.h"
#include "network/webtask.h"

namespace mixxx {

class MusicBrainzRecordingsTask : public network::WebTask {
    Q_OBJECT

  public:
    MusicBrainzRecordingsTask(
            QNetworkAccessManager* networkAccessManager,
            QList<QUuid>&& recordingIds,
            QObject* parent = nullptr);
    ~MusicBrainzRecordingsTask() override;

  signals:
    void succeeded(
            QList<musicbrainz::TrackRelease> trackReleases);
    void failed(
            network::WebResponse response,
            int errorCode,
            QString errorMessage);

  private slots:
    void slotNetworkReplyFinished();

  private:
    bool doStart(
            QNetworkAccessManager* networkAccessManager,
            int parentTimeoutMillis) override;
    QUrl doAbort() override;
    QUrl doTimeOut() override;

    void emitSucceeded(
            QList<musicbrainz::TrackRelease>&& trackReleases);
    void emitFailed(
            network::WebResponse&& response,
            int errorCode,
            QString&& errorMessage);

    void continueWithNextRequest();

    const QUrlQuery m_urlQuery;

    QList<QUuid> m_queuedRecordingIds;
    QSet<QUuid> m_finishedRecordingIds;

    QMap<QUuid, musicbrainz::TrackRelease> m_trackReleases;

    QPointer<QNetworkReply> m_pendingNetworkReply;
    int m_parentTimeoutMillis;
};

} // namespace mixxx
