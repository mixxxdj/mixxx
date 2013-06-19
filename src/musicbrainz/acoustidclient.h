/*****************************************************************************
 *  Copyright © 2012 John Maguire <john.maguire@gmail.com>                   *
 *                   David Sansome <me@davidsansome.com>                     *
 *  This work is free. You can redistribute it and/or modify it under the    *
 *  terms of the Do What The Fuck You Want To Public License, Version 2,     *
 *  as published by Sam Hocevar.                                             *
 *  See http://www.wtfpl.net/ for more details.                              *
 *****************************************************************************/
    
#ifndef ACOUSTIDCLIENT_H
#define ACOUSTIDCLIENT_H

#include <QMap>
#include <QObject>
#include <QtNetwork>

#include "network.h"
#include "trackinfoobject.h"

class QXmlStreamReader;

class AcoustidClient : public QObject {
  Q_OBJECT

    // Gets a MBID from a Chromaprint fingerprint.
    // A fingerprint identifies one particular encoding of a song and is created
    // by Fingerprinter.  An MBID identifies the actual song and can be passed to
    // Musicbrainz to get metadata.
    // You can create one AcoustidClient and make multiple requests using it.
    // IDs are provided by the caller when a request is started and included in
    // the finished signal - they have no meaning to AcoustidClient.

  public:
    AcoustidClient(QObject* parent = 0);

    // Network requests will be aborted after this interval.
    void setTimeout(int msec);

    // Starts a request and returns immediately.  Finished() will be emitted
    // later with the same ID.
    void start(int id, const QString& fingerprint, int duration);

    // Cancels the request with the given ID.  Finished() will never be emitted
    // for that ID.  Does nothing if there is no request with the given ID.
    void cancel(int id);

    // Cancels all requests.  Finished() will never be emitted for any pending
    // requests.
    void cancelAll();

    QString parseResult(QXmlStreamReader& reader);

  signals:
    void finished(int id, const QString& mbid);

  private slots:
    void requestFinished();

  private:
    static const int m_DefaultTimeout;

    QNetworkAccessManager m_network;
    NetworkTimeouts m_timeouts;
    QMap<QNetworkReply*, int> m_requests;
};

#endif // ACOUSTIDCLIENT_H
