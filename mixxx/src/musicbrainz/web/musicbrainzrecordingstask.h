#pragma once

#include <QList>
#include <QMap>
#include <QSet>
#include <QUrlQuery>
#include <QUuid>

#include "musicbrainz/musicbrainz.h"
#include "network/webtask.h"
#include "util/performancetimer.h"

namespace mixxx {

class MusicBrainzRecordingsTask : public network::WebTask {
    Q_OBJECT

  public:
    MusicBrainzRecordingsTask(
            QNetworkAccessManager* networkAccessManager,
            const QList<QUuid>& recordingIds,
            QObject* parent = nullptr);
    ~MusicBrainzRecordingsTask() override = default;

  signals:
    void succeeded(
            const QList<mixxx::musicbrainz::TrackRelease>& trackReleases);
    void failed(
            const mixxx::network::WebResponse& response,
            int errorCode,
            const QString& errorMessage);
    void currentRecordingFetchedFromMusicBrainz();

  protected:
    void onNetworkError(
            QNetworkReply* finishedNetworkReply,
            network::HttpStatusCode statusCode) override;

  private:
    QNetworkReply* doStartNetworkRequest(
            QNetworkAccessManager* networkAccessManager,
            int parentTimeoutMillis) override;
    void doNetworkReplyFinished(
            QNetworkReply* pFinishedNetworkReply,
            network::HttpStatusCode statusCode) override;

    void emitSucceeded(
            const QList<musicbrainz::TrackRelease>& trackReleases);
    void emitFailed(
            const network::WebResponse& response,
            int errorCode,
            const QString& errorMessage);

    const QUrlQuery m_urlQuery;

    QList<QUuid> m_queuedRecordingIds;
    QSet<QUuid> m_finishedRecordingIds;

    QMap<QUuid, musicbrainz::TrackRelease> m_trackReleases;

    PerformanceTimer m_lastRequestSentAt;

    int m_parentTimeoutMillis;
};

} // namespace mixxx
