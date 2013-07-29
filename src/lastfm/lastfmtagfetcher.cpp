#include <QDebug>
#include "trackinfoobject.h"
#include "track/tagutils.h"
#include "lastfm/lastfmtagfetcher.h"
#include "lastfm/lastfmclient.h"

LastFmTagFetcher::LastFmTagFetcher(QObject *parent)
                : QObject(parent),
                  m_LastFmClient(this) {
    connect(&m_LastFmClient, SIGNAL(finished(int, const TagCounts&)),
            this, SLOT(tagsFetched(int, const TagCounts&)));
}

void LastFmTagFetcher::startFetch(const TrackPointer track) {
    cancel();
    QList<TrackPointer> tracks;
    tracks.append(track);
    m_tracks = tracks;

//    foreach (const TrackPointer pTrack, m_tracks) {
//        emit fetchProgress(tr("Searching last.fm for track"));
//    }
    for (int i = 0; i < m_tracks.size(); i++) {
        QString artist = m_tracks[i]->getArtist();
//        m_LastFmClient.start(i, artist);
        QString title = m_tracks[i]->getTitle();
        m_LastFmClient.start(i, artist, title);
    }
}

void LastFmTagFetcher::cancel() {
    m_LastFmClient.cancelAll();
    m_tracks.clear();
}

void LastFmTagFetcher::tagsFetched(int index,
                                   const TagCounts& tags) {
    if (index >= m_tracks.count()) {
        return;
    }
    qDebug() << "LastFmTagFetcher retrieved tags";
    const TrackPointer pTrack = m_tracks[index];
    pTrack->setTags(tags);
}
