#include "library/dao/playlistdao.h"

#include "moc_playlistdao.cpp"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QRandomGenerator>
#endif
#include <QtDebug>
#include <QtSql>

#include "library/autodj/autodjprocessor.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "track/track.h"
#include "util/compatibility.h"
#include "util/math.h"

PlaylistDAO::PlaylistDAO()
        : m_pAutoDJProcessor(nullptr) {
}

void PlaylistDAO::initialize(const QSqlDatabase& database) {
    DAO::initialize(database);
    populatePlaylistMembershipCache();
}

void PlaylistDAO::populatePlaylistMembershipCache() {
    // Minor optimization: reserve space in m_playlistsTrackIsIn.
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) from " PLAYLIST_TRACKS_TABLE);
    if (query.exec() && query.next()) {
        m_playlistsTrackIsIn.reserve(query.value(0).toInt());
    } else {
        LOG_FAILED_QUERY(query);
    }

    // now fetch all Tracks from all playlists and insert them into the hashmap
    query.prepare("SELECT track_id, playlist_id from " PLAYLIST_TRACKS_TABLE);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    const int trackIdColumn = query.record().indexOf(PLAYLISTTRACKSTABLE_TRACKID);
    const int playlistIdColumn = query.record().indexOf(PLAYLISTTRACKSTABLE_PLAYLISTID);
    while (query.next()) {
        m_playlistsTrackIsIn.insert(TrackId(query.value(trackIdColumn)),
                                    query.value(playlistIdColumn).toInt());

    }
}

int PlaylistDAO::createPlaylist(const QString& name, const HiddenType hidden) {
    //qDebug() << "PlaylistDAO::createPlaylist"
    //         << QThread::currentThread()
    //         << m_database.connectionName();
    // Start the transaction
    ScopedTransaction transaction(m_database);

    // Find out the highest position for the existing playlists so we know what
    // position this playlist should have.
    QSqlQuery query(m_database);
    query.prepare("SELECT max(position) as posmax FROM Playlists");

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return -1;
    }

    //Get the id of the last playlist.
    int position = 0;
    if (query.next()) {
        position = query.value(query.record().indexOf("posmax")).toInt();
        position++; // Append after the last playlist.
    }

    //qDebug() << "Inserting playlist" << name << "at position" << position;

    query.prepare("INSERT INTO Playlists (name, position, hidden, date_created, date_modified) "
                  "VALUES (:name, :position, :hidden,  CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)");
    query.bindValue(":name", name);
    query.bindValue(":position", position);
    query.bindValue(":hidden", static_cast<int>(hidden));

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return -1;
    }

    int playlistId = query.lastInsertId().toInt();
    // Commit the transaction
    transaction.commit();
    emit added(playlistId);
    return playlistId;
}

int PlaylistDAO::createUniquePlaylist(QString* pName, const HiddenType hidden) {
    int playlistId = getPlaylistIdFromName(*pName);
    int i = 1;

    if (playlistId != -1) {
        // Calculate a unique name
        *pName += "(%1)";
        while (playlistId != -1) {
            i++;
            playlistId = getPlaylistIdFromName(pName->arg(i));
        }
        *pName = pName->arg(i);
    }
    return createPlaylist(*pName, hidden);
}

QString PlaylistDAO::getPlaylistName(const int playlistId) const {
    //qDebug() << "PlaylistDAO::getPlaylistName" << QThread::currentThread() << m_database.connectionName();

    QSqlQuery query(m_database);
    query.prepare("SELECT name FROM Playlists "
                  "WHERE id= :id");
    query.bindValue(":id", playlistId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return "";
    }

    // Get the name field
    QString name = "";
    const int nameColumn = query.record().indexOf("name");
    if (query.next()) {
        name = query.value(nameColumn).toString();
    }
    return name;
}

QList<TrackId> PlaylistDAO::getTrackIds(const int playlistId) const {
    QList<TrackId> trackIds;

    QSqlQuery query(m_database);
    query.prepare("SELECT DISTINCT track_id FROM PlaylistTracks "
                  "WHERE playlist_id = :id");
    query.bindValue(":id", playlistId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return trackIds;
    }

    const int trackIdColumn = query.record().indexOf("track_id");
    while (query.next()) {
        trackIds.append(TrackId(query.value(trackIdColumn)));
    }
    return trackIds;
}

