/*****************************************************************************
 *  Copyright © 2012 John Maguire <john.maguire@gmail.com>                   *
 *                   David Sansome <me@davidsansome.com>                     *
 *  This work is free. You can redistribute it and/or modify it under the    *
 *  terms of the Do What The Fuck You Want To Public License, Version 2,     *
 *  as published by Sam Hocevar.                                             *
 *  See http://www.wtfpl.net/ for more details.                              *
 *****************************************************************************/
    
#include <QFuture>
#include <QUrl>
#include <QtConcurrentMap>

#include "musicbrainz/tagfetcher.h"
#include "musicbrainz/chromaprinter.h"
#include "musicbrainz/musicbrainzclient.h"

TagFetcher::TagFetcher(QObject* parent)
          : QObject(parent),
            m_pFingerprintWatcher(NULL),
            m_AcoustidClient(this),
            m_MusicbrainzClient(this) {
    connect(&m_AcoustidClient, SIGNAL(Finished(int,QString)),
            this, SLOT(MbidFound(int,QString)));
    connect(&m_AcoustidClient, SIGNAL(submited(int,QString)),
            this, SIGNAL(submited(int,QString)));
    connect(&m_MusicbrainzClient, SIGNAL(Finished(int,MusicBrainzClient::ResultList)),
            this, SLOT(TagsFetched(int,MusicBrainzClient::ResultList)));
}

QString TagFetcher::GetFingerprint(const TrackPointer tio) {
    return chromaprinter(NULL).getFingerPrint(tio,false);
}

void TagFetcher::StartFetch(const TrackPointer track) {
    Cancel();
    // qDebug() << "start to fetch track metadata";
    QList<TrackPointer> tracks;
    tracks.append(track);
    m_tracks = tracks;

    QFuture<QString> future = QtConcurrent::mapped(m_tracks, GetFingerprint);
    m_pFingerprintWatcher = new QFutureWatcher<QString>(this);
    m_pFingerprintWatcher->setFuture(future);
    connect(m_pFingerprintWatcher, SIGNAL(resultReadyAt(int)),
            SLOT(FingerprintFound(int)));

    foreach (const TrackPointer ptrack, m_tracks) {
        emit fetchProgress(tr("Fingerprinting track"));
    }
}

void TagFetcher::Cancel() {
    // qDebug()<< "Cancel tagfetching";
    if (m_pFingerprintWatcher) {
        m_pFingerprintWatcher->cancel();

        delete m_pFingerprintWatcher;
        m_pFingerprintWatcher = NULL;
    }

    m_AcoustidClient.CancelAll();
    m_MusicbrainzClient.CancelAll();
    m_tracks.clear();
}

void TagFetcher::FingerprintFound(int index) {
    QFutureWatcher<QString>* watcher = reinterpret_cast<QFutureWatcher<QString>*>(sender());
    if (!watcher || index >= m_tracks.count()) {
        return;
    }

    const QString fingerprint = watcher->resultAt(index);
    const TrackPointer ptrack = m_tracks[index];

    if (fingerprint.isEmpty()) {
        emit ResultAvailable(ptrack, QList<TrackPointer>());
        return;
    }

    emit fetchProgress(tr("Identifying track"));
    // qDebug() << "start to look it up on musicbrainz";
    m_AcoustidClient.Start(index, fingerprint, ptrack->getDuration());
}

void TagFetcher::MbidFound(int index, const QString& mbid) {
    if (index >= m_tracks.count()) {
        return;
    }

    const TrackPointer pTrack = m_tracks[index];

    if (mbid.isEmpty()) {
        emit ResultAvailable(pTrack, QList<TrackPointer>());
        return;
    }

    emit fetchProgress(tr("Downloading Metadata"));
    m_MusicbrainzClient.Start(index, mbid);
}

void TagFetcher::TagsFetched(int index, const MusicBrainzClient::ResultList& results) {
    if (index >= m_tracks.count()) {
        return;
    }
    // qDebug() << "Tagfetcher got musicbrainz results and now refurbrishs them";
    const TrackPointer originalTrack = m_tracks[index];
    QList<TrackPointer> tracksGuessed;

    foreach (const MusicBrainzClient::Result& result, results) {
        TrackPointer track(new TrackInfoObject(originalTrack->getLocation(),false),
                           &QObject::deleteLater);
        track->setTitle(result.m_title);
        track->setArtist(result.m_artist);
        track->setAlbum(result.m_album);
        track->setDuration(result.m_duration);
        track->setTrackNumber(QString::number(result.m_track));
        track->setYear(QString::number(result.m_year));
        tracksGuessed << track;
    }
    // qDebug() << "send this to the world";
    emit ResultAvailable(originalTrack, tracksGuessed);
}
