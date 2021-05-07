#pragma once

#include <QStringList>
#include <QVector>

#include "aoide/json/tag.h"
#include "aoide/util.h"
#include "network/jsonwebtask.h"
#include "util/optional.h"

namespace aoide {

// TODO: Currently not implemented in aoide
class ListTagsTask : public mixxx::network::JsonWebTask {
    Q_OBJECT

  public:
    ListTagsTask(
            QNetworkAccessManager* networkAccessManager,
            QUrl baseUrl,
            const QString& collectionUid,
            const std::optional<QStringList>& facets = std::nullopt,
            const Pagination& pagination = Pagination());
    ~ListTagsTask() override = default;

  signals:
    void succeeded(
            const QVector<aoide::json::TagCount>& result);

  private:
    void onFinished(
            const mixxx::network::JsonWebResponse& jsonResponse) override;

    void emitSucceeded(
            const QVector<json::TagCount>& result);
};

} // namespace aoide
