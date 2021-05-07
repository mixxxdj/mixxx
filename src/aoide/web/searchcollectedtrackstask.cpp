#include "aoide/web/searchcollectedtrackstask.h"

#include <QMetaMethod>

#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide SearchCollectedTracksTask");

} // anonymous namespace

namespace aoide {

namespace {

mixxx::network::JsonWebRequest buildRequest(
        const QString& collectionUid,
        const QJsonObject& baseQuery,
        const TrackSearchOverlayFilter& overlayFilter,
        const QStringList& searchTerms,
        const Pagination& pagination) {
    DEBUG_ASSERT(baseQuery.isEmpty() ||
            baseQuery.value(QLatin1String("@type")).toString() == QStringLiteral("query"));

    QJsonObject searchParams;

    QJsonArray allFilters;
    auto baseFilter = baseQuery.value(QLatin1String("filter")).toObject();
    if (!baseFilter.isEmpty()) {
        allFilters += baseFilter;
    }
    if (overlayFilter.minBpm.isValid()) {
        allFilters += QJsonObject{{
                QLatin1String("numeric"),
                QJsonArray{
                        "musicTempoBpm",
                        QJsonObject{{
                                QLatin1String("ge"),
                                overlayFilter.minBpm.value(),
                        }},
                },
        }};
    }
    if (overlayFilter.maxBpm.isValid()) {
        allFilters += QJsonObject{{
                QLatin1String("numeric"),
                QJsonArray{
                        "musicTempoBpm",
                        QJsonObject{{
                                QLatin1String("le"),
                                overlayFilter.maxBpm.value(),
                        }},
                },
        }};
    }
    if (!overlayFilter.anyGenreLabels.isEmpty()) {
        QJsonArray genreFilters;
        for (const auto& genreLabel : overlayFilter.anyGenreLabels) {
            genreFilters += QJsonObject{{
                    QLatin1String("tag"),
                    QJsonObject{
                            {
                                    QLatin1String("facets"),
                                    QJsonArray{
                                            QLatin1String("genre"),
                                    },
                            },
                            {QLatin1String("label"),
                                    QJsonObject{
                                            {QLatin1String("matches"), genreLabel.value()},
                                    }},
                    },
            }};
        }
        allFilters += QJsonObject{{
                QLatin1String("any"),
                genreFilters,
        }};
    }
    if (!overlayFilter.allCommentTerms.isEmpty()) {
        for (const auto& commentTerm : overlayFilter.allCommentTerms) {
            allFilters += QJsonObject{{
                    QLatin1String("tag"),
                    QJsonObject{
                            {
                                    QLatin1String("facets"),
                                    QJsonArray{
                                            QLatin1String("comment"),
                                    },
                            },
                            {QLatin1String("label"),
                                    QJsonObject{
                                            {QLatin1String("contains"), commentTerm.value()},
                                    }},
                    },
            }};
        }
    }
    for (auto&& searchTerm : searchTerms) {
        if (searchTerm.isEmpty()) {
            continue;
        }
        QJsonArray anyFilters;
        // Search for term in all string fields
        anyFilters += QJsonObject{{
                QLatin1String("phrase"),
                QJsonArray{
                        QJsonArray{}, // any string field
                        QJsonArray{searchTerm},
                },
        }};
        // Search for term in all tag labels (both plain and faceted tags)
        anyFilters += QJsonObject{
                {
                        QLatin1String("tag"),
                        QJsonObject{
                                // no facets = any faceted or plain tag
                                {QLatin1String("label"),
                                        QJsonObject{
                                                {QLatin1String("contains"), searchTerm},
                                        }},
                        },
                }};
        allFilters += QJsonObject{{QLatin1String("any"), anyFilters}};
    }
    if (!allFilters.isEmpty()) {
        searchParams.insert(
                QLatin1String("filter"),
                QJsonObject{{QLatin1String("all"), allFilters}});
    }
    QString sort = baseQuery.value(QLatin1String("sort")).toString();
    QStringList sortFields = sort.split(
            QChar(','),
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            Qt::SkipEmptyParts);
#else
            QString::SkipEmptyParts);
#endif
    QJsonArray ordering;
    for (auto sortField : sortFields) {
        sortField = sortField.trimmed();
        QLatin1String direction;
        if (sortField.startsWith('+')) {
            direction = QLatin1String("asc");
            sortField = sortField.right(sortField.size() - 1).trimmed();
        } else if (sortField.startsWith('-')) {
            direction = QLatin1String("desc");
            sortField = sortField.right(sortField.size() - 1).trimmed();
        } else {
            kLogger.warning()
                    << "Missing direction for sort field"
                    << sortField;
        }
        auto sortOrder = QJsonArray{
                sortField,
                direction,
        };
        ordering += sortOrder;
    }
    if (!ordering.isEmpty()) {
        searchParams.insert(QLatin1String("ordering"), ordering);
    }

    QUrlQuery query;
    query.addQueryItem(
            QStringLiteral("resolveUrlFromPath"),
            QStringLiteral("true"));
    pagination.addToQuery(&query);
    return mixxx::network::JsonWebRequest{
            mixxx::network::HttpRequestMethod::Post,
            QStringLiteral("/c/") +
                    collectionUid +
                    QStringLiteral("/t/search"),
            query,
            QJsonDocument(searchParams)};
}

} // anonymous namespace

SearchCollectedTracksTask::SearchCollectedTracksTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl,
        const QString& collectionUid,
        const QJsonObject& baseQuery,
        const TrackSearchOverlayFilter& overlayFilter,
        const QStringList& searchTerms,
        const Pagination& pagination)
        : mixxx::network::JsonWebTask(
                  networkAccessManager,
                  std::move(baseUrl),
                  buildRequest(collectionUid, baseQuery, overlayFilter, searchTerms, pagination)) {
}

void SearchCollectedTracksTask::onFinished(
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
    const auto searchResults = jsonResponse.content().array();

    emitSucceeded(searchResults);
}

void SearchCollectedTracksTask::emitSucceeded(
        const QJsonArray& searchResults) {
    const auto signal = QMetaMethod::fromSignal(
            &SearchCollectedTracksTask::succeeded);
    DEBUG_ASSERT(receivers(signal.name()) <= 1); // unique connection
    if (isSignalConnected(signal)) {
        emit succeeded(searchResults);
    } else {
        deleteLater();
    }
}

} // namespace aoide
