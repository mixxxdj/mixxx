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

// see API-KEY site here http://acoustid.org/application/496
// I registered the KEY for version 1.12 -- kain88 (may 2013)
const QString CLIENT_APIKEY = "czKxnkyO";
const QString ACOUSTID_URL = "http://api.acoustid.org/v2/lookup";
const int AcoustidClient::m_DefaultTimeout = 5000; // msec

AcoustidClient::AcoustidClient(QObject* parent)
              : QObject(parent),
                m_network(this),
                m_timeouts(m_DefaultTimeout, this) {
}

void AcoustidClient::setTimeout(int msec) {
    m_timeouts.setTimeout(msec);
}

void AcoustidClient::start(int id, const QString& fingerprint, int duration) {
    QUrl url;
    url.addQueryItem("format", "xml");
    url.addQueryItem("client", CLIENT_APIKEY);
    url.addQueryItem("duration", QString::number(duration));
    url.addQueryItem("meta", "recordingids");
    url.addQueryItem("fingerprint", fingerprint);
    QByteArray body = url.encodedQuery();

    QNetworkRequest req(QUrl::fromEncoded(ACOUSTID_URL.toAscii()));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    req.setRawHeader("Content-Encoding", "gzip");

    qDebug() << "AcoustIdClient POST request:" << ACOUSTID_URL
             << "body:" << body;

    QNetworkReply* reply = m_network.post(req, gzipCompress(body));
    connect(reply, SIGNAL(finished()), SLOT(requestFinished()));
    m_requests[reply] = id;

    m_timeouts.addReply(reply);
}

void AcoustidClient::cancel(int id) {
    QNetworkReply* reply = m_requests.key(id);
    m_requests.remove(reply);
    delete reply;
}

void AcoustidClient::cancelAll() {
    qDeleteAll(m_requests.keys());
    m_requests.clear();
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
    if (status != 200) {
        QTextStream body(reply);
        qDebug() << "AcoustIdClient POST reply status:" << status << "body:" << body.readAll();
        emit(networkError(
             reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
             "AcoustID"));
        return;
    }


    QTextStream textReader(reply);
    QString body = textReader.readAll();
    qDebug() << "AcoustIdClient POST reply status:" << status << "body:" << body;

    QXmlStreamReader reader(body);
    QString ID;
    while (!reader.atEnd()) {
        if (reader.readNext() == QXmlStreamReader::StartElement
            && reader.name()== "results") {
                ID = parseResult(reader);
            }
    }

    emit(finished(id, ID));
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
