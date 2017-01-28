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

#include "musicbrainz/musicbrainzclient.h"
#include "util/version.h"
#include "defs_urls.h"

const QString MusicBrainzClient::m_TrackUrl = "http://musicbrainz.org/ws/2/recording/";
const QString MusicBrainzClient::m_DateRegex = "^[12]\\d{3}";
const int MusicBrainzClient::m_DefaultTimeout = 5000; // msec

MusicBrainzClient::MusicBrainzClient(QObject* parent)
                 : QObject(parent),
                   m_network(this),
                   m_timeouts(m_DefaultTimeout, this) {
}

void MusicBrainzClient::start(int id, const QString& mbid) {
    typedef QPair<QString, QString> Param;

    QList<Param> parameters;
    parameters << Param("inc", "artists+releases+media");

    QUrl url(m_TrackUrl + mbid);
    url.setQueryItems(parameters);
    qDebug() << "MusicBrainzClient GET request:" << url.toString();
    QNetworkRequest req(url);
    // http://musicbrainz.org/doc/XML_Web_Service/Rate_Limiting#Provide_meaningful_User-Agent_strings
    QString mixxxMusicBrainzId(Version::applicationName() + "/" + Version::version() + " ( " + MIXXX_WEBSITE_URL + " )");
    req.setRawHeader("User-Agent", mixxxMusicBrainzId.toAscii());
    QNetworkReply* reply = m_network.get(req);
    connect(reply, SIGNAL(finished()), SLOT(requestFinished()));
    m_requests[reply] = id;

    m_timeouts.addReply(reply);
}

void MusicBrainzClient::cancel(int id) {
    QNetworkReply* reply = m_requests.key(id);
    m_requests.remove(reply);
    delete reply;
}

void MusicBrainzClient::cancelAll() {
    qDeleteAll(m_requests.keys());
    m_requests.clear();
}

namespace {
    QString decodeText(const QByteArray& data, const char* codecName) {
        QTextStream textStream(data);
        if ((nullptr != codecName) && (0 < strlen(codecName))) {
            textStream.setCodec(codecName);
        }
        return textStream.readAll();
    }
} // anonymous namespace

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
    qDebug() << "MusicBrainzClient GET reply status:" << status;

    // MusicBrainz returns 404 when the MBID is not in their database. We treat
    // a status of 404 the same as a 200 but it will produce an empty list of
    // results.
    if (status != 200 && status != 404) {
        emit(networkError(
             reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
             "MusicBrainz"));
        return;
    }

    const QByteArray body(reply->readAll());

    QXmlStreamReader reader(body);
    QByteArray codecName;
    while (!reader.atEnd()) {
        switch (reader.readNext()) {
        case QXmlStreamReader::Invalid:
        {
            qWarning() << "MusicBrainzClient GET reply body:"
                    << decodeText(body, codecName.constData());
            qWarning()
                << "MusicBrainzClient GET decoding error:"
                << reader.errorString();
            break;
        }
        case QXmlStreamReader::StartDocument:
        {
            // The character encoding is always an ASCII string
            codecName = reader.documentEncoding().toAscii();
            qDebug() << "MusicBrainzClient GET reply codec:"
                    << codecName.constData();
            qDebug() << "MusicBrainzClient GET reply body:"
                    << decodeText(body, codecName.constData());
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
                QRegExp regex(m_DateRegex);
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
