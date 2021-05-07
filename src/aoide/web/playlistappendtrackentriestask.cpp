#include "aoide/web/playlistappendtrackentriestask.h"

#include <QMetaMethod>

#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide PlaylistAppendTrackEntriesTask");

} // anonymous namespace

namespace aoide {

namespace {

mixxx::network::JsonWebRequest buildRequest(
        const json::EntityHeader& playlistEntityHeader,
        const QList<json::EntityUid>& trackUids) {
    json::EntityRevision rev = playlistEntityHeader.rev();
    QUrlQuery query;
    query.addQueryItem(
            QStringLiteral("rev"),
            QString::number(rev));
    QJsonArray entries;
    const auto addedAt = json::exportCurrentDateTime();
    for (const auto& trackUid : trackUids) {
        entries.append(QJsonObject{
                {QLatin1String("track"),
                        QJsonObject{
                                {
                                        QLatin1String("uid"),
                                        trackUid,
                                }}},
                {QLatin1String("addedAt"), addedAt}});
    }
    QJsonArray operations;
    operations.append(QJsonObject{
            {QLatin1String("append"),
                    QJsonObject{
                            {
                                    QLatin1String("entries"),
                                    entries,
                            }}}});
    return mixxx::network::JsonWebRequest{
            mixxx::network::HttpRequestMethod::Patch,
            QStringLiteral("/p/") +
                    playlistEntityHeader.uid().value() +
                    QStringLiteral("/entries/"),
            query,
            QJsonDocument(operations)};
}

} // anonymous namespace

PlaylistAppendTrackEntriesTask::PlaylistAppendTrackEntriesTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl,
        json::EntityHeader playlistEntityHeader,
        const QList<json::EntityUid>& trackUids)
        : mixxx::network::JsonWebTask(
                  networkAccessManager,
                  std::move(baseUrl),
                  buildRequest(playlistEntityHeader, trackUids)),
          m_playlistEntityHeader(std::move(playlistEntityHeader)) {
}

void PlaylistAppendTrackEntriesTask::onFinished(
        const mixxx::network::JsonWebResponse& jsonResponse) {
    if (!jsonResponse.isStatusCodeSuccess()) {
        kLogger.warning()
                << "Request failed with HTTP status code"
                << jsonResponse.statusCode();
        emitFailed(jsonResponse);
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(jsonResponse.statusCode() == mixxx::network::kHttpStatusCodeOk) {
        kLogger.warning()
                << "Unexpected HTTP status code"
                << jsonResponse.statusCode();
        emitFailed(jsonResponse);
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(jsonResponse.content().isArray()) {
        kLogger.warning()
                << "Invalid JSON content"
                << jsonResponse.content();
        emitFailed(jsonResponse);
        return;
    }
    emitSucceeded(
            json::PlaylistWithEntriesSummaryEntity(
                    jsonResponse.content().array()));
}

void PlaylistAppendTrackEntriesTask::emitSucceeded(
        const json::PlaylistWithEntriesSummaryEntity& playlistEntity) {
    const auto signal = QMetaMethod::fromSignal(
            &PlaylistAppendTrackEntriesTask::succeeded);
    DEBUG_ASSERT(receivers(signal.name()) <= 1); // unique connection
    if (isSignalConnected(signal)) {
        emit succeeded(
                std::move(playlistEntity));
    } else {
        deleteLater();
    }
}

} // namespace aoide
