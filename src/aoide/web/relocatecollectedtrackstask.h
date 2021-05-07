#pragma once

#include <QDir>
#include <QList>
#include <QPair>

#include "network/jsonwebtask.h"

namespace aoide {

// TODO: Currently not implemented in aoide
class RelocateCollectedTracksTask : public mixxx::network::JsonWebTask {
    Q_OBJECT

  public:
    RelocateCollectedTracksTask(
            QNetworkAccessManager* networkAccessManager,
            QUrl baseUrl,
            const QString& collectionUid,
            QList<QPair<QString, QString>> const& trackRelocations);
    RelocateCollectedTracksTask(
            QNetworkAccessManager* networkAccessManager,
            QUrl baseUrl,
            const QString& collectionUid,
            const QDir& oldRootDir,
            const QDir& newRootDir);
    ~RelocateCollectedTracksTask() override = default;

  signals:
    void succeeded();

  private:
    void onFinished(
            const mixxx::network::JsonWebResponse& jsonResponse) override;

    void emitSucceeded();
};

} // namespace aoide