int PlaylistDAO::getPlaylistIdFromName(const QString& name) const {
    //qDebug() << "PlaylistDAO::getPlaylistIdFromName" << QThread::currentThread() << m_database.connectionName();

    QSqlQuery query(m_database);
    query.prepare("SELECT id FROM Playlists WHERE name = :name");
    query.bindValue(":name", name);
    if (query.exec()) {
        if (query.next()) {
            return query.value(query.record().indexOf("id")).toInt();
        }
    } else {
        LOG_FAILED_QUERY(query);
    }
    return -1;
}

void PlaylistDAO::deletePlaylist(const int playlistId) {
    //qDebug() << "PlaylistDAO::deletePlaylist" << QThread::currentThread() << m_database.connectionName();
    ScopedTransaction transaction(m_database);

    // Get the playlist id for this
    QSqlQuery query(m_database);

    // Delete the row in the Playlists table.
    query.prepare("DELETE FROM Playlists "
                  "WHERE id= :id");
    query.bindValue(":id", playlistId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    // Delete the tracks in this playlist from the PlaylistTracks table.
    query.prepare("DELETE FROM PlaylistTracks "
                  "WHERE playlist_id = :id");
    query.bindValue(":id", playlistId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    transaction.commit();
    //TODO: Crap, we need to shuffle the positions of all the playlists?

    for (QMultiHash<TrackId, int>::iterator it = m_playlistsTrackIsIn.begin();
         it != m_playlistsTrackIsIn.end();) {
        if (it.value() == playlistId) {
            it = m_playlistsTrackIsIn.erase(it);
        } else {
            ++it;
        }
    }

    emit deleted(playlistId);
}

void PlaylistDAO::renamePlaylist(const int playlistId, const QString& newName) {
    QSqlQuery query(m_database);
    query.prepare("UPDATE Playlists SET name = :name WHERE id = :id");
    query.bindValue(":name", newName);
    query.bindValue(":id", playlistId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }
    emit renamed(playlistId, newName);
}

bool PlaylistDAO::setPlaylistLocked(const int playlistId, const bool locked) {
    QSqlQuery query(m_database);
    query.prepare("UPDATE Playlists SET locked = :lock WHERE id = :id");
    // SQLite3 doesn't support boolean value. Using integer instead.
    int lock = locked ? 1 : 0;
    query.bindValue(":lock", lock);
    query.bindValue(":id", playlistId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    emit lockChanged(playlistId);
    return true;
}

bool PlaylistDAO::isPlaylistLocked(const int playlistId) const {
    QSqlQuery query(m_database);
    query.prepare("SELECT locked FROM Playlists WHERE id = :id");
    query.bindValue(":id", playlistId);

    if (query.exec()) {
        if (query.next()) {
            int lockValue = query.value(0).toInt();
            return lockValue == 1;
        }
    } else {
        LOG_FAILED_QUERY(query);
    }
    return false;
}

bool PlaylistDAO::removeTracksFromPlaylist(int playlistId, int startIndex) {
    // Retain the first track if it is loaded in a deck
    ScopedTransaction transaction(m_database);
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM PlaylistTracks "
                  "WHERE playlist_id=:id AND position>=:pos");
    query.bindValue(":id", playlistId);
    query.bindValue(":pos", startIndex);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    transaction.commit();
    emit tracksChanged(QSet<int>{playlistId});
    return true;
}

bool PlaylistDAO::appendTracksToPlaylist(const QList<TrackId>& trackIds, const int playlistId) {
    // qDebug() << "PlaylistDAO::appendTracksToPlaylist"
    //          << QThread::currentThread() << m_database.connectionName();

    // Start the transaction
    ScopedTransaction transaction(m_database);

    int position = getMaxPosition(playlistId);

    // Append after the last song. If no songs or a failed query then 0 becomes 1.
    ++position;

    //Insert the song into the PlaylistTracks table
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO PlaylistTracks (playlist_id, track_id, position, pl_datetime_added)"
                  "VALUES (:playlist_id, :track_id, :position, CURRENT_TIMESTAMP)");
    query.bindValue(":playlist_id", playlistId);


    int insertPosition = position;
    for (const auto& trackId: trackIds) {
        query.bindValue(":track_id", trackId.toVariant());
        query.bindValue(":position", insertPosition++);
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            return false;
        }
    }

    // Commit the transaction
    transaction.commit();

    insertPosition = position;
    for (const auto& trackId: trackIds) {
        m_playlistsTrackIsIn.insert(trackId, playlistId);
        // TODO(XXX) don't emit if the track didn't add successfully.
        emit trackAdded(playlistId, trackId, insertPosition++);
    }
    emit tracksChanged(QSet<int>{playlistId});
    return true;
}

