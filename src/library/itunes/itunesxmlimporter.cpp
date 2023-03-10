#include "library/itunes/itunesxmlimporter.h"

#include <QSqlQuery>
#include <QString>
#include <QUrl>
#include <memory>
#include <utility>

#include "library/itunes/itunesimporter.h"
#include "library/itunes/ituneslocalhosttoken.h"
#include "library/itunes/itunespathmapping.h"
#include "library/queryutil.h"
#include "library/treeitemmodel.h"
#include "util/lcs.h"

#ifdef __SQLITE3__
#include <sqlite3.h>
#else                        // __SQLITE3__
#define SQLITE_CONSTRAINT 19 // Abort due to constraint violation
#endif                       // __SQLITE3__

namespace {

const QString kDict = "dict";
const QString kKey = "key";
const QString kTrackId = "Track ID";
const QString kName = "Name";
const QString kArtist = "Artist";
const QString kAlbum = "Album";
const QString kAlbumArtist = "Album Artist";
const QString kGenre = "Genre";
const QString kGrouping = "Grouping";
const QString kBPM = "BPM";
const QString kBitRate = "Bit Rate";
const QString kComments = "Comments";
const QString kTotalTime = "Total Time";
const QString kYear = "Year";
const QString kLocation = "Location";
const QString kTrackNumber = "Track Number";
const QString kRating = "Rating";
const QString kTrackType = "Track Type";
const QString kRemote = "Remote";

} // anonymous namespace

ITunesXMLImporter::ITunesXMLImporter(LibraryFeature* parentFeature,
        QString filePath,
        QSqlDatabase& database,
        ITunesPathMapping& pathMapping,
        bool& cancelImport)
        : m_parentFeature(parentFeature),
          m_file(filePath),
          m_xml(&m_file),
          m_database(database),
          m_pathMapping(pathMapping),
          m_cancelImport(cancelImport) {
}

ITunesImport ITunesXMLImporter::importLibrary() {
    bool isTracksParsed = false;

    ITunesImport iTunesImport;
    iTunesImport.isMusicFolderLocatedAfterTracks = false;

    if (!m_file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open iTunes music collection";
        return iTunesImport;
    }

    while (!m_xml.atEnd() && !m_cancelImport) {
        m_xml.readNext();
        if (m_xml.isStartElement()) {
            if (m_xml.name() == QLatin1String("key")) {
                QString key = m_xml.readElementText();
                if (key == "Music Folder") {
                    if (isTracksParsed) {
                        iTunesImport.isMusicFolderLocatedAfterTracks = true;
                    }
                    if (readNextStartElement()) {
                        guessMusicLibraryMountpoint();
                    }
                } else if (key == "Tracks") {
                    parseTracks();
                    iTunesImport.playlistRoot = parsePlaylists();
                    isTracksParsed = true;
                }
            }
        }
    }

    if (m_xml.hasError()) {
        // do error handling
        qDebug() << "Abort processing iTunes music collection";
        qDebug() << "line:" << m_xml.lineNumber()
                 << "column:" << m_xml.columnNumber()
                 << "error:" << m_xml.errorString();
    }

    return iTunesImport;
}

