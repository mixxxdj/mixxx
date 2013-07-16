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

    typedef QMap<int, QString> TagCounts;

    // Starts a request and returns immediately.  finished() will be emitted
    // later with the same ID.
    void start(int id, const QString& artist, const QString& title);
    void cancel(int id);
    void cancelAll();

  signals:
    // Emitted when song's social tags have been fetched
    void finished(int id, TagCounts tags);
    
  private slots:
    void requestFinished();

  private:
    static const int m_DefaultTimeout;
    NetworkTimeouts m_timeouts;
    QMap<QNetworkReply*, int> m_requests;
};

#endif // LASTFMCLIENT_H
