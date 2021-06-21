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

ClementineDbConnection::ClementineDbConnection()
        : m_database() {
}

ClementineDbConnection::~ClementineDbConnection() {
    qDebug() << "Close Clementine database";
    m_database.close();
}

bool ClementineDbConnection::open(const QFileInfo& databaseFile) {
    m_database = QSqlDatabase::addDatabase("QSQLITE", "CLEMENTINE_DB_CONNECTION");
    m_database.setHostName("localhost");
    m_database.setDatabaseName(databaseFile.filePath());
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
    QSqlQuery* trackDataSource;
    QSqlQuery queryPlaylist(m_database);
    queryPlaylist.setForwardOnly(true);
    QSqlQuery queryLibrary(m_database);
    queryLibrary.setForwardOnly(true);

    if (playlistId == 0) {
        //Query clementine library tracks
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
                "FROM songs ");
        if (!queryLibrary.exec()) {
            LOG_FAILED_QUERY(queryLibrary);
            return {};
        }
        trackDataSource = &queryLibrary;
    } else {
        //Query clementine playlist tracks
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
        trackDataSource = &queryPlaylist;
    }

    while (trackDataSource->next()) {
        if (playlistId > 0) {
            // Determine datasource/table for a the specific playlist track
            QString type = queryPlaylist.value(17).toString();
            qDebug() << "type " << type;
            if (type == QString("File")) {
                trackDataSource = &queryPlaylist;
            } else if (type == QString("Library")) {
                queryLibrary.next();
                trackDataSource = &queryLibrary;
            } else {
                qDebug() << "Unknown type " << type << " in playlist_item.type - skip!";
                continue;
            }
        }

        QString location = QUrl::fromEncoded(
                trackDataSource->value(2).toByteArray(), QUrl::StrictMode)
                                   .toLocalFile();
        qDebug() << "location " << location;

        entry.artist = trackDataSource->value(4).toString();
        entry.title = trackDataSource->value(1).toString();
        entry.trackId = trackDataSource->value(0).toInt();
        long long duration = trackDataSource->value(3).toLongLong();
        entry.year = trackDataSource->value(5).toInt();
        entry.album = trackDataSource->value(6).toString();
        entry.albumartist = trackDataSource->value(16).toString();
        entry.uri = QUrl::fromEncoded(
                trackDataSource->value(2).toByteArray(),
                QUrl::StrictMode);
        float clementineRating = trackDataSource->value(7).toFloat();
        if (clementineRating < 0) {
            entry.rating = 0;
        } else {
            entry.rating = int(clementineRating * 5);
        }
        entry.genre = trackDataSource->value(8).toString();
        entry.grouping = trackDataSource->value(15).toString();
        entry.tracknumber = trackDataSource->value(9).toInt();
        entry.bpm = trackDataSource->value(10).toDouble();
        entry.bitrate = trackDataSource->value(11).toInt();
        entry.comment = trackDataSource->value(12).toString();
        entry.playcount = trackDataSource->value(13).toInt();
        entry.composer = trackDataSource->value(14).toString();

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

        list.append(entry);

        if (playlistId > 0) {
            trackDataSource = &queryPlaylist;
        }
    }

    qDebug() << "ClementineDbConnection::getPlaylistEntries(), took"
             << time.elapsed().debugMillisWithUnit();

    return list;
}

// static
QFileInfo ClementineDbConnection::getDatabaseFile() {
    QFileInfo dbfile;

    QSettings ini(QSettings::IniFormat, QSettings::UserScope, "Clementine", "Clementine");
    dbfile = QFileInfo(QFileInfo(ini.fileName()).absoluteDir(), "clementine.db");
    qDebug() << "dbfile: " << dbfile;
    if (dbfile.exists()) {
        return dbfile;
    }

    // Legacy clementine Application Data Path
    dbfile = QFileInfo(
            QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
            "/.gnome2/Clementine/clementine.db");
    return dbfile;
}