bool PlaylistDAO::appendTrackToPlaylist(TrackId trackId, const int playlistId) {
    QList<TrackId> trackIds;
    trackIds.append(trackId);
    return appendTracksToPlaylist(trackIds, playlistId);
}

/** Find out how many playlists exist. */
unsigned int PlaylistDAO::playlistCount() const {
    // qDebug() << "PlaylistDAO::playlistCount" << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("SELECT count(*) as count FROM Playlists");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    int numRecords = 0;
    if (query.next()) {
        numRecords = query.value(query.record().indexOf("count")).toInt();
    }
    return numRecords;
}

int PlaylistDAO::getPlaylistId(const int index) const {
    //qDebug() << "PlaylistDAO::getPlaylistId"
    //         << QThread::currentThread() << m_database.connectionName();

    QSqlQuery query(m_database);
    query.prepare("SELECT id FROM Playlists");

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return -1;
    }

    int currentRow = 0;
    while(query.next()) {
        if (currentRow++ == index) {
            int id = query.value(0).toInt();
            return id;
        }
    }
    return -1;
}

PlaylistDAO::HiddenType PlaylistDAO::getHiddenType(const int playlistId) const {
    // qDebug() << "PlaylistDAO::getHiddenType"
    //          << QThread::currentThread() << m_database.connectionName();

    QSqlQuery query(m_database);
    query.prepare("SELECT hidden FROM Playlists WHERE id = :id");
    query.bindValue(":id", playlistId);

    if (query.exec()) {
        if (query.next()) {
            return static_cast<HiddenType>(query.value(0).toInt());
        }
    } else {
        LOG_FAILED_QUERY(query);
    }
    qDebug() << "PlaylistDAO::getHiddenType returns PLHT_UNKNOWN for playlistId "
             << playlistId;
    return PLHT_UNKNOWN;
}

bool PlaylistDAO::isHidden(const int playlistId) const {
    // qDebug() << "PlaylistDAO::isHidden"
    //          << QThread::currentThread() << m_database.connectionName();

    HiddenType ht = getHiddenType(playlistId);
    if (ht == PLHT_NOT_HIDDEN) {
        return false;
    }
    return true;
}


void PlaylistDAO::removeHiddenTracks(const int playlistId) {
    ScopedTransaction transaction(m_database);
    // This query deletes all tracks marked as deleted and all
    // phantom track_ids with no match in the library table
    QString queryString = QString(
        "SELECT position FROM PlaylistTracks "
        "WHERE PlaylistTracks.id NOT IN ("
        "SELECT PlaylistTracks.id "
        "FROM PlaylistTracks "
        "INNER JOIN library ON library.id = PlaylistTracks.track_id "
        "WHERE PlaylistTracks.playlist_id = %1 "
        "AND library.mixxx_deleted = 0 ) "
        "AND PlaylistTracks.playlist_id = %1")
            .arg(QString::number(playlistId));

    QSqlQuery query(m_database);
    if (!query.prepare(queryString)) {
        LOG_FAILED_QUERY(query);
        return;
    }

    query.setForwardOnly(true);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    while (query.next()) {
        int position = query.value(query.record().indexOf("position")).toInt();
        removeTracksFromPlaylistInner(playlistId, position);
    }

    transaction.commit();
    emit tracksChanged(QSet<int>{playlistId});
}


void PlaylistDAO::removeTracksFromPlaylistById(int playlistId, TrackId trackId) {
    ScopedTransaction transaction(m_database);
    removeTracksFromPlaylistByIdInner(playlistId, trackId);
    transaction.commit();
    emit tracksChanged(QSet<int>{playlistId});
}

void PlaylistDAO::removeTracksFromPlaylistByIdInner(int playlistId, TrackId trackId) {
    QSqlQuery query(m_database);
    query.prepare("SELECT position FROM PlaylistTracks WHERE playlist_id=:id "
                "AND track_id=:track_id");
    query.bindValue(":id", playlistId);
    query.bindValue(":track_id", trackId.toVariant());

    query.setForwardOnly(true);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    while (query.next()) {
        int position = query.value(query.record().indexOf("position")).toInt();
        removeTracksFromPlaylistInner(playlistId, position);
    }
}


void PlaylistDAO::removeTrackFromPlaylist(int playlistId, int position) {
    // qDebug() << "PlaylistDAO::removeTrackFromPlaylist"
    //          << QThread::currentThread() << m_database.connectionName();
    ScopedTransaction transaction(m_database);
    removeTracksFromPlaylistInner(playlistId, position);
    transaction.commit();
    emit tracksChanged(QSet<int>{playlistId});
}

