/*****************************************************************************
 *  Copyright © 2012 John Maguire <john.maguire@gmail.com>                   *
 *                   David Sansome <me@davidsansome.com>                     *
 *  This work is free. You can redistribute it and/or modify it under the    *
 *  terms of the Do What The Fuck You Want To Public License, Version 2,     *
 *  as published by Sam Hocevar.                                             *
 *  See http://www.wtfpl.net/ for more details.                              *
 *****************************************************************************/

#include <QNetworkReply>
#include <QJsonDocument>
#include <QUrl>
#include <QUrlQuery>
#include <QtDebug>

#include "musicbrainz/acoustidclient.h"
#include "musicbrainz/gzip.h"
#include "musicbrainz/network.h"

namespace {

// see API-KEY site here http://acoustid.org/application/496
// I registered the KEY for version 1.12 -- kain88 (may 2013)
// See also: https://acoustid.org/webservice
const QString CLIENT_APIKEY = "czKxnkyO";
const QString ACOUSTID_URL = "https://api.acoustid.org/v2/lookup";
constexpr int kDefaultTimeout = 5000; // msec

} // anonymous namespace

AcoustidClient::AcoustidClient(QObject* parent)
              : QObject(parent),
                m_network(this),
                m_timeouts(kDefaultTimeout, this) {
}

void AcoustidClient::setTimeout(int msec) {
    m_timeouts.setTimeout(msec);
}

void AcoustidClient::start(int id, const QString& fingerprint, int duration) {
    QUrlQuery urlQuery;
    // The default format is JSON
    //urlQuery.addQueryItem("format", "json");
    urlQuery.addQueryItem("client", CLIENT_APIKEY);
    urlQuery.addQueryItem("duration", QString::number(duration));
    urlQuery.addQueryItem("meta", "recordingids");
    urlQuery.addQueryItem("fingerprint", fingerprint);
    // application/x-www-form-urlencoded request bodies must be percent encoded.
    QByteArray body = urlQuery.query(QUrl::FullyEncoded).toLatin1();

    QUrl url(ACOUSTID_URL);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    req.setRawHeader("Content-Encoding", "gzip");

    qDebug() << "AcoustIdClient POST request:" << ACOUSTID_URL
             << "body:" << body;

    QNetworkReply* reply = m_network.post(req, gzipCompress(body));
    connect(reply, &QNetworkReply::finished, this, &AcoustidClient::requestFinished);
    m_requests[reply] = id;

    m_timeouts.addReply(reply);
}

void AcoustidClient::cancel(int id) {
    QNetworkReply* reply = m_requests.key(id);
    m_requests.remove(reply);
    delete reply;
}

void AcoustidClient::cancelAll() {
    auto requests = m_requests;
    m_requests.clear();

    for (auto it = requests.constBegin();
         it != requests.constEnd(); ++it) {
        delete it.key();
    }
}

void AcoustidClient::requestFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;

    reply->deleteLater();
    if (!m_requests.contains(reply))
        return;

    int id = m_requests.take(reply);

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray body = reply->readAll();
    qDebug() << "AcoustIdClient POST reply status:" << statusCode << "body:" << body;

    const auto jsonResponse = QJsonDocument::fromJson(body);
    QString statusText;
    if (jsonResponse.isObject()) {
        statusText = jsonResponse.object().value("status").toString();
    }

    if (statusCode != 200 || statusText != "ok") {
        QString errorMessage;
        int errorCode = 0;
        if (jsonResponse.isObject() && statusText == "error") {
            const auto error = jsonResponse.object().value("error").toObject();
            errorMessage = error.value("message").toString();
            errorCode = error.value("message").toInt(errorCode);
        }
        emit networkError(
             statusCode,
             "AcoustID",
             errorMessage,
             errorCode);
        return;
    }

    QString recordingId;
    DEBUG_ASSERT(jsonResponse.isObject());
    DEBUG_ASSERT(jsonResponse.object().value("results").isArray());
    const QJsonArray results = jsonResponse.object().value("results").toArray();
    if (!results.isEmpty()) {
        // Only take the first result with the maximum(?) score
        DEBUG_ASSERT(results.at(0).toObject().value("recordings").isArray());
        const QJsonArray recordings = results.at(0).toObject().value("recordings").toArray();
        if (!recordings.isEmpty()) {
            // Only take the first recording
            recordingId = recordings.at(0).toObject().value("id").toString();
        }
    }
    emit finished(id, recordingId);
}
