#pragma once

#include "network/jsonwebtask.h"

namespace aoide {

class DeleteCollectionTask : public mixxx::network::JsonWebTask {
    Q_OBJECT

  public:
    DeleteCollectionTask(
            QNetworkAccessManager* networkAccessManager,
            QUrl baseUrl,
            QString collectionUid);
    ~DeleteCollectionTask() override = default;

  signals:
    void succeeded(
            const QString& collectionUid);

  private:
    void onFinished(
            const mixxx::network::JsonWebResponse& jsonResponse) override;

    void emitSucceeded();

    QString m_collectionUid;
};

} // namespace aoide
