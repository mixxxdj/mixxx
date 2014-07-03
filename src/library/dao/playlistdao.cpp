#include <QtDebug>
#include <QtSql>

#include "trackinfoobject.h"
#include "library/dao/playlistdao.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"

PlaylistDAO::PlaylistDAO(QSqlDatabase& database)
        : m_database(database) {
}

PlaylistDAO::~PlaylistDAO() {
}

void PlaylistDAO::initialize() {
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
    emit(added(playlistId));
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

QList<int> PlaylistDAO::getTrackIds(const int playlistId) const {
    QSqlQuery query(m_database);
    query.prepare("SELECT DISTINCT track_id FROM PlaylistTracks "
                  "WHERE playlist_id = :id");
    query.bindValue(":id", playlistId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return QList<int> ();
    }

    QList<int> ids;
    const int trackIdColumn = query.record().indexOf("track_id");
    while (query.next()) {
        ids.append(query.value(trackIdColumn).toInt());
    }
    return ids;
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

    emit(deleted(playlistId));
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
    emit(renamed(playlistId, newName));
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
    emit(lockChanged(playlistId));
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

bool PlaylistDAO::appendTracksToPlaylist(const QList<int>& trackIds, const int playlistId) {
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
    foreach (int trackId, trackIds) {
        query.bindValue(":track_id", trackId);
        query.bindValue(":position", insertPosition++);
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            return false;
        }
    }

    // Commit the transaction
    transaction.commit();

    insertPosition = position;
    foreach (int trackId, trackIds) {
        // TODO(XXX) don't emit if the track didn't add successfully.
        emit(trackAdded(playlistId, trackId, insertPosition++));
    }
    emit(changed(playlistId));
    return true;
}

bool PlaylistDAO::appendTrackToPlaylist(const int trackId, const int playlistId) {
    QList<int> tracks;
    tracks.append(trackId);
    return appendTracksToPlaylist(tracks, playlistId);
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

void PlaylistDAO::removeTrackFromPlaylist(const int playlistId, const int position)
{
    // qDebug() << "PlaylistDAO::removeTrackFromPlaylist"
    //          << QThread::currentThread() << m_database.connectionName();
    ScopedTransaction transaction(m_database);
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
    int trackId = query.value(query.record().indexOf("track_id")).toInt();

    //Delete the track from the playlist.
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
    transaction.commit();

    emit(trackRemoved(playlistId, trackId, position));
    emit(changed(playlistId));
}

void PlaylistDAO::removeTracksFromPlaylist(const int playlistId, QList<int>& positions) {
    // get positions in reversed order
    qSort(positions.begin(), positions.end(), qGreater<int>());

    //qDebug() << "PlaylistDAO::removeTrackFromPlaylist"
    //         << QThread::currentThread() << m_database.connectionName();
    ScopedTransaction transaction(m_database);
    QSqlQuery query(m_database);
    foreach (int position , positions) {
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
        int trackId = query.value(query.record().indexOf("track_id")).toInt();

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

        emit(trackRemoved(playlistId, trackId, position));
    }
    transaction.commit();
    emit(changed(playlistId));
}

bool PlaylistDAO::insertTrackIntoPlaylist(const int trackId, const int playlistId, int position) {
    if (playlistId < 0 || trackId < 0 || position < 0)
        return false;

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
    query.bindValue(":track_id", trackId);
    query.bindValue(":position", position);


    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    transaction.commit();

    emit(trackAdded(playlistId, trackId, position));
    emit(changed(playlistId));
    return true;
}

int PlaylistDAO::insertTracksIntoPlaylist(const QList<int>& trackIds, const int playlistId, int position) {
    if (playlistId < 0 || position < 0 ) {
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
    foreach (int trackId, trackIds) {
        if (trackId < 0) {
            continue;
        }
        // Move all tracks in playlist up by 1.
        query.prepare(QString("UPDATE PlaylistTracks SET position=position+1 "
                              "WHERE position>=%1 AND "
                              "playlist_id=%2").arg(insertPositon).arg(playlistId));

        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            continue;
        }

        // Insert the track at the given position
        insertQuery.bindValue(":playlist_id", playlistId);
        insertQuery.bindValue(":track_id", trackId);
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
    foreach (int trackId, trackIds) {
        // TODO(XXX) The position is wrong if any track failed to insert.
        emit(trackAdded(playlistId, trackId, insertPositon++));
    }
    emit(changed(playlistId));
    return tracksAdded;
}

void PlaylistDAO::addPlaylistToAutoDJQueue(const int playlistId, const bool bTop) {
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
    QList<int> ids;
    while (query.next()) {
        ids.append(query.value(0).toInt());
    }
    addTracksToAutoDJQueue(ids, bTop);
}

void PlaylistDAO::addTracksToAutoDJQueue(const QList<int>& trackIds, const bool bTop) {
    // Get the ID of the Auto-DJ playlist
    int autoDJId = getPlaylistIdFromName(AUTODJ_TABLE);

    if (bTop) {
        // Start at position 2 because position 1 might be already loaded to the deck.
        insertTracksIntoPlaylist(trackIds, autoDJId, 2);
    } else {
        appendTracksToPlaylist(trackIds, autoDJId);
    }
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
        .arg(PLAYLISTTRACKSTABLE_PLAYLISTID)        // %1
        .arg(PLAYLISTTRACKSTABLE_TRACKID)           // %2
        .arg(PLAYLISTTRACKSTABLE_POSITION)          // %3
        .arg(PLAYLISTTRACKSTABLE_DATETIMEADDED));   // %4
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
        .arg(PLAYLISTTRACKSTABLE_PLAYLISTID)    // %1
        .arg(PLAYLISTTRACKSTABLE_TRACKID)       // %2
        .arg(PLAYLISTTRACKSTABLE_POSITION));    // %3
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
        int copiedTrackId = query.value(0).toInt();
        int copiedPosition = query.value(1).toInt();
        emit(trackAdded(targetPlaylistID, copiedTrackId, copiedPosition));
    }
    emit(changed(targetPlaylistID));
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

void PlaylistDAO::removeTracksFromPlaylists(const QList<int>& trackIds) {
    QStringList trackIdList;
    foreach (int id, trackIds) {
        if (trackIdList.count() >= 255) {
            // Avoid that the resulting SQL query to exceed the maximum length
            // The maximum number of bytes in the text of an SQL statement is
            // limited to SQLITE_MAX_SQL_LENGTH which defaults to 1000000
            // (from http://www.sqlite.org/limits.html)
            removeTracksFromPlaylistsInner(trackIdList);
            trackIdList.clear();
        }
        trackIdList << QString::number(id);
    }
    removeTracksFromPlaylistsInner(trackIdList);
}

void PlaylistDAO::removeTracksFromPlaylistsInner(const QStringList& trackIdList) {
    QSqlQuery query(m_database);
    query.prepare(QString("SELECT DISTINCT playlist_id FROM PlaylistTracks WHERE track_id in (%1)")
                  .arg(trackIdList.join(",")));

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    // Collect all ids of the playlists that contains the tracks to remove
    QList<int> removedTracksPlaylistIds;
    const int playlistIDColoumn = query.record().indexOf("playlist_id");
    while (query.next()) {
        removedTracksPlaylistIds.append(query.value(playlistIDColoumn).toInt());
    }

    query.prepare(QString("DELETE FROM PlaylistTracks WHERE track_id in (%1)")
                  .arg(trackIdList.join(",")));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    foreach (int playlistId, removedTracksPlaylistIds) {
        emit(changed(playlistId));
    }
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
     //   3) Set postion=dest where pos=-1 -- Move that track from dummy pos to final destination

    QString queryString;

    // Move moved track to  dummy position -1
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

    emit(changed(playlistId));
}

void PlaylistDAO::shuffleTracks(const int playlistId, const QList<int>& positions) {
    ScopedTransaction transaction(m_database);
    QSqlQuery query(m_database);

    int seed = QDateTime::currentDateTime().toTime_t();
    qsrand(seed);

    // This is a modified Fisher-Yates shuffling algorithm.
    // Before swapping two tracks, it checks the IDs of the tracks before and
    // after each position to make sure we don't end up with doubled tracks.
    // If there's a duplicate, it simply tries again with a new position.
    // It shouldn't take more than 5 tries normally, but there's a limit of 10
    // to be safe.
    foreach (int oldPosition, positions) {
        int random;
        int newPosition;
        bool positionOk = false;
        // Check if either track will be next to a copy of itself
        for (int i=0; i<10 && !positionOk; i++) {
            random = (int)(qrand() / (RAND_MAX + 1.0) * (positions.count()));
            newPosition = positions.at(random);
            QString checkQuery = "SELECT track_id FROM PlaylistTracks "
                    "WHERE position=%1 AND playlist_id=%2";
            query.exec(checkQuery.arg(QString::number(oldPosition),
                                      QString::number(playlistId)));
            int oldTrackId = -1;
            if (query.first()) {
                oldTrackId = query.value(0).toInt();
            }
            query.clear();
            query.exec(checkQuery.arg(QString::number(newPosition),
                                      QString::number(playlistId)));
            int newTrackId = -1;
            if (query.first()) {
                newTrackId = query.value(0).toInt();
            }
            query.clear();
            checkQuery = "SELECT track_id FROM PlaylistTracks "
                    "WHERE (position=%1-1 OR position=%1+1) AND track_id=%2 "
                    "AND playlist_id=%3";
            query.exec(checkQuery.arg(QString::number(oldPosition),
                                      QString::number(newTrackId),
                                      QString::number(playlistId)));
            if (!query.first()) {
                query.exec(checkQuery.arg(QString::number(newPosition),
                                          QString::number(oldTrackId),
                                          QString::number(playlistId)));
                if (!query.first()) {
                    positionOk = true;
                }
            }

            query.clear();
        }
        if (!positionOk) {
            qDebug() << "Tried to re-sort too many times; had to double up songs.";
        }

        // Swap the tracks
        qDebug() << "Swapping tracks " << oldPosition << " and " << newPosition;
        QString swapQuery = "UPDATE PlaylistTracks SET position=%1 "
                "WHERE position=%2 AND playlist_id=%3";
        query.exec(swapQuery.arg(QString::number(-1),
                                 QString::number(oldPosition),
                                 QString::number(playlistId)));
        query.exec(swapQuery.arg(QString::number(oldPosition),
                                 QString::number(newPosition),
                                 QString::number(playlistId)));
        query.exec(swapQuery.arg(QString::number(newPosition),
                                 QString::number(-1),
                                 QString::number(playlistId)));

        if (query.lastError().isValid())
            qDebug() << query.lastError();
    }

    transaction.commit();
    emit(changed(playlistId));
}
