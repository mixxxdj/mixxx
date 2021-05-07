#pragma once

#include <QStringList>
#include <QVector>

#include "aoide/json/tag.h"
#include "aoide/util.h"
#include "network/jsonwebtask.h"
#include "util/optional.h"

namespace aoide {

// TODO: Currently not implemented in aoide
class ListTagsFacetsTask : public mixxx::network::JsonWebTask {
    Q_OBJECT

  public:
    ListTagsFacetsTask(
            QNetworkAccessManager* networkAccessManager,
            QUrl baseUrl,
            const QString& collectionUid,
            const std::optional<QStringList>& facets = std::nullopt,
            const Pagination& pagination = Pagination());
    ~ListTagsFacetsTask() override = default;

  signals:
    void succeeded(
            const QVector<aoide::json::TagFacetCount>& result);

  private:
    void onFinished(
            const mixxx::network::JsonWebResponse& jsonResponse) override;

    void emitSucceeded(
            const QVector<aoide::json::TagFacetCount>& result);
};

} // namespace aoide
