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
        const QString& xmlFilePath,
        const QSqlDatabase& database,
        const std::atomic<bool>& cancelImport)
        : m_parentFeature(parentFeature),
          m_xmlFilePath(xmlFilePath),
          m_xmlFile(xmlFilePath),
          m_xml(&m_xmlFile),
          m_database(database),
          m_cancelImport(cancelImport) {
    // By default set m_mixxxItunesRoot and m_dbItunesRoot to strip out
    // file://localhost/ from the URL. When we load the user's iTunes XML
    // configuration we may replace this with something based on the detected
    // location of the user's iTunes path but the defaults are necessary in case
    // their iTunes XML does not include the "Music Folder" key.
    m_pathMapping.mixxxITunesRoot = "";
    m_pathMapping.dbITunesRoot = kiTunesLocalhostToken;
}

ITunesImport ITunesXMLImporter::importLibrary() {
    bool isTracksParsed = false;

    ITunesImport iTunesImport;
    bool isMusicFolderLocatedAfterTracks = false;

    if (!m_xmlFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open iTunes music collection";
        return iTunesImport;
    }

    while (!m_xml.atEnd() && !m_cancelImport.load()) {
        m_xml.readNext();
        if (m_xml.isStartElement()) {
            if (m_xml.name() == QLatin1String("key")) {
                QString key = m_xml.readElementText();
                if (key == "Music Folder") {
                    if (isTracksParsed) {
                        isMusicFolderLocatedAfterTracks = true;
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

    if (isMusicFolderLocatedAfterTracks) {
        qDebug() << "Updating iTunes real path from "
                 << m_pathMapping.dbITunesRoot << " to "
                 << m_pathMapping.mixxxITunesRoot;
        // In some iTunes files "Music Folder" XML node is located at the end of
        // file. So, we need to
        QSqlQuery query(m_database);
        query.prepare(
                "UPDATE itunes_library SET location = replace( location, "
                ":itunes_path, :mixxx_path )");
        query.bindValue(":itunes_path",
                m_pathMapping.dbITunesRoot.replace(kiTunesLocalhostToken, ""));
        query.bindValue(":mixxx_path", m_pathMapping.mixxxITunesRoot);
        bool success = query.exec();

        if (!success) {
            LOG_FAILED_QUERY(query);
        }
    }

    return iTunesImport;
}

void ITunesXMLImporter::guessMusicLibraryMountpoint() {
    // Normally the Folder Layout it some thing like that
    // iTunes/
    // iTunes/Album Artwork
    // iTunes/iTunes Media <- this is the "Music Folder"
    // iTunes/iTunes Music Library.xml <- this location we already knew
    QString musicFolder = QUrl(m_xml.readElementText()).toLocalFile();

    QString musicFolderTest = musicFolder;
    musicFolderTest.replace(kiTunesLocalhostToken, "");
    QDir musicFolderDir(musicFolderTest);

    // The music folder exists, so a simple transformation
    // of replacing localhost token with nothing will work.
    if (musicFolderDir.exists()) {
        // Leave defaults intact.
        return;
    }

    // The iTunes Music Library doesn't exist! This means we are likely loading
    // the library from a system that is different from the one that wrote the
    // iTunes configuration. The configuration file path, m_xmlFilePath is a readable
    // location that in most situation is "close" to the music library path so
    // since we can read that file we will try to infer the music library mount
    // point from it.

    // Examples:

    // Windows with non-itunes-managed music:
    // m_xmlFilePath: c:/Users/LegacyII/Music/iTunes/iTunes Music Library.xml
    // Music Folder: file://localhost/C:/Users/LegacyII/Music/
    // Transformation:  "//localhost/" -> ""

    // Mac OS X with iTunes-managed music:
    // m_xmlFilePath: /Users/rjryan/Music/iTunes/iTunes Music Library.xml
    // Music Folder: file://localhost/Users/rjryan/Music/iTunes/iTunes Media/
    // Transformation: "//localhost" -> ""

    // Linux reading an OS X partition mounted at /media/foo to an
    // iTunes-managed music folder:
    // m_xmlFilePath: /media/foo/Users/rjryan/Music/iTunes/iTunes Music Library.xml
    // Music Folder: file://localhost/Users/rjryan/Music/iTunes/iTunes Media/
    // Transformation: "//localhost" -> "/media/foo"

    // Linux reading a Windows partition mounted at /media/foo to an
    // non-itunes-managed music folder:
    // m_xmlFilePath: /media/foo/Users/LegacyII/Music/iTunes/iTunes Music Library.xml
    // Music Folder: file://localhost/C:/Users/LegacyII/Music/
    // Transformation:  "//localhost/C:" -> "/media/foo"

    // Algorithm:
    // 1. Find the largest common subsequence shared between m_xmlFilePath and
    //    "Music Folder"
    // 2. For all tracks, replace the left-side of of the LCS in "Music Folder"
    //    with the left-side of the LCS in m_xmlFilePath.

    QString lcs = LCS(m_xmlFilePath, musicFolder);

    if (lcs.size() <= 1) {
        qDebug() << "ERROR: Couldn't find a suitable transformation to load "
                    "iTunes data files. Leaving defaults intact.";
    }

    int musicFolderLcsIndex = musicFolder.indexOf(lcs);
    if (musicFolderLcsIndex < 0) {
        qDebug() << "ERROR: Detected LCS" << lcs
                 << "is not present in musicFolder:" << musicFolder;
        return;
    }

    int dbfileLcsIndex = m_xmlFilePath.indexOf(lcs);
    if (dbfileLcsIndex < 0) {
        qDebug() << "ERROR: Detected LCS" << lcs
                 << "is not present in m_xmlFilePath" << m_xmlFilePath;
        return;
    }

    m_pathMapping.dbITunesRoot = musicFolder.left(musicFolderLcsIndex);
    m_pathMapping.mixxxITunesRoot = m_xmlFilePath.left(dbfileLcsIndex);
    qDebug() << "Detected translation rule for iTunes files:"
             << m_pathMapping.dbITunesRoot << "->" << m_pathMapping.mixxxITunesRoot;
}

void ITunesXMLImporter::parseTracks() {
    bool inContainerDictionary = false;
    bool inTrackDictionary = false;
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
    while (!m_xml.atEnd() && !m_cancelImport.load()) {
        m_xml.readNext();

        if (m_xml.isStartElement()) {
            if (m_xml.name() == kDict) {
                if (!inTrackDictionary && !inContainerDictionary) {
                    inContainerDictionary = true;
                    continue;
                } else if (inContainerDictionary && !inTrackDictionary) {
                    // We are in a <dict> tag that holds track information
                    inTrackDictionary = true;
                    // Parse track here
                    parseTrack(query);
                }
            }
        }

        if (m_xml.isEndElement() && m_xml.name() == kDict) {
            if (inTrackDictionary && inContainerDictionary) {
                inTrackDictionary = false;
                continue;
            } else if (inContainerDictionary && !inTrackDictionary) {
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
    QString albumArtist;
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
                    albumArtist = content;
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
    query.bindValue(":album_artist", albumArtist);
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
    QSqlQuery queryInsertToPlaylists(m_database);
    queryInsertToPlaylists.prepare(
            "INSERT INTO itunes_playlists (id, name) "
            "VALUES (:id, :name)");

    QSqlQuery queryInsertToPlaylistTracks(m_database);
    queryInsertToPlaylistTracks.prepare(
            "INSERT INTO itunes_playlist_tracks (playlist_id, track_id, position) "
            "VALUES (:playlist_id, :track_id, :position)");

    while (!m_xml.atEnd() && !m_cancelImport.load()) {
        m_xml.readNext();
        // We process and iterate the <dict> tags holding playlist summary information here
        if (m_xml.isStartElement() && m_xml.name() == kDict) {
            parsePlaylist(queryInsertToPlaylists,
                    queryInsertToPlaylistTracks,
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

void ITunesXMLImporter::parsePlaylist(QSqlQuery& queryInsertToPlaylists,
        QSqlQuery& queryInsertToPlaylistTracks,
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
    while (!m_xml.atEnd() && !m_cancelImport.load()) {
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
                    queryInsertToPlaylists.bindValue(":id", playlist_id);
                    queryInsertToPlaylists.bindValue(":name", playlistname);

                    bool success = queryInsertToPlaylists.exec();
                    if (!success) {
                        if (queryInsertToPlaylists.lastError()
                                        .nativeErrorCode() ==
                                QString::number(SQLITE_CONSTRAINT)) {
                            // We assume a duplicate Playlist name
                            playlistname += QString(" #%1").arg(playlist_id);
                            queryInsertToPlaylists.bindValue(":name", playlistname);

                            bool success = queryInsertToPlaylists.exec();
                            if (!success) {
                                // unexpected error
                                LOG_FAILED_QUERY(queryInsertToPlaylists);
                                break;
                            }
                        } else {
                            // unexpected error
                            LOG_FAILED_QUERY(queryInsertToPlaylists);
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

                    queryInsertToPlaylistTracks.bindValue(":playlist_id", playlist_id);
                    queryInsertToPlaylistTracks.bindValue(":track_id", track_reference);
                    queryInsertToPlaylistTracks.bindValue(":position", playlist_position++);

                    // Insert tracks if we are not in a pre-build playlist
                    if (!isSystemPlaylist && !queryInsertToPlaylistTracks.exec()) {
                        qDebug() << "SQL Error in ITunesXMLImporter.cpp: line" << __LINE__ << " "
                                 << queryInsertToPlaylistTracks.lastError();
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