void PlaylistDAO::removeTracksFromPlaylist(int playlistId, const QList<int>& positions) {
    // get positions in reversed order
    auto sortedPositons = positions;
    std::sort(sortedPositons.begin(), sortedPositons.end(), std::greater<int>());

    //qDebug() << "PlaylistDAO::removeTrackFromPlaylist"
    //         << QThread::currentThread() << m_database.connectionName();
    ScopedTransaction transaction(m_database);
    for (const auto position : qAsConst(sortedPositons)) {
        removeTracksFromPlaylistInner(playlistId, position);
    }
    transaction.commit();
    emit tracksChanged(QSet<int>{playlistId});
}

void PlaylistDAO::removeTracksFromPlaylistInner(int playlistId, int position) {
    QSqlQuery query(m_database);
    query.prepare("SELECT track_id FROM PlaylistTracks WHERE playlist_id=:id "
                  "AND position=:position");
    query.bindValue(":id", playlistId);
    query.bindValue(":position", position);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    if (!query.next()) {
        qDebug() << "removeTrackFromPlaylist no track exists at position:"
                 << position << "in playlist:" << playlistId;
        return;
    }
    TrackId trackId(query.value(query.record().indexOf("track_id")));

    // Delete the track from the playlist.
    query.prepare("DELETE FROM PlaylistTracks "
                  "WHERE playlist_id=:id AND position= :position");
    query.bindValue(":id", playlistId);
    query.bindValue(":position", position);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    QString queryString;
    queryString = QString("UPDATE PlaylistTracks SET position=position-1 "
                          "WHERE position>=%1 AND "
                          "playlist_id=%2").arg(QString::number(position),
                                                QString::number(playlistId));
    if (!query.exec(queryString)) {
        LOG_FAILED_QUERY(query);
    }

    m_playlistsTrackIsIn.remove(trackId, playlistId);
    emit trackRemoved(playlistId, trackId, position);
}



bool PlaylistDAO::insertTrackIntoPlaylist(TrackId trackId, const int playlistId, int position) {
    if (playlistId < 0 || !trackId.isValid() || position < 0) {
        return false;
    }

    ScopedTransaction transaction(m_database);

    int max_position = getMaxPosition(playlistId) + 1;

    if (position > max_position) {
        position = max_position;
    }

    // Move all the tracks in the playlist up by one
    QString queryString =
            QString("UPDATE PlaylistTracks SET position=position+1 "
                    "WHERE position>=%1 AND "
                    "playlist_id=%2").arg(QString::number(position),
                                          QString::number(playlistId));

    QSqlQuery query(m_database);
    if (!query.exec(queryString)) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    //Insert the song into the PlaylistTracks table
    query.prepare("INSERT INTO PlaylistTracks (playlist_id, track_id, position, pl_datetime_added)"
                  "VALUES (:playlist_id, :track_id, :position, CURRENT_TIMESTAMP)");
    query.bindValue(":playlist_id", playlistId);
    query.bindValue(":track_id", trackId.toVariant());
    query.bindValue(":position", position);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    transaction.commit();

    m_playlistsTrackIsIn.insert(trackId, playlistId);
    emit trackAdded(playlistId, trackId, position);
    emit tracksChanged(QSet<int>{playlistId});
    return true;
}

int PlaylistDAO::insertTracksIntoPlaylist(const QList<TrackId>& trackIds,
                                          const int playlistId, int position) {
    if (playlistId < 0 || position < 0) {
        return 0;
    }

    int tracksAdded = 0;
    ScopedTransaction transaction(m_database);

    int max_position = getMaxPosition(playlistId) + 1;

    if (position > max_position) {
        position = max_position;
    }

    QSqlQuery insertQuery(m_database);
    insertQuery.prepare("INSERT INTO PlaylistTracks (playlist_id, track_id, position)"
                        "VALUES (:playlist_id, :track_id, :position)");
    QSqlQuery query(m_database);
    int insertPositon = position;
    for (const auto& trackId: trackIds) {
        if (!trackId.isValid()) {
            continue;
        }
        // Move all tracks in playlist up by 1.
        // TODO(XXX) We could do this in one query before the for loop.
        query.prepare(QString("UPDATE PlaylistTracks SET position=position+1 "
                              "WHERE position>=%1 AND "
                              "playlist_id=%2").arg(insertPositon).arg(playlistId));

        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            continue;
        }

        // Insert the track at the given position
        insertQuery.bindValue(":playlist_id", playlistId);
        insertQuery.bindValue(":track_id", trackId.toVariant());
        insertQuery.bindValue(":position", insertPositon);
        if (!insertQuery.exec()) {
            LOG_FAILED_QUERY(insertQuery);
            continue;
        }

        // Increment the insert position for the track.
        ++insertPositon;
        ++tracksAdded;
    }

    transaction.commit();

    insertPositon = position;
    for (const auto& trackId: trackIds) {
        m_playlistsTrackIsIn.insert(trackId, playlistId);
        // TODO(XXX) The position is wrong if any track failed to insert.
        emit trackAdded(playlistId, trackId, insertPositon++);
    }
    emit tracksChanged(QSet<int>{playlistId});
    return tracksAdded;
}

