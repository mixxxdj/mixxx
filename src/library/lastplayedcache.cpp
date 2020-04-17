#include "library/lastplayedcache.h"

#include <QSqlDatabase>
#include <QtDebug>

#include "library/dao/playlistdao.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"

LastPlayedCache::LastPlayedCache(TrackCollection* trackCollection)
        : m_pTrackCollection(trackCollection) {
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
    pTrack->setLastPlayedDate(fetchLastPlayedTime(m_pTrackCollection->database(), pTrack));
}

// static, so that trackdao.cpp can fetch values for initial population in the cache.
QDateTime LastPlayedCache::fetchLastPlayedTime(const QSqlDatabase& db, TrackPointer pTrack) {
    QSqlQuery updateQuery(db);
    const QString queryString = QString(
            "  SELECT "
            "    datetime_played "
            "  FROM "
            "    last_played "
            "  WHERE "
            "    track_id = :trackid");
    updateQuery.prepare(queryString);
    updateQuery.bindValue(":trackid", pTrack->getId().toVariant());
    if (!updateQuery.exec()) {
        LOG_FAILED_QUERY(updateQuery);
    }
    updateQuery.first();
    return updateQuery.value(0).toDateTime();
}
