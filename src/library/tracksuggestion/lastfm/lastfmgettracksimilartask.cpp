#include "library/tracksuggestion/lastfm/lastfmgettracksimilartask.h"

#include <QDebug>
#include <QMetaMethod>
#include <QString>
#include <QXmlStreamReader>

#include "util/assert.h"
#include "util/logger.h"
#include "util/thread_affinity.h"

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
    //For now we will get the response as an XML file...
    //Later on json can be used if needed.
    //urlQuery.addQueryItem(
    //        QStringLiteral("format"),
    //        QStringLiteral("json"));
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

} // anonymous namespace

LastfmGetTrackSimilarTask::LastfmGetTrackSimilarTask(
        QNetworkAccessManager* networkAccessManager,
        const QString& artist,
        const QString& track,
        QObject* parent)
        : network::WebTask(
                  networkAccessManager,
                  parent),
          m_urlQuery(lookupUrlQuery(artist, track)) {
}

QNetworkReply* LastfmGetTrackSimilarTask::doStartNetworkRequest(
        QNetworkAccessManager* networkAccessManager,
        int parentTimeoutMillis) {
    Q_UNUSED(parentTimeoutMillis)
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(networkAccessManager);

    const QNetworkRequest networkRequest = createNetworkRequest(m_urlQuery);

    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "GET"
                << networkRequest.url();
    }
    return networkAccessManager->get(networkRequest);
}

void LastfmGetTrackSimilarTask::doNetworkReplyFinished(
        QNetworkReply* finishedNetworkReply,
        network::HttpStatusCode statusCode) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    const QByteArray body = finishedNetworkReply->readAll();

    if (statusCode != 200 &&
            statusCode != 301 &&
            statusCode != 404) {
        kLogger.info()
                << "GET reply"
                << "statusCode:" << statusCode
                << "body:" << body;
        emitFailed(
                network::WebResponse(
                        finishedNetworkReply->url(),
                        finishedNetworkReply->request().url(),
                        statusCode),
                statusCode,
                body);
        return;
    }

    emitSucceeded(body);
}

void LastfmGetTrackSimilarTask::emitSucceeded(const QByteArray& response) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&LastfmGetTrackSimilarTask::succeeded)) {
        kLogger.warning()
                << "Unhandled succeeded signal";
        deleteLater();
        return;
    }
    emit succeeded(response);
}

void LastfmGetTrackSimilarTask::emitFailed(
        const network::WebResponse& response,
        int errorCode,
        const QString& errorMessage) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&LastfmGetTrackSimilarTask::failed)) {
        kLogger.warning()
                << "Unhandled failed signal"
                << response
                << errorCode
                << errorMessage;
        deleteLater();
        return;
    }
    emit failed(
            response,
            errorCode,
            errorMessage);
}

} // namespace mixxx