void ITunesXMLImporter::guessMusicLibraryMountpoint() {
    // Normally the Folder Layout it some thing like that
    // iTunes/
    // iTunes/Album Artwork
    // iTunes/iTunes Media <- this is the "Music Folder"
    // iTunes/iTunes Music Library.xml <- this location we already knew
    QString music_folder = QUrl(m_xml.readElementText()).toLocalFile();

    QString music_folder_test = music_folder;
    music_folder_test.replace(iTunesLocalhostToken(), "");
    QDir music_folder_dir(music_folder_test);

    // The music folder exists, so a simple transformation
    // of replacing localhost token with nothing will work.
    if (music_folder_dir.exists()) {
        // Leave defaults intact.
        return;
    }

    // The iTunes Music Library doesn't exist! This means we are likely loading
    // the library from a system that is different from the one that wrote the
    // iTunes configuration. The configuration file path, m_dbfile is a readable
    // location that in most situation is "close" to the music library path so
    // since we can read that file we will try to infer the music library mount
    // point from it.

    // Examples:

    // Windows with non-itunes-managed music:
    // m_dbfile: c:/Users/LegacyII/Music/iTunes/iTunes Music Library.xml
    // Music Folder: file://localhost/C:/Users/LegacyII/Music/
    // Transformation:  "//localhost/" -> ""

    // Mac OS X with iTunes-managed music:
    // m_dbfile: /Users/rjryan/Music/iTunes/iTunes Music Library.xml
    // Music Folder: file://localhost/Users/rjryan/Music/iTunes/iTunes Media/
    // Transformation: "//localhost" -> ""

    // Linux reading an OS X partition mounted at /media/foo to an
    // iTunes-managed music folder:
    // m_dbfile: /media/foo/Users/rjryan/Music/iTunes/iTunes Music Library.xml
    // Music Folder: file://localhost/Users/rjryan/Music/iTunes/iTunes Media/
    // Transformation: "//localhost" -> "/media/foo"

    // Linux reading a Windows partition mounted at /media/foo to an
    // non-itunes-managed music folder:
    // m_dbfile: /media/foo/Users/LegacyII/Music/iTunes/iTunes Music Library.xml
    // Music Folder: file://localhost/C:/Users/LegacyII/Music/
    // Transformation:  "//localhost/C:" -> "/media/foo"

    // Algorithm:
    // 1. Find the largest common subsequence shared between m_dbfile and "Music
    //    Folder"
    // 2. For all tracks, replace the left-side of of the LCS in "Music Folder"
    //    with the left-side of the LCS in m_dbfile.

    QString lcs = LCS(m_dbfile, music_folder);

    if (lcs.size() <= 1) {
        qDebug() << "ERROR: Couldn't find a suitable transformation to load "
                    "iTunes data files. Leaving defaults intact.";
    }

    int musicFolderLcsIndex = music_folder.indexOf(lcs);
    if (musicFolderLcsIndex < 0) {
        qDebug() << "ERROR: Detected LCS" << lcs
                 << "is not present in music_folder:" << music_folder;
        return;
    }

    int dbfileLcsIndex = m_dbfile.indexOf(lcs);
    if (dbfileLcsIndex < 0) {
        qDebug() << "ERROR: Detected LCS" << lcs
                 << "is not present in m_dbfile" << m_dbfile;
        return;
    }

    m_pathMapping.dbITunesRoot = music_folder.left(musicFolderLcsIndex);
    m_pathMapping.mixxxITunesRoot = m_dbfile.left(dbfileLcsIndex);
    qDebug() << "Detected translation rule for iTunes files:"
             << m_pathMapping.dbITunesRoot << "->" << m_pathMapping.mixxxITunesRoot;
}

void ITunesXMLImporter::parseTracks() {
    bool in_container_dictionary = false;
    bool in_track_dictionary = false;
    QSqlQuery query(m_database);
    query.prepare(
            "INSERT INTO itunes_library (id, artist, title, album, "
            "album_artist, year, genre, grouping, comment, tracknumber,"
            "bpm, bitrate,"
            "duration, location,"
            "rating ) "
            "VALUES (:id, :artist, :title, :album, :album_artist, :year, "
            ":genre, :grouping, :comment, :tracknumber,"
            ":bpm, :bitrate,"
            ":duration, :location,"
            ":rating )");

    qDebug() << "Parse iTunes music collection";

    // read all sunsequent <dict> until we reach the closing ENTRY tag
    while (!m_xml.atEnd() && !m_cancelImport) {
        m_xml.readNext();

        if (m_xml.isStartElement()) {
            if (m_xml.name() == kDict) {
                if (!in_track_dictionary && !in_container_dictionary) {
                    in_container_dictionary = true;
                    continue;
                } else if (in_container_dictionary && !in_track_dictionary) {
                    // We are in a <dict> tag that holds track information
                    in_track_dictionary = true;
                    // Parse track here
                    parseTrack(query);
                }
            }
        }

        if (m_xml.isEndElement() && m_xml.name() == kDict) {
            if (in_track_dictionary && in_container_dictionary) {
                in_track_dictionary = false;
                continue;
            } else if (in_container_dictionary && !in_track_dictionary) {
                // Done parsing tracks.
                break;
            }
        }
    }
}

