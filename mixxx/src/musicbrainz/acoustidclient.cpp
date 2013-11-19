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

#include "acoustidclient.h"
#include "gzip.h"
#include "network.h"

// see API-KEY site here http://acoustid.org/application/496
// I registered the KEY for version 1.12 -- kain88 (may 2013)
const QString CLIENT_APIKEY = "czKxnkyO";
const QString CLIENT_NAME = "Mixxx1.12";
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

    QNetworkRequest req(QUrl::fromEncoded(ACOUSTID_URL.toAscii()));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    req.setRawHeader("Content-Encoding", "gzip");
    req.setRawHeader("User-Agent", CLIENT_NAME.toAscii());
    
    QNetworkReply* reply = m_network.post(req, gzipCompress(url.encodedQuery()));
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
    
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
        emit finished(id, QString());
        return;
    }

    QXmlStreamReader reader(reply);
    QString ID;
    while (!reader.atEnd()) {
        if (reader.readNext() == QXmlStreamReader::StartElement 
            && reader.name()== "results") {
                ID = parseResult(reader);
            }
    }

    emit finished(id, ID);
}

QString AcoustidClient::parseResult(QXmlStreamReader& reader){

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