void PlaylistDAO::addPlaylistToAutoDJQueue(const int playlistId, AutoDJSendLoc loc) {
    //qDebug() << "Adding tracks from playlist " << playlistId << " to the Auto-DJ Queue";

    // Query the PlaylistTracks database to locate tracks in the selected
    // playlist. Tracks are automatically sorted by position.
    QSqlQuery query(m_database);
    query.prepare("SELECT track_id FROM PlaylistTracks "
                  "WHERE playlist_id = :plid ORDER BY position ASC");
    query.bindValue(":plid", playlistId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    // Loop through the tracks, adding them to the Auto-DJ Queue. Start at
    // position 2 because position 1 was already loaded to the deck.
    QList<TrackId> trackIds;
    while (query.next()) {
        trackIds.append(TrackId(query.value(0)));
    }
    addTracksToAutoDJQueue(trackIds, loc);
}

int PlaylistDAO::getPreviousPlaylist(const int currentPlaylistId, HiddenType hidden) const {
    // Find out the highest position existing in the playlist so we know what
    // position this track should have.
    QSqlQuery query(m_database);
    query.prepare("SELECT max(id) as id FROM Playlists "
                  "WHERE id < :id AND hidden = :hidden");
    query.bindValue(":id", currentPlaylistId);
    query.bindValue(":hidden", hidden);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return -1;
    }

     // Get the id of the highest playlist
    int previousPlaylistId = -1;
    if (query.next()) {
        previousPlaylistId = query.value(query.record().indexOf("id")).toInt();
    }
    return previousPlaylistId;
}

bool PlaylistDAO::copyPlaylistTracks(const int sourcePlaylistID, const int targetPlaylistID) {
    // Start the transaction
    ScopedTransaction transaction(m_database);

    // Copy the new tracks after the last track in the target playlist.
    int positionOffset = getMaxPosition(targetPlaylistID);

    // Copy the tracks from one playlist to another, adjusting the position of
    // each copied track, and preserving the date/time added.
    // INSERT INTO PlaylistTracks (playlist_id, track_id, position, pl_datetime_added) SELECT :target_plid, track_id, position + :position_offset, pl_datetime_added FROM PlaylistTracks WHERE playlist_id = :source_plid;
    QSqlQuery query(m_database);
    query.prepare(QString("INSERT INTO " PLAYLIST_TRACKS_TABLE
                          " (%1, %2, %3, %4) SELECT :target_plid, %2, "
                          "%3 + :position_offset, %4 FROM " PLAYLIST_TRACKS_TABLE
                          " WHERE %1 = :source_plid")
                          .arg(
                                  PLAYLISTTRACKSTABLE_PLAYLISTID,      // %1
                                  PLAYLISTTRACKSTABLE_TRACKID,         // %2
                                  PLAYLISTTRACKSTABLE_POSITION,        // %3
                                  PLAYLISTTRACKSTABLE_DATETIMEADDED)); // %4
    query.bindValue(":position_offset", positionOffset);
    query.bindValue(":source_plid", sourcePlaylistID);
    query.bindValue(":target_plid", targetPlaylistID);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    // Query each added track and its new position.
    // SELECT track_id, position FROM PlaylistTracks WHERE playlist_id = :target_plid AND position > :position_offset;
    query.prepare(QString("SELECT %2, %3 FROM " PLAYLIST_TRACKS_TABLE
                          " WHERE %1 = :target_plid AND %3 > :position_offset")
                          .arg(
                                  PLAYLISTTRACKSTABLE_PLAYLISTID, // %1
                                  PLAYLISTTRACKSTABLE_TRACKID,    // %2
                                  PLAYLISTTRACKSTABLE_POSITION)); // %3
    query.bindValue(":target_plid", targetPlaylistID);
    query.bindValue(":position_offset", positionOffset);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    // Commit the transaction
    transaction.commit();

    // Let subscribers know about each added track.
    while (query.next()) {
        TrackId copiedTrackId(query.value(0));
        int copiedPosition = query.value(1).toInt();
        m_playlistsTrackIsIn.insert(copiedTrackId, targetPlaylistID);
        emit trackAdded(targetPlaylistID, copiedTrackId, copiedPosition);
    }
    emit tracksChanged(QSet<int>{targetPlaylistID});
    return true;
}