void ITunesXMLImporter::parseTrack(QSqlQuery& query) {
    // qDebug() << "----------------TRACK-----------------";
    int id = -1;
    QString title;
    QString artist;
    QString album;
    QString album_artist;
    QString year;
    QString genre;
    QString grouping;
    QString location;

    int bpm = 0;
    int bitrate = 0;

    // duration of a track
    int playtime = 0;
    int rating = 0;
    QString comment;
    QString tracknumber;
    QString tracktype;

    while (!m_xml.atEnd()) {
        m_xml.readNext();

        if (m_xml.isStartElement()) {
            if (m_xml.name() == kKey) {
                QString key = m_xml.readElementText();

                QString content;
                if (readNextStartElement()) {
                    content = m_xml.readElementText();
                }

                // qDebug() << "Key: " << key << " Content: " << content;

                if (key == kTrackId) {
                    id = content.toInt();
                    continue;
                }
                if (key == kName) {
                    title = content;
                    continue;
                }
                if (key == kArtist) {
                    artist = content;
                    continue;
                }
                if (key == kAlbum) {
                    album = content;
                    continue;
                }
                if (key == kAlbumArtist) {
                    album_artist = content;
                    continue;
                }
                if (key == kGenre) {
                    genre = content;
                    continue;
                }
                if (key == kGrouping) {
                    grouping = content;
                    continue;
                }
                if (key == kBPM) {
                    bpm = content.toInt();
                    continue;
                }
                if (key == kBitRate) {
                    bitrate = content.toInt();
                    continue;
                }
                if (key == kComments) {
                    comment = content;
                    continue;
                }
                if (key == kTotalTime) {
                    playtime = (content.toInt() / 1000);
                    continue;
                }
                if (key == kYear) {
                    year = content;
                    continue;
                }
                if (key == kLocation) {
                    location = mixxx::FileInfo::fromQUrl(QUrl(content)).location();
                    // Replace first part of location with the mixxx iTunes Root
                    // on systems where iTunes installed it only strips //localhost
                    // on iTunes from foreign systems the mount point is replaced
                    if (!m_pathMapping.dbITunesRoot.isEmpty()) {
                        location.replace(m_pathMapping.dbITunesRoot, m_pathMapping.mixxxITunesRoot);
                    }
                    continue;
                }
                if (key == kTrackNumber) {
                    tracknumber = content;
                    continue;
                }
                if (key == kRating) {
                    // value is an integer and ranges from 0 to 100
                    rating = (content.toInt() / 20);
                    continue;
                }
                if (key == kTrackType) {
                    tracktype = content;
                    continue;
                }
            }
        }
        // exit loop on closing </dict>
        if (m_xml.isEndElement() && m_xml.name() == kDict) {
            break;
        }
    }

    // If file is a remote file from iTunes Match, don't save it to the database.
    // There's no way that mixxx can access it.
    if (tracktype == kRemote) {
        return;
    }

    // If we reach the end of <dict>
    // Save parsed track to database
    query.bindValue(":id", id);
    query.bindValue(":artist", artist);
    query.bindValue(":title", title);
    query.bindValue(":album", album);
    query.bindValue(":album_artist", album_artist);
    query.bindValue(":genre", genre);
    query.bindValue(":grouping", grouping);
    query.bindValue(":year", year);
    query.bindValue(":duration", playtime);
    query.bindValue(":location", location);
    query.bindValue(":rating", rating);
    query.bindValue(":comment", comment);
    query.bindValue(":tracknumber", tracknumber);
    query.bindValue(":bpm", bpm);
    query.bindValue(":bitrate", bitrate);

    bool success = query.exec();

    if (!success) {
        LOG_FAILED_QUERY(query);
        return;
    }
}

std::unique_ptr<TreeItem> ITunesXMLImporter::parsePlaylists() {
    qDebug() << "Parse iTunes playlists";
    std::unique_ptr<TreeItem> pRootItem = TreeItem::newRoot(m_parentFeature);
    QSqlQuery query_insert_to_playlists(m_database);
    query_insert_to_playlists.prepare(
            "INSERT INTO itunes_playlists (id, name) "
            "VALUES (:id, :name)");

    QSqlQuery query_insert_to_playlist_tracks(m_database);
    query_insert_to_playlist_tracks.prepare(
            "INSERT INTO itunes_playlist_tracks (playlist_id, track_id, position) "
            "VALUES (:playlist_id, :track_id, :position)");

    while (!m_xml.atEnd() && !m_cancelImport) {
        m_xml.readNext();
        // We process and iterate the <dict> tags holding playlist summary information here
        if (m_xml.isStartElement() && m_xml.name() == kDict) {
            parsePlaylist(query_insert_to_playlists,
                    query_insert_to_playlist_tracks,
                    *pRootItem);
            continue;
        }
        if (m_xml.isEndElement()) {
            if (m_xml.name() == QLatin1String("array")) {
                break;
            }
        }
    }
    return pRootItem;
}

