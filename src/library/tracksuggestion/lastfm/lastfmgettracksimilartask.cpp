#include "library/tracksuggestion/lastfm/lastfmgettracksimilartask.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QString>

#include "util/assert.h"
#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("LastfmGetTrackSimilarTask");

// API account details
// Website:             https://www.last.fm/api
// Application name: 	Mixxx
// Description:         Mixxx is free and open-source software for DJing.
// API key: 	        7830e11363f1a74c9cdb126aae9d84af
// Shared secret: 	51671e36013c555ed91e8bb5080f50f3
// Registered to: 	fatihemreyildiz
// Created:             Saturday 11 Jun 2022, 11:29am

// About this method
// This method is called track.Getsimilar.
// Based on all the users listening data
// this method gets the similar tracks for a track on Last.fm.
// It doesn't require any authentication for to use.
// More info can be found: https://www.last.fm/api/show/track.getSimilar

const QString kApiKey = QStringLiteral("7830e11363f1a74c9cdb126aae9d84af");

const QString kSharedSecretKey = QStringLiteral("51671e36013c555ed91e8bb5080f50f3");

const QUrl kBaseUrl = QStringLiteral("https://ws.audioscrobbler.com/");

const QString kRequestPath = QStringLiteral("/2.0/");

QUrlQuery lookupUrlQuery(
        const QString& artist,
        const QString& track) {
    QUrlQuery urlQuery;
    urlQuery.addQueryItem(
            QStringLiteral("method"),
            QStringLiteral("track.getsimilar"));
    urlQuery.addQueryItem(
            QStringLiteral("artist"),
            artist);
    urlQuery.addQueryItem(
            QStringLiteral("track"),
            track);
    urlQuery.addQueryItem(
            QStringLiteral("autocorrect"),
            QString::number(1));
    urlQuery.addQueryItem(
            QString("api_key"),
            kApiKey);
    urlQuery.addQueryItem(
            QStringLiteral("format"),
            QStringLiteral("json"));
    //QUESTION: Should limit changed via preference?
    //urlQuery.addQueryItem(
    //        QStringLiteral("limit"),
    //        QString::number(5));

    return urlQuery;
}

QNetworkRequest createNetworkRequest(const QUrlQuery& urlQuery) {
    DEBUG_ASSERT(kBaseUrl.isValid());
    QUrl url = kBaseUrl;
    url.setPath(kRequestPath);
    url.setQuery(urlQuery);
    DEBUG_ASSERT(url.isValid());
    QNetworkRequest networkRequest(url);
    return networkRequest;
}

network::JsonWebRequest lookupRequest() {
    return network::JsonWebRequest{
            network::HttpRequestMethod::Get,
            kRequestPath,
            QUrlQuery(),
            QJsonDocument(),
    };
}

} // anonymous namespace

LastfmGetTrackSimilarTask::LastfmGetTrackSimilarTask(
        QNetworkAccessManager* networkAccessManager,
        const QString& artist,
        const QString& track,
        QObject* parent)
        : network::JsonWebTask(
                  networkAccessManager,
                  kBaseUrl,
                  lookupRequest(),
                  parent),
          m_urlQuery(lookupUrlQuery(artist, track)) {
}

QNetworkReply* LastfmGetTrackSimilarTask::sendNetworkRequest(
        QNetworkAccessManager* networkAccessManager,
        network::HttpRequestMethod method,
        const QUrl& url,
        const QJsonDocument& content) {
    Q_UNUSED(method);
    DEBUG_ASSERT(method == network::HttpRequestMethod::Get);
    Q_UNUSED(content);
    DEBUG_ASSERT(content.isEmpty());
    DEBUG_ASSERT(url.isValid());
    DEBUG_ASSERT(!m_urlQuery.isEmpty());

    const QNetworkRequest networkRequest = createNetworkRequest(m_urlQuery);

    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "GET"
                << url;
    }

    DEBUG_ASSERT(networkAccessManager);
    return networkAccessManager->get(networkRequest);
}

void LastfmGetTrackSimilarTask::onFinished(
        const network::JsonWebResponse& response) {
    if (!response.isStatusCodeSuccess()) {
        kLogger.warning()
                << "Request failed with HTTP status code"
                << response.statusCode();
        emitFailed(response);
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(response.statusCode() == network::kHttpStatusCodeOk) {
        kLogger.warning()
                << "Unexpected HTTP status code"
                << response.statusCode();
        emitFailed(response);
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(response.content().isObject()) {
        kLogger.warning()
                << "Invalid JSON content"
                << response.content();
        emitFailed(response);
        return;
    }
    const auto jsonObject = response.content().object();

    if (jsonObject.value(QStringLiteral("similartracks")).isNull()) {
        kLogger.warning()
                << "Similar tracks not found";
        emitFailed(response);
        return;
    }

    //DEBUG_ASSERT(jsonObject.value(QLatin1String("similartracks")).isObject());

    const auto similarTracksObject = jsonObject.value(QLatin1String("similartracks")).toObject();

    const QJsonArray tracks = similarTracksObject.value(QLatin1String("track")).toArray();

    QList<QMap<QString, QString>> results;

    for (const auto& track : tracks) {
        results.append(QMap<QString, QString>{});
        DEBUG_ASSERT(track.isObject());
        const auto trackObject = track.toObject();
        const auto trackName = trackObject.value(QLatin1String("name")).toString();
        const auto trackPlaycount = trackObject.value(QLatin1String("playcount")).toInt();
        const auto trackMBID = trackObject.value(QLatin1String("mbid")).toString();
        const auto trackMatch = trackObject.value(QLatin1String("match")).toDouble();
        const auto trackUrl = trackObject.value(QLatin1String("url")).toString();
        const auto trackDuration = trackObject.value(QLatin1String("duration")).toInt();
        const QString resultPlaycount = QString::number(trackPlaycount);
        const QString resultMatch = QString::number(trackMatch);
        const QString resultDuration = QString::number(trackDuration);
        const auto artistObject = trackObject.value(QLatin1String("artist")).toObject();
        if (artistObject.isEmpty()) {
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "No artist available for track"
                        << trackName;
            }
            continue;
        }
        const auto artistName = artistObject.value(QLatin1String("name")).toString();
        const auto artistUrl = artistObject.value(QLatin1String("url")).toString();
        const auto artistMbid = artistObject.value(QLatin1String("mbid")).toString();
        results.last().insert(QLatin1String("title"), trackName);
        results.last().insert(QLatin1String("artist"), artistName);
        results.last().insert(QLatin1String("playcount"), resultPlaycount);
        results.last().insert(QLatin1String("match"), resultMatch);
        results.last().insert(QLatin1String("duration"), resultDuration);
    }

    emitSucceeded(results);
}

void LastfmGetTrackSimilarTask::emitSucceeded(const QList<QMap<QString, QString>>& suggestions) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&LastfmGetTrackSimilarTask::succeeded)) {
        kLogger.warning()
                << "Unhandled succeeded signal";
        deleteLater();
        return;
    }
    emit succeeded(suggestions);
}

} // namespace mixxx
