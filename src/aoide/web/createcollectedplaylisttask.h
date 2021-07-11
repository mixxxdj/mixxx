#pragma once

#include "aoide/json/playlist.h"
#include "network/jsonwebtask.h"

namespace aoide {

class CreateCollectedPlaylistTask : public mixxx::network::JsonWebTask {
    Q_OBJECT

  public:
    CreateCollectedPlaylistTask(
            QNetworkAccessManager* networkAccessManager,
            QUrl baseUrl,
            const QString& collectionUid,
            const json::Playlist& playlist);
    ~CreateCollectedPlaylistTask() override = default;

  signals:
    void succeeded(
            const aoide::json::PlaylistEntity& playlistEntity);

  private:
    void onFinished(
            const mixxx::network::JsonWebResponse& jsonResponse) override;

    void emitSucceeded(
            const json::PlaylistEntity& playlistEntity);
};

} // namespace aoide