bool ITunesXMLImporter::readNextStartElement() {
    QXmlStreamReader::TokenType token = QXmlStreamReader::NoToken;
    while (token != QXmlStreamReader::EndDocument && token != QXmlStreamReader::Invalid) {
        token = m_xml.readNext();
        if (token == QXmlStreamReader::StartElement) {
            return true;
        }
    }
    return false;
}

void ITunesXMLImporter::parsePlaylist(QSqlQuery& query_insert_to_playlists,
        QSqlQuery& query_insert_to_playlist_tracks,
        TreeItem& root) {
    // qDebug() << "Parse Playlist";

    QString playlistname;
    int playlist_id = -1;
    int playlist_position = -1;
    int track_reference = -1;
    // indicates that we haven't found the <
    bool isSystemPlaylist = false;
    bool isPlaylistItemsStarted = false;

    // We process and iterate the <dict> tags holding playlist summary information here
    while (!m_xml.atEnd() && !m_cancelImport) {
        m_xml.readNext();

        if (m_xml.isStartElement()) {
            if (m_xml.name() == kKey) {
                QString key = m_xml.readElementText();
                // The rules are processed in sequence
                // That is, XML is ordered.
                // For iTunes Playlist names are always followed by the ID.
                // Afterwars the playlist entries occur
                if (key == "Name") {
                    readNextStartElement();
                    playlistname = m_xml.readElementText();
                    continue;
                }
                // When parsing the ID, the playlistname has already been found
                if (key == "Playlist ID") {
                    readNextStartElement();
                    playlist_id = m_xml.readElementText().toInt();
                    playlist_position = 1;
                    continue;
                }
                // Hide playlists that are system playlists
                if (key == "Master" || key == "Movies" || key == "TV Shows" ||
                        key == "Music" || key == "Books" || key == "Purchased") {
                    isSystemPlaylist = true;
                    continue;
                }

                if (key == "Playlist Items") {
                    isPlaylistItemsStarted = true;

                    // if the playlist is prebuild don't hit the database
                    if (isSystemPlaylist) {
                        continue;
                    }
                    query_insert_to_playlists.bindValue(":id", playlist_id);
                    query_insert_to_playlists.bindValue(":name", playlistname);

                    bool success = query_insert_to_playlists.exec();
                    if (!success) {
                        if (query_insert_to_playlists.lastError()
                                        .nativeErrorCode() ==
                                QString::number(SQLITE_CONSTRAINT)) {
                            // We assume a duplicate Playlist name
                            playlistname += QString(" #%1").arg(playlist_id);
                            query_insert_to_playlists.bindValue(":name", playlistname);

                            bool success = query_insert_to_playlists.exec();
                            if (!success) {
                                // unexpected error
                                LOG_FAILED_QUERY(query_insert_to_playlists);
                                break;
                            }
                        } else {
                            // unexpected error
                            LOG_FAILED_QUERY(query_insert_to_playlists);
                            return;
                        }
                    }
                    // append the playlist to the child model
                    root.appendChild(playlistname);
                }
                // When processing playlist entries, playlist name and id have
                // already been processed and persisted
                if (key == kTrackId) {
                    readNextStartElement();
                    track_reference = m_xml.readElementText().toInt();

                    query_insert_to_playlist_tracks.bindValue(":playlist_id", playlist_id);
                    query_insert_to_playlist_tracks.bindValue(":track_id", track_reference);
                    query_insert_to_playlist_tracks.bindValue(":position", playlist_position++);

                    // Insert tracks if we are not in a pre-build playlist
                    if (!isSystemPlaylist && !query_insert_to_playlist_tracks.exec()) {
                        qDebug() << "SQL Error in ITunesXMLImporter.cpp: line" << __LINE__ << " "
                                 << query_insert_to_playlist_tracks.lastError();
                        qDebug() << "trackid" << track_reference;
                        qDebug() << "playlistname; " << playlistname;
                        qDebug() << "-----------------";
                    }
                }
            }
        }
        if (m_xml.isEndElement()) {
            if (m_xml.name() == QLatin1String("array")) {
                // qDebug() << "exit playlist";
                break;
            }
            if (m_xml.name() == kDict && !isPlaylistItemsStarted) {
                // Some playlists can be empty, so we need to exit.
                break;
            }
        }
    }
}
