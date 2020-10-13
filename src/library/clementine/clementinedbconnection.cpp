#include <QSettings>
#include <QFile>
#include <QFileInfo>
#include <QSqlError>

#include "library/queryutil.h"
#include "library/clementine/clementinedbconnection.h"
#include "util/performancetimer.h"
#include "track/track.h"

ClementineDbConnection::ClementineDbConnection() {
}

ClementineDbConnection::~ClementineDbConnection() {
    qDebug() << "Close Clementine database";
    m_database.close();
}

void ClementineDbConnection::setTrackCollection(TrackCollectionManager* pTrackCollection) {
    m_pTrackCollectionManager = pTrackCollection;
}

bool ClementineDbConnection::open(const QString& databaseFile) {
    m_database = QSqlDatabase::addDatabase("QSQLITE","CLEMENTINE_DB_CONNECTION");
    m_database.setHostName("localhost");
    m_database.setDatabaseName(databaseFile);
    m_database.setConnectOptions("SQLITE_OPEN_READONLY");

    //Open the database connection in this thread.
    if (!m_database.open()) {
        m_database.setConnectOptions(); // clear options
        qWarning() << "Failed to open Clementine database." << m_database.lastError();
        return false;
    } else {
        qDebug() << "Successful opened Clementine database";
        return true;
    }
}

QList<struct ClementineDbConnection::Playlist> ClementineDbConnection::getPlaylists() {

    QList<struct ClementineDbConnection::Playlist> list;
    struct ClementineDbConnection::Playlist playlist;

    QSqlQuery query(m_database);
    query.prepare("SELECT ROWID, playlists.name FROM playlists ORDER By playlists.name");

    if (query.exec()) {
        while (query.next()) {
            playlist.playlistId = query.value(0).toString();
            playlist.name = query.value(1).toString();
            list.append(playlist);
        }
    } else {
        LOG_FAILED_QUERY(query);
    }
    return list;
}

QList<struct ClementineDbConnection::PlaylistEntry> ClementineDbConnection::getPlaylistEntries(int playlistId) {

    PerformanceTimer time;
    time.start();

    QList<struct ClementineDbConnection::PlaylistEntry> list;
    struct ClementineDbConnection::PlaylistEntry entry;

    QSqlQuery query(m_database);
    query.setForwardOnly(true); // Saves about 50% time

    QString queryString;

    queryString = QString(
        "SELECT "
        "ROWID, "
        "playlist_items.title, "              // 1
        "playlist_items.filename, "           // 2
        "playlist_items.length, "             // 3
        "playlist_items.artist, "             // 4
        "playlist_items.year, "               // 5
        "playlist_items.album, "              // 6
        "playlist_items.rating, "             // 7
        "playlist_items.genre, "              // 8
        "playlist_items.track, "              // 9
        //"playlist_items.DateAddedStamp, "   //
        "playlist_items.bpm, "                // 10
        "playlist_items.bitrate, "            // 11
        "playlist_items.comment, "            // 12
        "playlist_items.playcount, "          // 13
        "playlist_items.composer, "           // 14
        "playlist_items.grouping, "           // 15
        "playlist_items.type, "           // 16
        "playlist_items.albumartist "           // 17
        "FROM playlist_items "
        "WHERE playlist_items.playlist = %1")
            .arg(playlistId);


    query.prepare(queryString);
    if (query.exec()) {
        while (query.next()) {
            QString type =  query.value(16).toString();
            if(type != "File")
                continue;

            //Search for track in mixxx lib to provide bpm information
            QString location = QUrl::fromEncoded(query.value(2).toByteArray(), QUrl::StrictMode).toLocalFile();
            bool track_already_in_library = false;
            TrackPointer pTrack = m_pTrackCollectionManager->getOrAddTrack(
                TrackRef::fromFileInfo(location),
                &track_already_in_library);

            entry.artist = query.value(4).toString();
            entry.title = query.value(1).toString();
            entry.trackId = query.value(0).toInt();
            long long duration = query.value(3).toLongLong();
            entry.year = query.value(5).toInt();
            entry.album = query.value(6).toString();
            entry.albumartist = query.value(17).toString();
            entry.uri = QUrl::fromEncoded(query.value(2).toByteArray(), QUrl::StrictMode);
            entry.rating = query.value(7).toInt();
            entry.genre = query.value(8).toString();
            entry.grouping = query.value(15).toString();
            entry.tracknumber = query.value(9).toInt();
            entry.bpm = query.value(10).toInt();
            entry.bitrate = query.value(11).toInt();
            entry.comment = query.value(12).toString();
            entry.playcount = query.value(13).toInt();
            entry.composer = query.value(14).toString();

            if(entry.artist == "" && entry.title == ""){
                entry.artist = "Unknown"; // may confuse with real "Unknown" artists from whitelabels?
                entry.title = location.split(QDir::separator()).last();
            }
            if(entry.bpm == -1){
                entry.bpm=0;
            }
            if(duration == -1){
                entry.duration=0;
            }
            else{
                entry.duration=int(duration/1000000000);
            }

            //If found in mixxx lib overwrite information
            if(track_already_in_library){
                entry.bpm = pTrack->getBpm();
                entry.duration = pTrack->getDurationInt();
            }

            list.append(entry);
        }
    } else {
        LOG_FAILED_QUERY(query);
    }

    qDebug() << "ClementineDbConnection::getPlaylistEntries(), took"
             << time.elapsed().debugMillisWithUnit();

    return list;
}

// static
QString ClementineDbConnection::getDatabaseFile() {

    QString dbfile;

    QSettings ini(QSettings::IniFormat, QSettings::UserScope,
            "Clementine","Clementine");
    dbfile = QFileInfo(ini.fileName()).absolutePath();
    dbfile += "/clementine.db";
    if (QFile::exists(dbfile)) {
        return dbfile;
    }

    // Legacy Banshee Application Data Path
    dbfile = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    dbfile += "/.gnome2/Clementine/clementine.db";
    if (QFile::exists(dbfile)) {
        return dbfile;
    }


    return QString();
}
