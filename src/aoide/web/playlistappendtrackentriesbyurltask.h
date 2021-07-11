#pragma once

#include "aoide/web/playlistappendtrackentriestask.h"
#include "aoide/web/resolvecollectedtrackstask.h"
#include "network/networktask.h"

namespace aoide {

class PlaylistAppendTrackEntriesByUrlTask : public mixxx::network::NetworkTask {
    Q_OBJECT

  public:
    PlaylistAppendTrackEntriesByUrlTask(
            QNetworkAccessManager* networkAccessManager,
            QUrl baseUrl,
            QString collectionUid,
            json::EntityHeader playlistEntityHeader,
            QList<QUrl> trackUrls);
    ~PlaylistAppendTrackEntriesByUrlTask() override = default;

  public slots:
    void slotStart(
            int timeoutMillis) override;
    void slotAbort() override;

  signals:
    void failed(
            const mixxx::network::JsonWebResponse& response);
    void networkError(
            QNetworkReply::NetworkError errorCode,
            const QString& errorString,
            const mixxx::network::WebResponseWithContent& responseWithContent);
    void succeeded(
            const aoide::json::PlaylistWithEntriesSummaryEntity& playlistEntity,
            const QList<QUrl>& unresolvedTrackUrls);

  private slots:
    void slotResolveTracksByUrlSucceeded(
            const QMap<QUrl, json::EntityUid>& resolvedTrackUrls,
            const QList<QUrl>& unresolvedTrackUrls);

    void slotPlaylistAddTracksSucceeded(
            const json::PlaylistWithEntriesSummaryEntity& playlistEntity);

    void slotSubtaskAborted(
            const QUrl& requestUrl);
    void slotSubtaskNetworkError(
            QNetworkReply::NetworkError errorCode,
            const QString& errorString,
            const mixxx::network::WebResponseWithContent& responseWithContent);
    void slotSubtaskFailed(
            const mixxx::network::JsonWebResponse& response);

  private:
    void startResolveCollectedTracksTask();
    void startPlaylistAppendTrackEntriesTask(
            const QList<json::EntityUid>& trackUids);

    void emitSucceeded(
            const json::PlaylistWithEntriesSummaryEntity& playlistEntity);

    void emitNetworkError(
            QNetworkReply::NetworkError errorCode,
            const QString& errorString,
            const mixxx::network::WebResponseWithContent& responseWithContent);
    void emitFailed(
            const mixxx::network::JsonWebResponse& response);

    void reset();

    const QUrl m_baseUrl;
    const QString m_collectionUid;

    int m_timeoutMillis;

    QPointer<ResolveCollectedTracksTask> m_resolveTracksByUrlTask;
    QPointer<PlaylistAppendTrackEntriesTask> m_playlistAppendTrackEntriesTask;

    json::EntityHeader m_playlistEntityHeader;
    QList<QUrl> m_unresolvedTrackUrls;
};

} // namespace aoide