int PlaylistDAO::getMaxPosition(const int playlistId) const {
    // Find out the highest position existing in the playlist so we know what
    // position this track should have.
    QSqlQuery query(m_database);
    query.prepare("SELECT max(position) as position FROM PlaylistTracks "
                  "WHERE playlist_id = :id");
    query.bindValue(":id", playlistId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    // Get the position of the highest track in the playlist.
    int position = 0;
    if (query.next()) {
        position = query.value(query.record().indexOf("position")).toInt();
    }
    return position;
}

void PlaylistDAO::removeTracksFromPlaylists(const QList<TrackId>& trackIds) {
    // copy the hash, because there is no guarantee that "it" is valid after remove
    QMultiHash<TrackId, int> playlistsTrackIsInCopy = m_playlistsTrackIsIn;
    QSet<int> playlistIds;

    ScopedTransaction transaction(m_database);
    for (const auto& trackId : trackIds) {
        for (auto it = playlistsTrackIsInCopy.constBegin();
                it != playlistsTrackIsInCopy.constEnd(); ++it) {
            if (it.key() == trackId) {
                const auto playlistId = it.value();
                removeTracksFromPlaylistByIdInner(playlistId, trackId);
                playlistIds.insert(playlistId);
            }
        }
    }
    transaction.commit();

    emit tracksChanged(playlistIds);
}

int PlaylistDAO::tracksInPlaylist(const int playlistId) const {
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(id) AS count FROM PlaylistTracks "
                  "WHERE playlist_id = :playlist_id");
    query.bindValue(":playlist_id", playlistId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Couldn't get the number of tracks in playlist"
                                << playlistId;
        return -1;
    }
    int count = -1;
    const int countColumn = query.record().indexOf("count");
    while (query.next()) {
        count = query.value(countColumn).toInt();
    }
    return count;
}

void PlaylistDAO::moveTrack(const int playlistId, const int oldPosition, const int newPosition) {
    ScopedTransaction transaction(m_database);
    QSqlQuery query(m_database);

    // Algorithm for code below
    // Case 1: destination < source (newPositon < oldPosition)
    //    1) Set position = -1 where pos=source -- Gives that track a dummy index to keep stuff simple.
    //    2) Decrement position where pos >= dest AND pos < source
    //    3) Set position = dest where pos=-1 -- Move track from dummy pos to final destination.

     // Case 2: destination > source (newPos > oldPos)
     //   1) Set position=-1 where pos=source -- Give track a dummy index again.
     //   2) Decrement position where pos > source AND pos <= dest
     //   3) Set position=dest where pos=-1 -- Move that track from dummy pos to final destination

    QString queryString;

    // Move moved track to dummy position -1
    queryString = QString("UPDATE PlaylistTracks SET position=-1 "
                          "WHERE position=%1 AND "
                          "playlist_id=%2").arg(QString::number(oldPosition),
                                                QString::number(playlistId));
    query.exec(queryString);

    if (newPosition < oldPosition) {
        queryString = QString("UPDATE PlaylistTracks SET position=position+1 "
                              "WHERE position >= %1 AND position < %2 AND "
                              "playlist_id=%3").arg(QString::number(newPosition),
                                                    QString::number(oldPosition),
                                                    QString::number(playlistId));
    } else {
        queryString = QString("UPDATE PlaylistTracks SET position=position-1 "
                              "WHERE position>%1 AND position<=%2 AND "
                              "playlist_id=%3").arg(QString::number(oldPosition),
                                                    QString::number(newPosition),
                                                    QString::number(playlistId));
    }
    query.exec(queryString);

    queryString = QString("UPDATE PlaylistTracks SET position = %1 "
                          "WHERE position=-1 AND "
                          "playlist_id=%2").arg(QString::number(newPosition),
                                                QString::number(playlistId));
    query.exec(queryString);

    transaction.commit();

    // Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
        qDebug() << query.lastError();
    }

    emit tracksChanged(QSet<int>{playlistId});
}

