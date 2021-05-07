#pragma once

#include <QList>
#include <QMap>

#include "aoide/json/entity.h"
#include "network/jsonwebtask.h"

namespace aoide {

class ResolveCollectedTracksTask : public mixxx::network::JsonWebTask {
    Q_OBJECT

  public:
    ResolveCollectedTracksTask(
            QNetworkAccessManager* networkAccessManager,
            QUrl baseUrl,
            const QString& collectionUid,
            QList<QUrl> trackUrls);
    ~ResolveCollectedTracksTask() override = default;

  signals:
    void succeeded(
            const QMap<QUrl, json::EntityUid>& resolvedTrackUrls,
            const QList<QUrl>& unresolvedTrackUrls);

  private:
    void onFinished(
            const mixxx::network::JsonWebResponse& jsonResponse) override;

    void emitSucceeded(
            const QMap<QUrl, json::EntityUid>& resolvedTrackUrls,
            const QList<QUrl>& unresolvedTrackUrls);

    QList<QUrl> m_unresolvedTrackUrls;
};

} // namespace aoide
