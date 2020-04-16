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
    // XXX: why isn't value binding working???
    QSqlQuery lastPlayedQuery(m_pTrackCollection->database());
    lastPlayedQuery.prepare(
            "CREATE TEMPORARY VIEW IF NOT EXISTS last_played AS "
            "SELECT "
            "  PlaylistTracks.track_id, "
            "  MAX(PlaylistTracks.pl_datetime_added) as datetime_played "
            "FROM PlaylistTracks "
            "JOIN Playlists ON PlaylistTracks.playlist_id == Playlists.id "
            "WHERE Playlists.hidden = :setlogid "
            "GROUP BY PlaylistTracks.track_id");
    // .arg(LASTPLAYEDTABLE_NAME));
    lastPlayedQuery.bindValue(":setlogid", PlaylistDAO::PLHT_SET_LOG);
    if (!lastPlayedQuery.exec()) {
        LOG_FAILED_QUERY(lastPlayedQuery);
    }
}

void LastPlayedCache::slotPlaylistTrackChanged(
        int playlistId, TrackId trackId, int /* a_iPosition */) {
    // qDebug() << "LastPlayedCache::slotPlaylistTrackChanged() playlistId:" << playlistId;
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
            "    track_id = :trackid ");
    // .arg(pTrack->getId().toString());
    qDebug() << "yesSss????????????? " << updateQuery.prepare(queryString);
    updateQuery.bindValue(":trackid", pTrack->getId().toVariant(), QSql::Out);
    if (!updateQuery.exec()) {
        LOG_FAILED_QUERY(updateQuery);
    }
    const int col = updateQuery.record().indexOf("datetime_played");
    updateQuery.first();
    return updateQuery.value(col).toDateTime();
}
