
#include <QtDebug>
#include <QtCore>
#include <QSqlQuery>
#include <QSqlError>
#include "trackinfoobject.h"
#include "library/dao/playlistdao.h"

PlaylistDAO::PlaylistDAO(QSqlDatabase& database)
        : m_database(database) {

}

PlaylistDAO::~PlaylistDAO()
{
}

void PlaylistDAO::initialize()
{
    qDebug() << "PlaylistDAO::initialize" << QThread::currentThread() << m_database.connectionName();
}

/** Create a playlist with the given name.
    @param name The name of the playlist to be created.
*/
void PlaylistDAO::createPlaylist(QString name, bool hidden)
{
    qDebug() << "PlaylistDAO::createPlaylist"
             << QThread::currentThread()
             << m_database.connectionName();
    //Start the transaction
    m_database.transaction();

    //Find out the highest position for the existing playlists so we know what
    //position this playlist should have.
    QSqlQuery query(m_database);
    query.prepare("SELECT max(position) as posmax FROM Playlists");

    if (!query.exec()) {
        qDebug() << query.lastError();
        m_database.rollback();
        return;
    }
    //query.finish();

    //Get the id of the last playlist.
    int position = 0;
    if (query.next()) {
        position = query.value(query.record().indexOf("posmax")).toInt();
        position++; //Append after the last playlist.
    }

    qDebug() << "Inserting playlist" << name << "at position" << position;

    query.prepare("INSERT INTO Playlists (name, position, hidden) "
                  "VALUES (:name, :position, :hidden)");
 				 // ":date_created, :date_modified)");
    query.bindValue(":name", name);
    query.bindValue(":position", position);
    query.bindValue(":hidden", hidden ? 1 : 0);

    if (!query.exec()) {
        qDebug() << query.lastError();
        m_database.rollback();
    }
    //query.finish();

    //Commit the transaction
    m_database.commit();
}

/** Find out the name of the playlist at the given position */
QString PlaylistDAO::getPlaylistName(unsigned int position)
{
    qDebug() << "PlaylistDAO::getPlaylistName" << QThread::currentThread() << m_database.connectionName();

    QSqlQuery query(m_database);
    query.prepare("SELECT name FROM Playlists "
                  "WHERE position = :position");
    query.bindValue(":position", position);

    if (!query.exec()) {
        qDebug() << "getPlaylistName" << query.lastError();
        return "";
    }

    // Get the name field
    QString name = "";
    while (query.next()) {
        name = query.value(query.record().indexOf("name")).toString();
    }
    //query.finish();
    return name;
}

int PlaylistDAO::getPlaylistIdFromName(QString name) {
    qDebug() << "PlaylistDAO::getPlaylistIdFromName" << QThread::currentThread() << m_database.connectionName();

    QSqlQuery query(m_database);
    query.prepare("SELECT id FROM Playlists WHERE name = :name");
    query.bindValue(":name", name);
    if (query.exec()) {
        if (query.next()) {
            return query.value(query.record().indexOf("id")).toInt();
        }
    } else {
        qDebug() << "getPlaylistIdFromName:" << query.lastError();
    }
    return -1;
}


/** Delete a playlist */
void PlaylistDAO::deletePlaylist(int playlistId)
{
    qDebug() << "PlaylistDAO::deletePlaylist" << QThread::currentThread() << m_database.connectionName();
    m_database.transaction();

    //Get the playlist id for this
    QSqlQuery query(m_database);

    //Delete the row in the Playlists table.
    query.prepare("DELETE FROM Playlists "
                  "WHERE id= :id");
    query.bindValue(":id", playlistId);
    if (!query.exec()) {
     	qDebug() << "deletePlaylist" << query.lastError();
      m_database.rollback();
     	return;
    }
    //query.finish();

    //Delete the tracks in this playlist from the PlaylistTracks table.
    query.prepare("DELETE FROM PlaylistTracks "
                  "WHERE playlist_id = :id");
    query.bindValue(":id", playlistId);
    if (!query.exec()) {
     	qDebug() << "deletePlaylist" << query.lastError();
      m_database.rollback();
     	return;
    }
    //query.finish();

    m_database.commit();
    //TODO: Crap, we need to shuffle the positions of all the playlists?
}

