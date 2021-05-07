#pragma once

#include <QDir>
#include <QStringList>

#include "network/jsonwebtask.h"

namespace aoide {

class PurgeTracksTask : public mixxx::network::JsonWebTask {
    Q_OBJECT

  public:
    PurgeTracksTask(
            QNetworkAccessManager* networkAccessManager,
            QUrl baseUrl,
            const QString& collectionUid,
            const QStringList& trackLocations);
    PurgeTracksTask(
            QNetworkAccessManager* networkAccessManager,
            QUrl baseUrl,
            const QString& collectionUid,
            const QDir& rootDir);
    ~PurgeTracksTask() override = default;

  signals:
    void succeeded();

  private:
    void onFinished(
            const mixxx::network::JsonWebResponse& jsonResponse) override;

    void emitSucceeded();
};

} // namespace aoide
