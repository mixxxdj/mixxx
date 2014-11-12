#include <QtDebug>
#include <QDesktopServices>
#include <QSettings>
#include <QFile>
#include <QFileInfo>
#include <QSqlError>

#include "library/queryutil.h"

#include "bansheedbconnection.h"

BansheeDbConnection::BansheeDbConnection() {
}

BansheeDbConnection::~BansheeDbConnection() {
    qDebug() << "Close Banshee database";
    m_database.close();
}

bool BansheeDbConnection::open(const QString& databaseFile) {
    m_database = QSqlDatabase::addDatabase("QSQLITE", "BANSHE_DB_CONNECTION");
    m_database.setHostName("localhost");
    m_database.setDatabaseName(databaseFile);
    m_database.setConnectOptions("SQLITE_OPEN_READONLY");

    //Open the database connection in this thread.
    if (!m_database.open()) {
        m_database.setConnectOptions(); // clear options
        qDebug() << "Failed to open Banshee database." << m_database.lastError();
        return false;
    } else {
        // TODO(DSC): Verify schema
        // Banshee Schema file:
        // https://git.gnome.org/browse/banshee/tree/src/Core/Banshee.Services/Banshee.Database/BansheeDbFormatMigrator.cs
        // "Grouping" was introduced in schema 19 2008-08-19
        // Tested from 39 to 45
        qDebug() << "Successful opened Banshee database";
        return true;
    }
}

int BansheeDbConnection::getSchemaVersion() {
    QSqlQuery query(m_database);
    query.prepare("SELECT Value FROM CoreConfiguration WHERE Key = \"DatabaseVersion\"");

    if (query.exec()) {
        if (query.next()) {
            return query.value(0).toInt();
        }
    } else {
        LOG_FAILED_QUERY(query);
    }
    return -1;
}