/** Append a track to a playlist */
void PlaylistDAO::appendTrackToPlaylist(int trackId, int playlistId)
{
    qDebug() << "PlaylistDAO::appendTrackToPlaylist"
             << QThread::currentThread() << m_database.connectionName();
    qDebug() << "appendTrackToPlaylist, track:" << trackId
             << "playlist:" << playlistId;

    //Start the transaction
    m_database.transaction();

    //Find out the highest position existing in the playlist so we know what
    //position this track should have.
    QSqlQuery query(m_database);
    query.prepare("SELECT max(position) as position FROM PlaylistTracks "
                  "WHERE playlist_id = :id");
    query.bindValue(":id", playlistId);
    query.exec();

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << "appendTrackToPlaylist" << query.lastError();
      // m_database.rollback();
      // return;
    }

    // Get the position of the highest playlist...
    int position = 0;
    if (query.next()) {
        position = query.value(query.record().indexOf("position")).toInt();
    }
    position++; //Append after the last song.


    //Insert the song into the PlaylistTracks table
    query.prepare("INSERT INTO PlaylistTracks (playlist_id, track_id, position)"
                  "VALUES (:playlist_id, :track_id, :position)");
    query.bindValue(":playlist_id", playlistId);
    query.bindValue(":track_id", trackId);
    query.bindValue(":position", position);
    query.exec();

    //Start the transaction
    m_database.commit();
}

/** Find out how many playlists exist. */
unsigned int PlaylistDAO::playlistCount()
{
    qDebug() << "PlaylistDAO::playlistCount" << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("SELECT count(*) as count FROM Playlists");

    if (!query.exec()) {
        qDebug() << query.lastError();
    }

    int numRecords = 0;
    if (query.next()) {
        numRecords = query.value(query.record().indexOf("count")).toInt();
    }
    return numRecords;
}

int PlaylistDAO::getPlaylistId(int position)
{
    qDebug() << "PlaylistDAO::getPlaylistId"
             << QThread::currentThread() << m_database.connectionName();

    //Find out the highest position existing in the playlist so we know what
    //position this track should have.
    QSqlQuery query(m_database);
    query.prepare("SELECT id FROM Playlists "
                  "WHERE position=:position");
    query.bindValue(":position", position);

    if (!query.exec()) {
        qDebug() << query.lastError();
        return -1;
    }

    //Get the id field
    int playlistId = -1;
    if (query.next()) {
        playlistId = query.value(query.record().indexOf("id")).toInt();
    }
    //query.finish();

    return playlistId;
}

void PlaylistDAO::removeTrackFromPlaylist(int playlistId, int position)
{
    qDebug() << "PlaylistDAO::removeTrackFromPlaylist"
             << QThread::currentThread() << m_database.connectionName();
    m_database.transaction();
    QSqlQuery query(m_database);
    //Delete the track from the playlist.
    query.prepare("DELETE FROM PlaylistTracks "
                  "WHERE playlist_id=:id AND position= :position");
    query.bindValue(":id", playlistId);
    query.bindValue(":position", position);

    if (!query.exec()) {
        qDebug() << query.lastError();
        m_database.rollback();
        return;
    }
    //query.finish();

    QString queryString;
    queryString = QString("UPDATE PlaylistTracks SET position=position-1 "
                          "WHERE position>=%1 AND "
                          "playlist_id=%2").arg(position).arg(playlistId);
    query.exec(queryString);
    //query.finish();
    m_database.commit();
}

void PlaylistDAO::insertTrackIntoPlaylist(int trackId, int playlistId, int position)
{
    qDebug() << "insertTrackIntoPlaylist, track:" << trackId
             << "playlist:" << playlistId
             << "position:" << position;

    if (playlistId < 0 || trackId < 0 || position < 0)
        return;

    m_database.transaction();

    // Move all the tracks in the playlist up by one
    QString queryString =
            QString("UPDATE PlaylistTracks SET position=position+1 "
                    "WHERE position>=%1 AND "
                    "playlist_id=%2").arg(position).arg(playlistId);

    QSqlQuery query(m_database);
    if (!query.exec(queryString)) {
        qDebug() << query.lastError();
    }

    //Insert the song into the PlaylistTracks table
    query.prepare("INSERT INTO PlaylistTracks (playlist_id, track_id, position)"
                  "VALUES (:playlist_id, :track_id, :position)");
    query.bindValue(":playlist_id", playlistId);
    query.bindValue(":track_id", trackId);
    query.bindValue(":position", position);

    if (!query.exec()) {
        qDebug() << query.lastError();
    }

    //query.finish();

    m_database.commit();
}
