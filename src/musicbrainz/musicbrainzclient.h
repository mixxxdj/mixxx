/*****************************************************************************
 *  Copyright © 2012 John Maguire <john.maguire@gmail.com>                   *
 *                   David Sansome <me@davidsansome.com>                     *
 *  This work is free. You can redistribute it and/or modify it under the    *
 *  terms of the Do What The Fuck You Want To Public License, Version 2,     *
 *  as published by Sam Hocevar.                                             *
 *  See http://www.wtfpl.net/ for more details.                              *
 *****************************************************************************/

#ifndef MUSICBRAINZCLIENT_H
#define MUSICBRAINZCLIENT_H

#include <QHash>
#include <QMap>
#include <QObject>
#include <QXmlStreamReader>
#include <QtNetwork>

#include "musicbrainz/network.h"

class MusicBrainzClient : public QObject {
  Q_OBJECT

  // Gets metadata for a particular MBID.
  // An MBID is created from a fingerprint using chromaprint library.
  // You can create one MusicBrainzClient and make multiple requests using it.
  // IDs are provided by the caller when a request is started and included in
  // the Finished signal - they have no meaning to MusicBrainzClient.

  public:
    MusicBrainzClient(QObject* parent = 0);

    struct Result {
        Result() : m_duration(0), m_track(0), m_year(-1) {}

        bool operator <(const Result& other) const {
            #define cmp(field) \
                if (field < other.field) return true; \
                if (field > other.field) return false;

            cmp(m_track);
            cmp(m_year);
            cmp(m_title);
            cmp(m_artist);
            return false;

            #undef cmp
        }

        bool operator ==(const Result& other) const {
            return m_title == other.m_title &&
                   m_artist == other.m_artist &&
                   m_album == other.m_album &&
                   m_duration == other.m_duration &&
                   m_track == other.m_track &&
                   m_year == other.m_year;
        }

        QString m_title;
        QString m_artist;
        QString m_album;
        int m_duration;
        int m_track;
        int m_year;
    };
    typedef QList<Result> ResultList;


    // Starts a request and returns immediately.  finished() will be emitted
    // later with the same ID.
    void start(int id, const QString& mbid);
    static void consumeCurrentElement(QXmlStreamReader& reader);

    // Cancels the request with the given ID.  Finished() will never be emitted
    // for that ID.  Does nothing if there is no request with the given ID.
    void cancel(int id);

    // Cancels all requests.  Finished() will never be emitted for any pending
    // requests.
    void cancelAll();

  signals:
    // Finished signal emitted when fechting songs tags
    void finished(int id, const MusicBrainzClient::ResultList& result);
    void networkError(int httpStatus, QString app, QString message, int code);

  private slots:
    void requestFinished();

  private:
    struct Release {
        Release() : m_track(0), m_year(0) {}

        Result CopyAndMergeInto(const Result& orig) const {
            Result ret(orig);
            ret.m_album = m_album;
            ret.m_track = m_track;
            ret.m_year = m_year;
            return ret;
        }

        QString m_album;
        int m_track;
        int m_year;
    };

    static ResultList parseTrack(QXmlStreamReader& reader);
    static void parseArtist(QXmlStreamReader& reader, QString& artist);
    static Release parseRelease(QXmlStreamReader& reader);
    static ResultList uniqueResults(const ResultList& results);

  private:
    QNetworkAccessManager m_network;
    NetworkTimeouts m_timeouts;
    QMap<QNetworkReply*, int> m_requests;
};

inline uint qHash(const MusicBrainzClient::Result& result) {
  return qHash(result.m_album) ^
         qHash(result.m_artist) ^
         result.m_duration ^
         qHash(result.m_title) ^
         result.m_track ^
         result.m_year;
}

#endif // MUSICBRAINZCLIENT_H