QList<struct BansheeDbConnection::Playlist> BansheeDbConnection::getPlaylists() {

    QList<struct BansheeDbConnection::Playlist> list;
    struct BansheeDbConnection::Playlist playlist;

    QSqlQuery query(m_database);
    query.prepare("SELECT PlaylistID, Name FROM CorePlaylists ORDER By Name");

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

QList<struct BansheeDbConnection::PlaylistEntry> BansheeDbConnection::getPlaylistEntries(int playlistId) {

    QTime time;
    time.start();

    QList<struct BansheeDbConnection::PlaylistEntry> list;
    struct BansheeDbConnection::PlaylistEntry entry;

    QSqlQuery query(m_database);
    query.setForwardOnly(true); // Saves about 50% time

    QString queryString;

    if (playlistId == 0) {
        // Create Master Playlist
        queryString = QString(
            "SELECT "
            "CoreTracks.TrackID, "        // 0
            "CoreTracks.TrackID, "        // 1
            "CoreTracks.Title, "          // 2
            "CoreTracks.Uri, "            // 3
            "CoreTracks.Duration, "       // 4
            "CoreTracks.ArtistID, "       // 5
            "CoreArtists.Name, "          // 6
            "CoreTracks.Year, "           // 7
            "CoreTracks.AlbumID, "        // 8
            "CoreAlbums.Title, "          // 9
            "CoreTracks.Rating, "         // 10
            "CoreTracks.Genre, "          // 11
            "CoreTracks.TrackNumber, "    // 12
            "CoreTracks.DateAddedStamp, " // 13
            "CoreTracks.BPM, "            // 14
            "CoreTracks.BitRate, "        // 15
            "CoreTracks.Comment, "        // 16
            "CoreTracks.PlayCount, "      // 17
            "CoreTracks.Composer, "       // 18
            "CoreTracks.Grouping, "       // 19
            "CoreAlbums.ArtistID, "       // 20
            "AlbumArtists.Name "          // 21
            "FROM CoreTracks "
            "INNER JOIN CoreArtists ON CoreArtists.ArtistID = CoreTracks.ArtistID "
            "INNER JOIN CoreArtists AlbumArtists ON AlbumArtists.ArtistID = CoreAlbums.ArtistID "
            "INNER JOIN CoreAlbums ON CoreAlbums.AlbumID = CoreTracks.AlbumID ");
     } else {
        // SELECT playlist from CorePlaylistEntries
        queryString = QString(
            "SELECT "
            "CorePlaylistEntries.TrackID, "   // 0
            "CorePlaylistEntries.ViewOrder, " // 1
            "CoreTracks.Title, "              // 2
            "CoreTracks.Uri, "                // 3
            "CoreTracks.Duration, "           // 4
            "CoreTracks.ArtistID, "           // 5
            "CoreArtists.Name, "              // 6
            "CoreTracks.Year, "               // 7
            "CoreTracks.AlbumID, "            // 8
            "CoreAlbums.Title, "              // 9
            "CoreTracks.Rating, "             // 10
            "CoreTracks.Genre, "              // 11
            "CoreTracks.TrackNumber, "        // 12
            "CoreTracks.DateAddedStamp, "     // 13
            "CoreTracks.BPM, "                // 14
            "CoreTracks.BitRate, "            // 15
            "CoreTracks.Comment, "            // 16
            "CoreTracks.PlayCount, "          // 17
            "CoreTracks.Composer, "           // 18
            "CoreTracks.Grouping, "           // 19
            "CoreAlbums.ArtistID, "           // 20
            "AlbumArtists.Name "              // 21
            "FROM CorePlaylistEntries "
            "INNER JOIN CoreTracks ON CoreTracks.TrackID = CorePlaylistEntries.TrackID "
            "INNER JOIN CoreArtists ON CoreArtists.ArtistID = CoreTracks.ArtistID "
            "INNER JOIN CoreArtists AlbumArtists ON AlbumArtists.ArtistID = CoreAlbums.ArtistID "
            "INNER JOIN CoreAlbums ON CoreAlbums.AlbumID = CoreTracks.AlbumID "
            "WHERE CorePlaylistEntries.PlaylistID = %1")
                .arg(playlistId);
    }

    query.prepare(queryString);

    if (query.exec()) {
        while (query.next()) {
            entry.trackId = query.value(0).toInt();
            entry.viewOrder = query.value(1).toInt();
            m_trackMap[entry.trackId].title = query.value(2).toString();
            m_trackMap[entry.trackId].uri = QUrl::fromEncoded(query.value(3).toByteArray(), QUrl::StrictMode);
            m_trackMap[entry.trackId].duration = query.value(4).toInt();

            int artistId = query.value(5).toInt();
            m_artistMap[artistId].name = query.value(6).toString();
            m_trackMap[entry.trackId].year = query.value(7).toInt();
            int albumId = query.value(8).toInt();
            m_albumMap[albumId].title = query.value(9).toString();
            int albumArtistId = query.value(20).toInt();
            m_artistMap[albumArtistId].name = query.value(21).toString();
            m_trackMap[entry.trackId].rating = query.value(10).toInt();
            m_trackMap[entry.trackId].genre = query.value(11).toString();
            m_trackMap[entry.trackId].grouping = query.value(19).toString();
            m_trackMap[entry.trackId].tracknumber = query.value(12).toInt();
            m_trackMap[entry.trackId].dateadded = query.value(13).toInt();
            m_trackMap[entry.trackId].bpm = query.value(14).toInt();
            m_trackMap[entry.trackId].bitrate = query.value(15).toInt();
            m_trackMap[entry.trackId].comment = query.value(16).toString();
            m_trackMap[entry.trackId].playcount = query.value(17).toInt();
            m_trackMap[entry.trackId].composer = query.value(18).toString();

            entry.pTrack = &m_trackMap[entry.trackId];
            entry.pArtist = &m_artistMap[artistId];
            entry.pAlbum = &m_albumMap[albumId];
            entry.pAlbumArtist = &m_artistMap[albumArtistId];
            list.append(entry);
        }
    } else {
        LOG_FAILED_QUERY(query);
    }

    qDebug() << "BansheeDbConnection::getPlaylistEntries(), took " << time.elapsed() << "ms";

    return list;
}

// static
QString BansheeDbConnection::getDatabaseFile() {

    QString dbfile;

    // Banshee Application Data Path
    // on Windows - "%APPDATA%\banshee-1" ("<Drive>:\Documents and Settings\<login>\<Application Data>\banshee-1")
    // on Unix and Mac OS X - "$HOME/.config/banshee-1"

    QSettings ini(QSettings::IniFormat, QSettings::UserScope,
            "banshee-1","banshee");
    dbfile = QFileInfo(ini.fileName()).absolutePath();
    dbfile += "/banshee.db";
    if (QFile::exists(dbfile)) {
        return dbfile;
    }

    // Legacy Banshee Application Data Path
    QSettings ini2(QSettings::IniFormat, QSettings::UserScope,
            "banshee","banshee");
    dbfile = QFileInfo(ini2.fileName()).absolutePath();
    dbfile += "/banshee.db";
    if (QFile::exists(dbfile)) {
        return dbfile;
    }

    // Legacy Banshee Application Data Path
    dbfile = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
    dbfile += "/.gnome2/banshee/banshee.db";
    if (QFile::exists(dbfile)) {
        return dbfile;
    }

    return QString();
}
