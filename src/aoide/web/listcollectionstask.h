#pragma once

#include <QVector>

#include "aoide/json/collection.h"
#include "aoide/util.h"
#include "network/jsonwebtask.h"

namespace aoide {

class ListCollectionsTask : public mixxx::network::JsonWebTask {
    Q_OBJECT

  public:
    ListCollectionsTask(
            QNetworkAccessManager* networkAccessManager,
            QUrl baseUrl,
            const QString& kind = QString(),
            const Pagination& pagination = Pagination());
    ~ListCollectionsTask() override = default;

  signals:
    void succeeded(
            const QVector<aoide::json::CollectionEntity>& result);

  private:
    void onFinished(
            const mixxx::network::JsonWebResponse& jsonResponse) override;

    void emitSucceeded(
            const QVector<aoide::json::CollectionEntity>& result);
};

} // namespace aoide
