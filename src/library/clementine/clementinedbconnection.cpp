#include "library/clementine/clementinedbconnection.h"

#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QSqlError>

#include "library/queryutil.h"
#include "library/trackcollectionmanager.h"
#include "track/track.h"
#include "util/performancetimer.h"

namespace {
constexpr int kClementineUndefinedBpm = -1;      // clementine saves undefined bpm as -1
constexpr int kClementineUndefinedDuration = -1; // clementine saves undefined duration as -1
} // namespace

ClementineDbConnection::ClementineDbConnection(const TrackCollectionManager* pTrackCollection)
        : m_database(),
          m_pTrackCollectionManager(pTrackCollection) {
}

ClementineDbConnection::~ClementineDbConnection() {
    qDebug() << "Close Clementine database";
    m_database.close();
}

bool ClementineDbConnection::open(const QString& databaseFile) {
    m_database = QSqlDatabase::addDatabase("QSQLITE", "CLEMENTINE_DB_CONNECTION");
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

QList<ClementinePlaylist> ClementineDbConnection::getPlaylists() const {
    QList<ClementinePlaylist> list;
    ClementinePlaylist playlist;

    QSqlQuery query(m_database);
    query.prepare("SELECT ROWID, playlists.name FROM playlists ORDER BY playlists.name");

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return {};
    }
    while (query.next()) {
        playlist.playlistId = query.value(0).toString();
        playlist.name = query.value(1).toString();
        list.append(playlist);
    }
    return list;
}

