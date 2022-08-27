#pragma once

#include <QList>
#include <QMap>
#include <QSet>
#include <QUrlQuery>
#include <QUuid>

#include "musicbrainz/musicbrainz.h"
#include "network/webtask.h"
#include "util/timer.h"

namespace mixxx {

class MusicBrainzRecordingsTask : public network::WebTask {
    Q_OBJECT

  public:
    MusicBrainzRecordingsTask(
            QNetworkAccessManager* networkAccessManager,
            QList<QUuid>&& recordingIds,
            QObject* parent = nullptr);
    ~MusicBrainzRecordingsTask() override = default;

  signals:
    void succeeded(
            const QList<musicbrainz::TrackRelease>& trackReleases);
    void failed(
            const network::WebResponse& response,
            int errorCode,
            const QString& errorMessage);

  private:
    QNetworkReply* doStartNetworkRequest(
            QNetworkAccessManager* networkAccessManager,
            int parentTimeoutMillis) override;
    bool doNetworkReplyFinished(
            QNetworkReply* finishedNetworkReply,
            network::HttpStatusCode statusCode) override;

    void doWaitingTaskAborted() override;

    void emitSucceeded(
            const QList<musicbrainz::TrackRelease>& trackReleases);
    void emitFailed(
            const network::WebResponse& response,
            int errorCode,
            const QString& errorMessage);

    void triggerSlotStart();

    const QUrlQuery m_urlQuery;

    QList<QUuid> m_queuedRecordingIds;
    QSet<QUuid> m_finishedRecordingIds;

    QMap<QUuid, musicbrainz::TrackRelease> m_trackReleases;

    QTimer m_requestTimer;

    Timer m_measurementTimer;

    int m_parentTimeoutMillis;
};

} // namespace mixxx