void PlaylistDAO::searchForDuplicateTrack(const int fromPosition,
                                          const int toPosition,
                                          TrackId trackID,
                                          const int excludePosition,
                                          const int otherTrackPosition,
                                          const QHash<int,TrackId>* pTrackPositionIds,
                                          int* pTrackDistance) {
    //qDebug() << "        Searching from " << fromPosition << " to " << toPosition;
    for (int pos = fromPosition; pos <= toPosition; pos++) {
        if (pTrackPositionIds->value(pos) == trackID &&
                pos != excludePosition) {
            int tempTrackDistance =
                    (otherTrackPosition - pos) * (otherTrackPosition - pos);
            if (tempTrackDistance < *pTrackDistance || *pTrackDistance == -1) {
                *pTrackDistance = tempTrackDistance;
            }
        }
    }
}

void PlaylistDAO::shuffleTracks(const int playlistId, const QList<int>& positions, const QHash<int,TrackId>& allIds) {
    ScopedTransaction transaction(m_database);
    QSqlQuery query(m_database);

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    // Seed the randomness generator
    qsrand(QDateTime::currentDateTimeUtc().toTime_t());
#endif
    QHash<int,TrackId> trackPositionIds = allIds;
    QList<int> newPositions = positions;
    const int searchDistance = math_max(trackPositionIds.count() / 4, 1);

    qDebug() << "Shuffling Tracks";
    qDebug() << "*** Search Distance: " << searchDistance;
    //for (int z = 0; z < positions.count(); z++) {
        //qDebug() << "*** Position: " << positions[z] << " | ID: " << allIds.value(positions[z]);
    //}

    // This is a modified Fisher-Yates shuffling algorithm.
    //
    // Description of the algorithm below:
    //
    // Loop through the set of tracks to be shuffled:
    //     1) Set Track A as the current point in the shuffle set
    //     2) Repeat a maximum of 10 times or until a good place (1/4 of the
    //        playlist away from a conflict) is reached:
    //         a) Pick a random track within the shuffle set (Track B)
    //         b) Check 1/4 of the playlist up and down (wrapped at the
    //            beginning and end) from Track B's position for Track A
    //         c) Check 1/4 of the playlist up and down (wrapped at the
    //            beginning and end) from Track A's position for Track B
    //         d) If there was a conflict, store the position if it was better
    //            than the already stored best position. The position is deemed
    //            "better" if the distance (square of the difference) of
    //            the closest conflict (Track B near Track A's position and vv)
    //            is larger than previous iterations.
    //     3) If no good place was found, use the stored best position
    //     4) Swap Track A and Track B

    for (int i = 0; i < newPositions.count(); i++) {
        bool conflictFound = true;
        int trackAPosition = newPositions.at(i);
        TrackId trackAId = trackPositionIds.value(trackAPosition);
        int trackBPosition = -1;
        TrackId trackBId;
        int bestTrackDistance = -1;
        int bestTrackBPosition = -1;

        //qDebug() << "Track A:";
        //qDebug() << "Position: " << trackAPosition << " | Id: " << trackAId;

        for (int limit = 10; limit > 0 && conflictFound; limit--) {
            int randomShuffleSetIndex = static_cast<int>(
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
                    QRandomGenerator::global()->generateDouble() *
#else
                    (qrand() / (RAND_MAX + 1.0)) *
#endif
                    newPositions.count());

            trackBPosition = positions.at(randomShuffleSetIndex);
            trackBId = trackPositionIds.value(trackBPosition);
            int trackDistance = -1;
            int playlistEnd = trackPositionIds.count();

            //qDebug() << "    Trying new Track B:";
            //qDebug() << "        Position: " << trackBPosition << " | Id: " <<trackBId;

            // Search around Track B for Track A
            searchForDuplicateTrack(
                    math_clamp(trackBPosition - searchDistance, 0, playlistEnd),
                    math_clamp(trackBPosition + searchDistance, 0, playlistEnd),
                    trackAId, trackAPosition, trackBPosition,
                    &trackPositionIds, &trackDistance);
            // Wrap search if needed
            if (trackBPosition - searchDistance < 1) {
                searchForDuplicateTrack(
                        playlistEnd + (trackBPosition - searchDistance),
                        playlistEnd,
                        trackAId, trackAPosition, trackBPosition,
                        &trackPositionIds, &trackDistance);
            }
            if (trackBPosition + searchDistance > playlistEnd) {
                searchForDuplicateTrack(
                        1,
                        (trackBPosition + searchDistance) - playlistEnd,
                        trackAId, trackAPosition, trackBPosition,
                        &trackPositionIds, &trackDistance);
            }
            // Search around Track A for Track B
            searchForDuplicateTrack(
                    math_clamp(trackAPosition - searchDistance, 0, playlistEnd),
                    math_clamp(trackAPosition + searchDistance, 0, playlistEnd),
                    trackBId, trackBPosition, trackAPosition,
                    &trackPositionIds, &trackDistance);
            // Wrap search if needed
            if (trackAPosition - searchDistance < 1) {
                searchForDuplicateTrack(
                        playlistEnd + (trackAPosition - searchDistance),
                        playlistEnd,
                        trackBId, trackBPosition, trackAPosition,
                        &trackPositionIds, &trackDistance);
            }
            if (trackAPosition + searchDistance > playlistEnd) {
                searchForDuplicateTrack(
                        1,
                        (trackAPosition + searchDistance) - playlistEnd,
                        trackBId, trackBPosition, trackAPosition,
                        &trackPositionIds, &trackDistance);
            }

            conflictFound = trackDistance != -1;
            //qDebug() << "            Conflict found? " << conflictFound;
            if (bestTrackDistance < trackDistance) {
                bestTrackDistance = trackDistance;
                bestTrackBPosition = trackBPosition;
            }
            //qDebug() << "        Current Best Position: " << bestTrackBPosition << " | Distance: " << bestTrackDistance;
        }

        if (conflictFound) {
            if (bestTrackBPosition > -1) {
                trackBPosition = bestTrackBPosition;
                trackBId = trackPositionIds.value(trackBPosition);
            }
        }

        //qDebug() << "Swapping tracks " << trackAPosition << " and " << trackBPosition;
        trackPositionIds.insert(trackAPosition, trackBId);
        trackPositionIds.insert(trackBPosition, trackAId);

        // TODO: The following use of QList<T>::swap(int, int) is deprecated
        // and should be replaced with QList<T>::swapItemsAt(int, int)
        // However, the proposed alternative has just been introduced in Qt
        // 5.13. Until the minimum required Qt version of Mixxx is increased,
        // we need a version check here.
        #if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
        newPositions.swap(newPositions.indexOf(trackAPosition),
                          newPositions.indexOf(trackBPosition));
        #else
        newPositions.swapItemsAt(newPositions.indexOf(trackAPosition),
                                 newPositions.indexOf(trackBPosition));
        #endif

        QString swapQuery = "UPDATE PlaylistTracks SET position=%1 "
                "WHERE position=%2 AND playlist_id=%3";
        query.exec(swapQuery.arg(QString::number(-1),
                                 QString::number(trackAPosition),
                                 QString::number(playlistId)));
        query.exec(swapQuery.arg(QString::number(trackAPosition),
                                 QString::number(trackBPosition),
                                 QString::number(playlistId)));
        query.exec(swapQuery.arg(QString::number(trackBPosition),
                                 QString::number(-1),
                                 QString::number(playlistId)));

        if (query.lastError().isValid()) {
            qDebug() << query.lastError();
        }
    }

    transaction.commit();
    emit tracksChanged(QSet<int>{playlistId});
}

