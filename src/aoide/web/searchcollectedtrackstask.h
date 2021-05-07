#pragma once

#include <QStringList>
#include <QVector>

#include "aoide/json/track.h"
#include "aoide/tracksearchoverlayfilter.h"
#include "aoide/util.h"
#include "network/jsonwebtask.h"

namespace aoide {

class SearchCollectedTracksTask : public mixxx::network::JsonWebTask {
    Q_OBJECT

  public:
    SearchCollectedTracksTask(
            QNetworkAccessManager* networkAccessManager,
            QUrl baseUrl,
            const QString& collectionUid,
            const QJsonObject& baseQuery,
            const TrackSearchOverlayFilter& overlayFilter,
            const QStringList& searchTerms,
            const Pagination& pagination = {});
    ~SearchCollectedTracksTask() override = default;

  signals:
    void succeeded(
            const QJsonArray& searchResults);

  private:
    void onFinished(
            const mixxx::network::JsonWebResponse& jsonResponse) override;

    void emitSucceeded(
            const QJsonArray& searchResults);
};

} // namespace aoide
