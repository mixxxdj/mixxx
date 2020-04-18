#include "library/lastplayedcache.h"

#include <QSqlDatabase>
#include <QtDebug>

#include "library/dao/playlistdao.h"
#include "library/trackcollection.h"

const QString LASTPLAYEDTABLE_NAME = "last_played_datetimes";

QDateTime LastPlayedFetcher::fetch(TrackPointer pTrack) {
    m_fetchQuery.prepare(
            "  SELECT "
            "    datetime_played "
            "  FROM "
            "    " +
            LASTPLAYEDTABLE_NAME +
            "  WHERE "
            "    track_id = :trackid");
    m_fetchQuery.bindValue(":trackid", pTrack->getId().toVariant());
    if (!m_fetchQuery.exec()) {
        LOG_FAILED_QUERY(m_fetchQuery);
        return QDateTime();
    }
    if (!m_fetchQuery.first()) {
        return QDateTime();
    }
    return m_fetchQuery.value(0).toDateTime();
}

LastPlayedCache::LastPlayedCache(TrackCollection* trackCollection)
        : m_pTrackCollection(trackCollection),
          m_helper(trackCollection->database()) {
    initTableView();

    connect(&m_pTrackCollection->getPlaylistDAO(),
            &PlaylistDAO::trackAdded,
            this,
            &LastPlayedCache::slotPlaylistTrackChanged);
    connect(&m_pTrackCollection->getPlaylistDAO(),
            &PlaylistDAO::trackRemoved,
            this,
            &LastPlayedCache::slotPlaylistTrackChanged);
}

void LastPlayedCache::initTableView() {
    QSqlQuery lastPlayedQuery(m_pTrackCollection->database());
    // Views can't use params, so just use .args here.  There is no injection
    // risk because these are constants.
    lastPlayedQuery.prepare(QStringLiteral(
            "CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
            "SELECT "
            "  PlaylistTracks.track_id, "
            "  MAX(PlaylistTracks.pl_datetime_added) as datetime_played "
            "FROM PlaylistTracks "
            "JOIN Playlists ON PlaylistTracks.playlist_id == Playlists.id "
            "WHERE Playlists.hidden = %2 "
            "GROUP BY PlaylistTracks.track_id")
                                    .arg(LASTPLAYEDTABLE_NAME,
                                            QString::number(PlaylistDAO::
                                                            PLHT_SET_LOG)));
    if (!lastPlayedQuery.exec()) {
        LOG_FAILED_QUERY(lastPlayedQuery);
    }
}

void LastPlayedCache::slotPlaylistTrackChanged(
        int playlistId, TrackId trackId, int /* a_iPosition */) {
    if (m_pTrackCollection->getPlaylistDAO().getHiddenType(playlistId) !=
            PlaylistDAO::PLHT_SET_LOG) {
        return;
    }

    TrackPointer pTrack = m_pTrackCollection->getTrackById(trackId);
    if (pTrack) {
        pTrack->setLastPlayedDate(m_helper.fetch(pTrack));
    }
}
