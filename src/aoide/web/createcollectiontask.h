#pragma once

#include "aoide/json/collection.h"
#include "network/jsonwebtask.h"

namespace aoide {

class CreateCollectionTask : public mixxx::network::JsonWebTask {
    Q_OBJECT

  public:
    CreateCollectionTask(
            QNetworkAccessManager* networkAccessManager,
            QUrl baseUrl,
            const json::Collection& collection);
    ~CreateCollectionTask() override = default;

  signals:
    void succeeded(
            const aoide::json::CollectionEntity& collectionEntity);

  private:
    void onFinished(
            const mixxx::network::JsonWebResponse& jsonResponse) override;

    void emitSucceeded(
            const json::CollectionEntity& collectionEntity);
};

} // namespace aoide