bool PlaylistDAO::isTrackInPlaylist(TrackId trackId, const int playlistId) const {
    return m_playlistsTrackIsIn.contains(trackId, playlistId);
}

void PlaylistDAO::getPlaylistsTrackIsIn(TrackId trackId,
                                        QSet<int>* playlistSet) const {
    playlistSet->clear();
    for (auto it = m_playlistsTrackIsIn.constFind(trackId);
         it != m_playlistsTrackIsIn.constEnd() && it.key() == trackId; ++it) {
        playlistSet->insert(it.value());
    }
}

void PlaylistDAO::setAutoDJProcessor(AutoDJProcessor* pAutoDJProcessor) {
    m_pAutoDJProcessor = pAutoDJProcessor;
}

void PlaylistDAO::addTracksToAutoDJQueue(const QList<TrackId>& trackIds, AutoDJSendLoc loc) {
    int iAutoDJPlaylistId = getPlaylistIdFromName(AUTODJ_TABLE);
    if (iAutoDJPlaylistId == -1) {
        return;
    }

    // If the first track is already loaded to the player,
    // alter the playlist only below the first track
    int position =
        (m_pAutoDJProcessor && m_pAutoDJProcessor->nextTrackLoaded()) ? 2 : 1;

    switch (loc) {
        case AutoDJSendLoc::TOP:
            insertTracksIntoPlaylist(trackIds, iAutoDJPlaylistId, position);
            break;
        case AutoDJSendLoc::BOTTOM:
            appendTracksToPlaylist(trackIds, iAutoDJPlaylistId);
            break;
        case AutoDJSendLoc::REPLACE:
            if (removeTracksFromPlaylist(iAutoDJPlaylistId, position)) {
                appendTracksToPlaylist(trackIds, iAutoDJPlaylistId);
            }
            break;
    }
}
