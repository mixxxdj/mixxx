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

#include "util/logger.h"
#include "util/version.h"
#include "defs_urls.h"


namespace {

mixxx::Logger kLogger("MusicBrainzClient");

const QString kRecordingUrl = "https://musicbrainz.org/ws/2/recording/";
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

void MusicBrainzClient::start(int id, QStringList recordingIds) {
    nextRequest(id, Request(std::move(recordingIds)));
}

void MusicBrainzClient::nextRequest(int id, Request request) {
    DEBUG_ASSERT(!m_requests.contains(id));
    if (request.recordingIds.isEmpty()) {
        emit finished(id, uniqueResults(request.results));
        return;
    }
    const auto recordingId = request.recordingIds.takeFirst();

    typedef QPair<QString, QString> Param;
    QList<Param> parameters;
    parameters << Param("inc", "artists+releases+media");

    QUrl url(kRecordingUrl + recordingId);
    QUrlQuery query;
    query.setQueryItems(parameters);
    url.setQuery(query);
    kLogger.debug()
            << "GET request:"
            << url.toString();
    QNetworkRequest req(url);
    // http://musicbrainz.org/doc/XML_Web_Service/Rate_Limiting#Provide_meaningful_User-Agent_strings
    QString mixxxMusicBrainzId(Version::applicationName() + "/" + Version::version() + " ( " + MIXXX_WEBSITE_URL + " )");
    // HTTP request headers must be latin1.
    req.setRawHeader("User-Agent", mixxxMusicBrainzId.toLatin1());
    QNetworkReply* reply = m_network.get(req);
    m_requests[id] = std::move(request);
    m_replies[reply] = id;
    connect(reply, &QNetworkReply::finished, this, &MusicBrainzClient::replyFinished);
    m_timeouts.addReply(reply);
}

void MusicBrainzClient::cancel(int id) {
    m_requests.remove(id);
    QNetworkReply* reply = m_replies.key(id);
    if (reply) {
        reply->abort();
    }
}

void MusicBrainzClient::cancelAll() {
    m_requests.clear();
    auto replies = m_replies;
    m_replies.clear();
    for (auto it = replies.constBegin();
         it != replies.constEnd(); ++it) {
        it.key()->abort();
    }
}

void MusicBrainzClient::replyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    reply->deleteLater();

    if (!m_replies.contains(reply)) {
        return;
    }
    int id = m_replies.take(reply);

    if (!m_requests.contains(id)) {
        return;
    }
    Request request = m_requests.take(id);

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
        emit networkError(
             reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
             "MusicBrainz", message, kDefaultErrorCode);
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
                        request.results << track;
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
    nextRequest(id, request);
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
    // TODO: QSet<T>::fromList(const QList<T>&) is deprecated and should be
    // replaced with QSet<T>(list.begin(), list.end()).
    // However, the proposed alternative has just been introduced in Qt
    // 5.14. Until the minimum required Qt version of Mixx is increased,
    // we need a version check here
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    ResultList ret = QSet<Result>(results.begin(), results.end()).values();
#else
    ResultList ret = QSet<Result>::fromList(results).toList();
#endif
    std::sort(ret.begin(), ret.end());
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
