/*****************************************************************************
 *  Copyright © 2012 John Maguire <john.maguire@gmail.com>                   *
 *                   David Sansome <me@davidsansome.com>                     *
 *  This work is free. You can redistribute it and/or modify it under the    *
 *  terms of the Do What The Fuck You Want To Public License, Version 2,     *
 *  as published by Sam Hocevar.                                             *
 *  See http://www.wtfpl.net/ for more details.                              *
 *****************************************************************************/
    
#ifndef TAGFETCHER_H
#define TAGFETCHER_H

#include <QFutureWatcher>
#include <QObject>

#include "musicbrainz/musicbrainzclient.h"
#include "musicbrainz/acoustidclient.h"
#include "trackinfoobject.h"


class TagFetcher : public QObject {
  Q_OBJECT

  // High level interface to Fingerprinter, AcoustidClient and
  // MusicBrainzClient.

  public:
    TagFetcher(QObject* parent = 0);

    void startFetch(const TrackPointer track);

  public slots:
    void cancel();

  signals:
    void resultAvailable(const TrackPointer originalTrack,
                         const QList<TrackPointer>& tracksGuessed);
    void fetchProgress(QString);

  private slots:
    void fingerprintFound(int index);
    void mbidFound(int index, const QString& mbid);
    void tagsFetched(int index, const MusicBrainzClient::ResultList& result);

  private:
    // has to be static so we can call it with QtConcurrent and have a nice
    // responsive UI while the fingerprint is calculated
    static QString getFingerprint(const TrackPointer tio);

    QFutureWatcher<QString>* m_pFingerprintWatcher;
    AcoustidClient m_AcoustidClient;
    MusicBrainzClient m_MusicbrainzClient;

    // Code can already be run on an arbitrary number of input tracks
    QList<TrackPointer> m_tracks;
};

#endif // TAGFETCHER_H
