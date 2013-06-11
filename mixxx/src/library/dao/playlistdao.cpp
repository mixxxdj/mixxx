#include <QtDebug>
#include <QtCore>
#include <QtSql>

#include "trackinfoobject.h"
#include "library/dao/playlistdao.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"

PlaylistDAO::PlaylistDAO(QSqlDatabase& database)
        : m_database(database) {
}

PlaylistDAO::~PlaylistDAO()
{
}

void PlaylistDAO::initialize()
{
}

int PlaylistDAO::createPlaylist(QString name, HiddenType hidden)
{
    // qDebug() << "PlaylistDAO::createPlaylist"
    //          << QThread::currentThread()
    //          << m_database.connectionName();
    //Start the transaction
    ScopedTransaction transaction(m_database);

    //Find out the highest position for the existing playlists so we know what
    //position this playlist should have.
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
        position++; //Append after the last playlist.
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
    //Commit the transaction
    transaction.commit();
    emit(added(playlistId));
    return playlistId;
}

QString PlaylistDAO::getPlaylistName(int playlistId)
{
    // qDebug() << "PlaylistDAO::getPlaylistName" << QThread::currentThread() << m_database.connectionName();

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
    while (query.next()) {
        name = query.value(query.record().indexOf("name")).toString();
    }
    return name;
}

int PlaylistDAO::getPlaylistIdFromName(QString name) {
    // qDebug() << "PlaylistDAO::getPlaylistIdFromName" << QThread::currentThread() << m_database.connectionName();

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

void PlaylistDAO::deletePlaylist(int playlistId)
{
    // qDebug() << "PlaylistDAO::deletePlaylist" << QThread::currentThread() << m_database.connectionName();
    ScopedTransaction transaction(m_database);

    //Get the playlist id for this
    QSqlQuery query(m_database);

    //Delete the row in the Playlists table.
    query.prepare("DELETE FROM Playlists "
                  "WHERE id= :id");
    query.bindValue(":id", playlistId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    //Delete the tracks in this playlist from the PlaylistTracks table.
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

void PlaylistDAO::renamePlaylist(int playlistId, const QString& newName) {
    QSqlQuery query(m_database);
    query.prepare("UPDATE Playlists SET name = :name WHERE id = :id");
    query.bindValue(":name", newName);
    query.bindValue(":id", playlistId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }
    emit(renamed(playlistId));
}

bool PlaylistDAO::setPlaylistLocked(int playlistId, bool locked) {
    QSqlQuery query(m_database);
    query.prepare("UPDATE Playlists SET locked = :lock WHERE id = :id");
    // SQLite3 doesn't support boolean value. Using integer instead.
    query.bindValue(":lock", static_cast<int>(locked));
    query.bindValue(":id", playlistId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    emit(lockChanged(playlistId));
    return true;
}

bool PlaylistDAO::isPlaylistLocked(int playlistId) {
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

bool PlaylistDAO::appendTracksToPlaylist(QList<int> trackIds, int playlistId) {
    // qDebug() << "PlaylistDAO::appendTracksToPlaylist"
    //          << QThread::currentThread() << m_database.connectionName();

    // Start the transaction
    ScopedTransaction transaction(m_database);

    int position = getMaxPosition(playlistId);

    // Append after the last song. If no songs or a failed query then 0 becomes
    // 1.
    position++;

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

bool PlaylistDAO::appendTrackToPlaylist(int trackId, int playlistId) {
    QList<int> tracks;
    tracks.append(trackId);
    return appendTracksToPlaylist(tracks, playlistId);
}

/** Find out how many playlists exist. */
unsigned int PlaylistDAO::playlistCount()
{
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

int PlaylistDAO::getPlaylistId(int index)
{
    // qDebug() << "PlaylistDAO::getPlaylistId"
    //          << QThread::currentThread() << m_database.connectionName();

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

PlaylistDAO::HiddenType PlaylistDAO::getHiddenType(int playlistId) {
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

bool PlaylistDAO::isHidden(int playlistId) {
    // qDebug() << "PlaylistDAO::isHidden"
    //          << QThread::currentThread() << m_database.connectionName();

    HiddenType ht = getHiddenType(playlistId);
    if (ht == PLHT_NOT_HIDDEN) {
        return false;
    }
    return true;
}

void PlaylistDAO::removeTrackFromPlaylist(int playlistId, int position)
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

void PlaylistDAO::removeTracksFromPlaylist(int playlistId, QList<int> positions) {
    // get positions in reversed order
    qSort(positions.begin(), positions.end(), qGreater<int>());

    // qDebug() << "PlaylistDAO::removeTrackFromPlaylist"
    //          << QThread::currentThread() << m_database.connectionName();
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

        emit(trackRemoved(playlistId, trackId, position));
    }
    transaction.commit();
    emit(changed(playlistId));
}

bool PlaylistDAO::insertTrackIntoPlaylist(int trackId, int playlistId, int position) {
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

int PlaylistDAO::insertTracksIntoPlaylist(QList<int> trackIds, int playlistId, int position) {
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
        insertPositon++;
        tracksAdded++;
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

void PlaylistDAO::addToAutoDJQueue(int playlistId, bool bTop) {
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

    // Get the ID of the Auto-DJ playlist
    int autoDJId = getPlaylistIdFromName(AUTODJ_TABLE);

    // Loop through the tracks, adding them to the Auto-DJ Queue. Start at
    // position 2 because position 1 was already loaded to the deck.
    QList<int> ids;
    while (query.next()) {
        ids.append(query.value(0).toInt());
    }
    if (bTop) {
        insertTracksIntoPlaylist(ids, autoDJId, 2);
    } else {
        // TODO(XXX): Care whether the append succeeded.
        appendTracksToPlaylist(ids, autoDJId);
    }
}

int PlaylistDAO::getPreviousPlaylist(int currentPlaylistId, HiddenType hidden) {
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

bool PlaylistDAO::copyPlaylistTracks(int sourcePlaylistID, int targetPlaylistID) {
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

int PlaylistDAO::getMaxPosition(int playlistId) {
    //Find out the highest position existing in the playlist so we know what
    //position this track should have.
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

void PlaylistDAO::removeTrackFromPlaylists(int trackId) {
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM PlaylistTracks WHERE "
                  "track_id=:=id");
    query.bindValue(":id", trackId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
}

void PlaylistDAO::removeTracksFromPlaylists(QList<int> ids) {
    QStringList idList;
    foreach (int id, ids) {
        idList << QString::number(id);
    }
    QSqlQuery query(m_database);
    query.prepare(QString("DELETE FROM PlaylistTracks WHERE track_id in (%1)")
                  .arg(idList.join(",")));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
}

int PlaylistDAO::tracksInPlaylist(int playlistId) {
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
    while (query.next()) {
        count = query.value(query.record().indexOf("count")).toInt();
    }
    return count;
}
