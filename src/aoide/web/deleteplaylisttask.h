#pragma once

#include "network/jsonwebtask.h"

namespace aoide {

class DeletePlaylistTask : public mixxx::network::JsonWebTask {
    Q_OBJECT

  public:
    DeletePlaylistTask(
            QNetworkAccessManager* networkAccessManager,
            QUrl baseUrl,
            QString playlistUid);
    ~DeletePlaylistTask() override = default;

  signals:
    void succeeded(
            const QString& playlistUid);

  private:
    void onFinished(
            const mixxx::network::JsonWebResponse& jsonResponse) override;

    void emitSucceeded();

    QString m_playlistUid;
};

} // namespace aoide
