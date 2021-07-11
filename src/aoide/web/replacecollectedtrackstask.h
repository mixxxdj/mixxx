#pragma once

#include <QList>

#include "aoide/json/track.h"
#include "network/jsonwebtask.h"

namespace aoide {

class ReplaceCollectedTracksTask : public mixxx::network::JsonWebTask {
    Q_OBJECT

  public:
    ReplaceCollectedTracksTask(
            QNetworkAccessManager* networkAccessManager,
            QUrl baseUrl,
            const QString& collectionUid,
            const QList<json::Track>& tracks);
    ~ReplaceCollectedTracksTask() override = default;

    /// The number of track replacements
    int size() const {
        return m_size;
    }

  signals:
    void succeeded(
            const QJsonObject& result);

  private:
    void onFinished(
            const mixxx::network::JsonWebResponse& jsonResponse) override;

    void emitSucceeded(
            const QJsonObject& entity);

    const int m_size;
};

} // namespace aoide
