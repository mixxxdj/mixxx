#ifndef LASTFMCLIENT_H
#define LASTFMCLIENT_H

#include <QMap>
#include <QObject>
#include <QtNetwork>

#include "network.h"

class LastFmClient : public QObject {
  Q_OBJECT

  // Fetch social tags for a track using liblastfm

  public:
    LastFmClient(QObject *parent = 0);

    typedef QMap<QString, int> TagCounts;

    // Starts a request and returns immediately.  finished() will be emitted
    // later with the same ID.
    void start(int id, const QString& artist, const QString& title);
    static void consumeCurrentElement(QXmlStreamReader& reader);

    // Cancels the request with the given ID.  Finished() will never be emitted
    // for that ID.  Does nothing if there is no request with the given ID.
    void cancel(int id);

    // Cancels all requests.  Finished() will never be emitted for any pending
    // requests.
    void cancelAll();

  signals:
    // Emitted when song's social tags have been fetched
    void finished(int id, TagCounts tags);
    
  private slots:
    void requestFinished();

  private:
    static QString escapeString(const QString& string);
    static TagCounts parseTopTags(QXmlStreamReader& reader);

    static const int m_iDefaultTimeout;
    static const QString m_sApiKey;
    static const QString m_sRestUrl;
    QNetworkAccessManager m_network;
    NetworkTimeouts m_timeouts;
    QMap<QNetworkReply*, int> m_requests;
};

#endif // LASTFMCLIENT_H
