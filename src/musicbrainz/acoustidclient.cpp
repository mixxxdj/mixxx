/*****************************************************************************
 *  Copyright © 2012 John Maguire <john.maguire@gmail.com>                   *
 *                   David Sansome <me@davidsansome.com>                     *
 *  This work is free. You can redistribute it and/or modify it under the    *
 *  terms of the Do What The Fuck You Want To Public License, Version 2,     *
 *  as published by Sam Hocevar.                                             *
 *  See http://www.wtfpl.net/ for more details.                              *
 *****************************************************************************/

#include <QCoreApplication>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QTextStream>
#include <QUrl>
#include <QtDebug>

#include "musicbrainz/acoustidclient.h"
#include "musicbrainz/gzip.h"
#include "musicbrainz/network.h"

namespace {

// see API-KEY site here http://acoustid.org/application/496
// I registered the KEY for version 1.12 -- kain88 (may 2013)
const QString CLIENT_APIKEY = "czKxnkyO";
const QString ACOUSTID_URL = "http://api.acoustid.org/v2/lookup";
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
    urlQuery.addQueryItem("format", "xml");
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

    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QTextStream textReader(reply);
    const QByteArray body(reply->readAll());
    QXmlStreamReader reader(body);

    QString statusText;
    while (!reader.atEnd() && statusText.isEmpty()) {
        if (reader.readNextStartElement()) {
            const QStringRef name = reader.name();
            if (name == "status") {
                statusText = reader.readElementText();
            }
        }
    }

    if (status != 200 || statusText != "ok") {
        qDebug() << "AcoustIdClient POST reply status:" << status << "body:" << body;
        QString message;
        QString code;
        while (!reader.atEnd() && (message.isEmpty() || code.isEmpty())) {
            if (reader.readNextStartElement()) {
                const QStringRef name = reader.name();
                if (name == "message") {
                    DEBUG_ASSERT(name.isEmpty()); // fail if we have duplicated message elements. 
                    message = reader.readElementText();
                } else if (name == "code") {
                    DEBUG_ASSERT(code.isEmpty()); // fail if we have duplicated code elements. 
                    code = reader.readElementText();
                }
            }
        }
        emit(networkError(
             reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
             "AcoustID", message, code.toInt()));
        return;
    }

    qDebug() << "AcoustIdClient POST reply status:" << status << "body:" << body;

    QString resultId;
    while (!reader.atEnd() && resultId.isEmpty()) {
        if (reader.readNextStartElement()
                && reader.name()== "results") {
            resultId = parseResult(reader);
        }
    }

    emit(finished(id, resultId));
}

QString AcoustidClient::parseResult(QXmlStreamReader& reader) {
    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType type = reader.readNext();
        if (type== QXmlStreamReader::StartElement) {
            QStringRef name = reader.name();
            if (name == "id") {
                return reader.readElementText();
            }
        }
        if (type == QXmlStreamReader::EndElement && reader.name()=="result") {
            break;
        }
    }
    return QString();
}
