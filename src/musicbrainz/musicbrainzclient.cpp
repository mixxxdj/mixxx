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
#include <QtNetwork>
#include <QSet>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QUrl>
#include <QJsonDocument>

#include "musicbrainz/musicbrainzclient.h"
#include "util/version.h"
#include "defs_urls.h"


namespace {

const QString kTrackUrl = "http://musicbrainz.org/ws/2/recording/";
const QString kDateRegex = "^[12]\\d{3}";
constexpr int kDefaultTimeout = 5000; // msec
constexpr int kDefaultErrorCode = 0;

QString decodeText(const QByteArray& data, const QStringRef codecName) {
    QTextStream textStream(data);
    if (!codecName.isEmpty()) {
        textStream.setCodec(QTextCodec::codecForName(codecName.toUtf8()));
    }
    return textStream.readAll();
}

} // anonymous namespace




MusicBrainzClient::MusicBrainzClient(QObject* parent)
                 : QObject(parent),
                   m_network(this),
                   m_timeouts(kDefaultTimeout, this) {
}

void MusicBrainzClient::start(int id, const QString& mbid) {
    typedef QPair<QString, QString> Param;

    QList<Param> parameters;
    parameters << Param("inc", "artists+releases+media");

    QUrlQuery query;
    query.setQueryItems(parameters);
    QUrl url(kTrackUrl + mbid);
    url.setQuery(query);
    qDebug() << "MusicBrainzClient GET request:" << url.toString();
    QNetworkRequest req(url);
    // http://musicbrainz.org/doc/XML_Web_Service/Rate_Limiting#Provide_meaningful_User-Agent_strings
    QString mixxxMusicBrainzId(Version::applicationName() + "/" + Version::version() + " ( " + MIXXX_WEBSITE_URL + " )");
    // HTTP request headers must be latin1.
    req.setRawHeader("User-Agent", mixxxMusicBrainzId.toLatin1());
    QNetworkReply* reply = m_network.get(req);
    connect(reply, &QNetworkReply::finished, this, &MusicBrainzClient::requestFinished);
    m_requests[reply] = id;

    m_timeouts.addReply(reply);
}

void MusicBrainzClient::cancel(int id) {
    QNetworkReply* reply = m_requests.key(id);
    m_requests.remove(reply);
    delete reply;
}

void MusicBrainzClient::cancelAll() {
    auto requests = m_requests;
    m_requests.clear();
    for (auto it = requests.constBegin();
         it != requests.constEnd(); ++it) {
        delete it.key();
    }
}

void MusicBrainzClient::requestFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;

    reply->deleteLater();
    if (!m_requests.contains(reply))
        return;

    int id = m_requests.take(reply);
    ResultList ret;

    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QTextStream textReader(reply);
    const QByteArray body(reply->readAll());
    QXmlStreamReader reader(body);

    // MusicBrainz returns 404 when the MBID is not in their database. We treat
    // a status of 404 the same as a 200 but it will produce an empty list of
    // results.
    if (status != 200 && status != 404) {
        qDebug() << "MusicBrainzClient POST reply status:" << status << "body:" << body;
        QJsonDocument jsonResponse = QJsonDocument::fromJson(body);
        QJsonObject jsonObject = jsonResponse.object();
        QString message = jsonObject["error"].toString();
        QStringList propertyNames;
        QStringList propertyKeys;
        QString strReply = (QString)reply->readAll();
        emit(networkError(
             reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
             "MusicBrainz", message, kDefaultErrorCode));
        return;
    }

    QStringRef codecName;
    while (!reader.atEnd()) {
        switch (reader.readNext()) {
        case QXmlStreamReader::Invalid:
        {
            qWarning() << "MusicBrainzClient GET reply body:"
                    << decodeText(body, codecName);
            qWarning()
                << "MusicBrainzClient GET decoding error:"
                << reader.errorString();
            break;
        }
        case QXmlStreamReader::StartDocument:
        {
            // The character encoding is always an ASCII string
            codecName = reader.documentEncoding();
            qDebug() << "MusicBrainzClient GET reply codec:"
                     << codecName;
            qDebug() << "MusicBrainzClient GET reply body:"
                     << decodeText(body, codecName);
            break;
        }
        case QXmlStreamReader::StartElement:
        {
            if (reader.name() == "recording") {
                ResultList tracks = parseTrack(reader);
                for (const Result& track: tracks) {
                    if (!track.m_title.isEmpty()) {
                        ret << track;
                    }
                }
            }
            break;
        }
        default:
        {
            // ignore any other token type
        }
        }
    }
    emit(finished(id, uniqueResults(ret)));
}

MusicBrainzClient::ResultList MusicBrainzClient::parseTrack(QXmlStreamReader& reader) {
    Result result;
    QList<Release> releases;

    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType type = reader.readNext();

        if (type == QXmlStreamReader::StartElement) {
            QStringRef name = reader.name();
            if (name == "title") {
                result.m_title = reader.readElementText();
            } else if (name == "length") {
                // convert msec to sec
                result.m_duration = reader.readElementText().toInt()*10000000;
            } else if (name == "artist") {
                parseArtist(reader, result.m_artist);
            } else if (name == "release") {
                releases << parseRelease(reader);
            }
        }

        if (type == QXmlStreamReader::EndElement && reader.name() == "recording") {
        break;
        }
    }

    ResultList ret;
    foreach (const Release& release, releases) {
        ret << release.CopyAndMergeInto(result);
    }
    return ret;
}

void MusicBrainzClient::parseArtist(QXmlStreamReader& reader, QString& artist) {
    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType type = reader.readNext();

        if (type == QXmlStreamReader::StartElement && reader.name() == "name") {
            artist = reader.readElementText();
        }

        if (type == QXmlStreamReader::EndElement && reader.name() == "artist") {
            return;
        }
    }
}

MusicBrainzClient::Release MusicBrainzClient::parseRelease(QXmlStreamReader& reader) {
    Release ret;

    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType type = reader.readNext();

        if (type == QXmlStreamReader::StartElement) {
            QStringRef name = reader.name();
            if (name == "title") {
                ret.m_album = reader.readElementText();
            } else if (name == "date") {
                QRegExp regex(kDateRegex);
                if (regex.indexIn(reader.readElementText()) == 0) {
                ret.m_year = regex.cap(0).toInt();
                }
            } else if (name == "track-list") {
                ret.m_track = reader.attributes().value("offset").toString().toInt() + 1;
                consumeCurrentElement(reader);
            }
        }

        if (type == QXmlStreamReader::EndElement && reader.name() == "release") {
            break;
        }
    }

    return ret;
}

MusicBrainzClient::ResultList MusicBrainzClient::uniqueResults(const ResultList& results) {
    ResultList ret = QSet<Result>::fromList(results).toList();
    qSort(ret);
    return ret;
}

void MusicBrainzClient::consumeCurrentElement(QXmlStreamReader& reader) {
    int level = 1;
    while (level != 0 && !reader.atEnd()) {
        switch (reader.readNext()) {
        case QXmlStreamReader::StartElement: ++level; break;
        case QXmlStreamReader::EndElement:   --level; break;
        default: break;
        }
    }
}
