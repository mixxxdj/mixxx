#pragma once

#include <QList>

#include "aoide/json/playlist.h"
#include "network/jsonwebtask.h"

namespace aoide {

class PlaylistAppendTrackEntriesTask : public mixxx::network::JsonWebTask {
    Q_OBJECT

  public:
    PlaylistAppendTrackEntriesTask(
            QNetworkAccessManager* networkAccessManager,
            QUrl baseUrl,
            json::EntityHeader playlistEntityHeader,
            const QList<json::EntityUid>& trackUids);
    ~PlaylistAppendTrackEntriesTask() override = default;

  signals:
    void succeeded(
            const aoide::json::PlaylistWithEntriesSummaryEntity& playlistEntity);

  private:
    void onFinished(
            const mixxx::network::JsonWebResponse& jsonResponse) override;

    void emitSucceeded(
            const json::PlaylistWithEntriesSummaryEntity& playlistEntity);

    json::EntityHeader m_playlistEntityHeader;
};

} // namespace aoide
