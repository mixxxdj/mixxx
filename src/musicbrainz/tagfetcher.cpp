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
#include <QtConcurrent>
#include <QtConcurrentMap>

#include "musicbrainz/tagfetcher.h"
#include "musicbrainz/chromaprinter.h"
#include "musicbrainz/musicbrainzclient.h"

TagFetcher::TagFetcher(QObject* parent)
          : QObject(parent),
            m_pFingerprintWatcher(NULL),
            m_AcoustidClient(this),
            m_MusicbrainzClient(this) {
    connect(&m_AcoustidClient, &AcoustidClient::finished,
            this, &TagFetcher::mbidFound);
    connect(&m_MusicbrainzClient, &MusicBrainzClient::finished,
            this, &TagFetcher::tagsFetched);
    connect(&m_AcoustidClient, &AcoustidClient::networkError,
            this, &TagFetcher::networkError);
    connect(&m_MusicbrainzClient, &MusicBrainzClient::networkError,
            this, &TagFetcher::networkError);
}

QString TagFetcher::getFingerprint(const TrackPointer tio) {
    return ChromaPrinter(NULL).getFingerprint(tio);
}

void TagFetcher::startFetch(const TrackPointer track) {
    cancel();
    // qDebug() << "start to fetch track metadata";
    QList<TrackPointer> tracks;
    tracks.append(track);
    m_tracks = tracks;

    QFuture<QString> future = QtConcurrent::mapped(m_tracks, getFingerprint);
    m_pFingerprintWatcher = new QFutureWatcher<QString>(this);
    m_pFingerprintWatcher->setFuture(future);
    connect(m_pFingerprintWatcher, &QFutureWatcher<QString>::resultReadyAt,
            this, &TagFetcher::fingerprintFound);

    emit(fetchProgress(tr("Fingerprinting track")));
}

void TagFetcher::cancel() {
    // qDebug()<< "Cancel tagfetching";
    if (m_pFingerprintWatcher) {
        m_pFingerprintWatcher->cancel();

        delete m_pFingerprintWatcher;
        m_pFingerprintWatcher = NULL;
    }

    m_AcoustidClient.cancelAll();
    m_MusicbrainzClient.cancelAll();
    m_tracks.clear();
}

void TagFetcher::fingerprintFound(int index) {
    QFutureWatcher<QString>* watcher = reinterpret_cast<QFutureWatcher<QString>*>(sender());
    if (!watcher || index >= m_tracks.count()) {
        return;
    }

    const QString fingerprint = watcher->resultAt(index);
    const TrackPointer ptrack = m_tracks[index];

    if (fingerprint.isEmpty()) {
        emit(resultAvailable(ptrack, QList<TrackPointer>()));
        return;
    }

    emit(fetchProgress(tr("Identifying track")));
    // qDebug() << "start to look up the MBID";
    m_AcoustidClient.start(index, fingerprint, ptrack->getDurationInt());
}

void TagFetcher::mbidFound(int index, const QString& mbid) {
    if (index >= m_tracks.count()) {
        return;
    }

    const TrackPointer pTrack = m_tracks[index];

    if (mbid.isEmpty()) {
        emit(resultAvailable(pTrack, QList<TrackPointer>()));
        return;
    }

    emit fetchProgress(tr("Downloading Metadata"));
    //qDebug() << "start to fetch tags from MB";
    m_MusicbrainzClient.start(index, mbid);
}

void TagFetcher::tagsFetched(int index, const MusicBrainzClient::ResultList& results) {
    if (index >= m_tracks.count()) {
        return;
    }
    // qDebug() << "Tagfetcher got musicbrainz results and now parses them";
    const TrackPointer originalTrack = m_tracks[index];
    QList<TrackPointer> tracksGuessed;
    foreach (const MusicBrainzClient::Result& result, results) {
        TrackPointer track(
                Track::newTemporary(
                        originalTrack->getFileInfo(),
                        originalTrack->getSecurityToken()));
        track->setTitle(result.m_title);
        track->setArtist(result.m_artist);
        track->setAlbum(result.m_album);
        track->setDuration(result.m_duration);
        track->setTrackNumber(QString::number(result.m_track));
        track->setYear(QString::number(result.m_year));
        tracksGuessed << track;
    }
    emit(resultAvailable(originalTrack, tracksGuessed));
}