QList<ClementinePlaylistEntry>
ClementineDbConnection::getPlaylistEntries(int playlistId) const {
    PerformanceTimer time;
    time.start();

    QList<ClementinePlaylistEntry> list;
    ClementinePlaylistEntry entry;

    //Query clementine playlist tracks
    QSqlQuery queryPlaylist(m_database);
    queryPlaylist.setForwardOnly(true); // Saves about 50% time
    queryPlaylist.prepare(
            "SELECT "
            "ROWID, "                      // 0
            "playlist_items.title, "       // 1
            "playlist_items.filename, "    // 2
            "playlist_items.length, "      // 3
            "playlist_items.artist, "      // 4
            "playlist_items.year, "        // 5
            "playlist_items.album, "       // 6
            "playlist_items.rating, "      // 7
            "playlist_items.genre, "       // 8
            "playlist_items.track, "       // 9
            "playlist_items.bpm, "         // 10
            "playlist_items.bitrate, "     // 11
            "playlist_items.comment, "     // 12
            "playlist_items.playcount, "   // 13
            "playlist_items.composer, "    // 14
            "playlist_items.grouping, "    // 15
            "playlist_items.albumartist, " // 16
            "playlist_items.type "         // 17
            "FROM playlist_items "
            "WHERE playlist_items.playlist = :playlistId");
    queryPlaylist.bindValue(":playlistId", playlistId);
    if (!queryPlaylist.exec()) {
        LOG_FAILED_QUERY(queryPlaylist);
        return {};
    }

    //Query clementine library tracks which are needed in playlist
    //Note that we use the same indices as in the playlist query!
    QSqlQuery queryLibrary(m_database);
    queryLibrary.setForwardOnly(true);
    queryLibrary.prepare(
            "SELECT "
            "songs.ROWID, "       // 0
            "songs.title, "       // 1
            "songs.filename, "    // 2
            "songs.length, "      // 3
            "songs.artist, "      // 4
            "songs.year, "        // 5
            "songs.album, "       // 6
            "songs.rating, "      // 7
            "songs.genre, "       // 8
            "songs.track, "       // 9
            "songs.bpm, "         // 10
            "songs.bitrate, "     // 11
            "songs.comment, "     // 12
            "songs.playcount, "   // 13
            "songs.composer, "    // 14
            "songs.grouping, "    // 15
            "songs.albumartist, " // 16
            "songs.albumartist "  // 17
            "FROM songs "
            "INNER JOIN playlist_items "
            "ON playlist_items.library_id = songs.ROWID "
            "WHERE playlist_items.playlist = :playlistId");
    queryLibrary.bindValue(":playlistId", playlistId);
    if (!queryLibrary.exec()) {
        LOG_FAILED_QUERY(queryLibrary);
        return {};
    }

    QSqlQuery* playlistTrackDataSourceQuery = &queryPlaylist;
    while (queryPlaylist.next()) {
        // Determine datasource/table for a the specific playlist track
        QString type = queryPlaylist.value(17).toString();
        qDebug() << "type " << type;
        if (type == QString("File")) {
            playlistTrackDataSourceQuery = &queryPlaylist;
        } else if (type == QString("Library")) {
            queryLibrary.next();
            playlistTrackDataSourceQuery = &queryLibrary;
        } else {
            qDebug() << "Unknown type " << type << " in playlist_item.type - skip!";
            continue;
        }

        //Search for track in mixxx lib to provide bpm information
        QString location = QUrl::fromEncoded(
                playlistTrackDataSourceQuery->value(2).toByteArray(), QUrl::StrictMode)
                                   .toLocalFile();
        qDebug() << "location " << location;

        bool trackAlreadyInLibrary = false;
        TrackPointer pTrack = m_pTrackCollectionManager->getOrAddTrack(
                TrackRef::fromFileInfo(location),
                &trackAlreadyInLibrary);

        entry.artist = playlistTrackDataSourceQuery->value(4).toString();
        entry.title = playlistTrackDataSourceQuery->value(1).toString();
        entry.trackId = playlistTrackDataSourceQuery->value(0).toInt();
        long long duration = playlistTrackDataSourceQuery->value(3).toLongLong();
        entry.year = playlistTrackDataSourceQuery->value(5).toInt();
        entry.album = playlistTrackDataSourceQuery->value(6).toString();
        entry.albumartist = playlistTrackDataSourceQuery->value(16).toString();
        entry.uri = QUrl::fromEncoded(
                playlistTrackDataSourceQuery->value(2).toByteArray(),
                QUrl::StrictMode);
        entry.rating = playlistTrackDataSourceQuery->value(7).toInt();
        entry.genre = playlistTrackDataSourceQuery->value(8).toString();
        entry.grouping = playlistTrackDataSourceQuery->value(15).toString();
        entry.tracknumber = playlistTrackDataSourceQuery->value(9).toInt();
        entry.bpm = playlistTrackDataSourceQuery->value(10).toDouble();
        entry.bitrate = playlistTrackDataSourceQuery->value(11).toInt();
        entry.comment = playlistTrackDataSourceQuery->value(12).toString();
        entry.playcount = playlistTrackDataSourceQuery->value(13).toInt();
        entry.composer = playlistTrackDataSourceQuery->value(14).toString();

        if (entry.artist.isEmpty() && entry.title.isEmpty()) {
            entry.artist =
                    QObject::tr("");
            entry.title = location.split(QDir::separator()).last();
        }
        if (entry.bpm == kClementineUndefinedBpm) {
            entry.bpm = 0;
        }
        if (duration == kClementineUndefinedDuration) {
            entry.duration = 0;
        } else {
            entry.duration = int(duration / 1000000000);
        }

        //If found in mixxx lib overwrite information
        if (trackAlreadyInLibrary) {
            entry.bpm = pTrack->getBpm();
            entry.duration = pTrack->getDurationInt();
        }

        list.append(entry);
    }

    qDebug() << "ClementineDbConnection::getPlaylistEntries(), took"
             << time.elapsed().debugMillisWithUnit();

    return list;
}

// static
QString ClementineDbConnection::getDatabaseFile() {
    QString dbfile;

    QSettings ini(QSettings::IniFormat, QSettings::UserScope, "Clementine", "Clementine");
    dbfile = QFileInfo(ini.fileName()).absolutePath();
    dbfile += "/clementine.db";
    if (QFile::exists(dbfile)) {
        return dbfile;
    }

    // Legacy clementine Application Data Path
    dbfile = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    dbfile += "/.gnome2/Clementine/clementine.db";
    if (QFile::exists(dbfile)) {
        return dbfile;
    }

    return QString();
}
