#include "library/itunes/itunesxmlimporter.h"

#include <QDateTime>
#include <QString>
#include <QUrl>
#include <memory>
#include <utility>

#include "library/itunes/itunesdao.h"
#include "library/itunes/itunesfeature.h"
#include "library/itunes/itunesimporter.h"
#include "library/itunes/ituneslocalhosttoken.h"
#include "library/treeitem.h"
#include "util/fileaccess.h"
#include "util/fileinfo.h"
#include "util/lcs.h"

namespace {

const QString kDict = "dict";
const QString kKey = "key";
const QString kTrackId = "Track ID";
const QString kName = "Name";
const QString kArtist = "Artist";
const QString kAlbum = "Album";
const QString kAlbumArtist = "Album Artist";
const QString kComposer = "Composer";
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
const QString kPlayCount = "Play Count";
const QString kPlayDateUTC = "Play Date UTC";
const QString kDateAdded = "Date Added";

} // anonymous namespace

ITunesXMLImporter::ITunesXMLImporter(
        ITunesFeature* pParentFeature,
        const QString& xmlFilePath,
        std::unique_ptr<ITunesDAO> dao)
        : ITunesImporter(pParentFeature),
          m_xmlFilePath(xmlFilePath),
          m_xmlFile(xmlFilePath),
          m_xml(&m_xmlFile),
          m_dao(std::move(dao)) {
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

    // In sandboxed builds we have to obtain an access token
    auto access = mixxx::FileAccess(mixxx::FileInfo(m_xmlFilePath));

    if (!m_xmlFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open iTunes music collection XML at " << m_xmlFilePath
                   << ":" << m_xmlFile.errorString();
        return iTunesImport;
    }

    while (!m_xml.atEnd() && !canceled()) {
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
                } else if (key == "Playlists") {
                    parsePlaylists();

                    // The parent feature may be null during testing
                    std::unique_ptr<TreeItem> pRootItem = m_pParentFeature
                            ? TreeItem::newRoot(m_pParentFeature)
                            : std::make_unique<TreeItem>();
                    m_dao->appendPlaylistTree(pRootItem.get());

                    iTunesImport.playlistRoot = std::move(pRootItem);
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
        m_dao->applyPathMapping(m_pathMapping);
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

    qDebug() << "Parse iTunes music collection";

    // read all sunsequent <dict> until we reach the closing ENTRY tag
    while (!m_xml.atEnd() && !canceled()) {
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
                    parseTrack();
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

void ITunesXMLImporter::parseTrack() {
    // qDebug() << "----------------TRACK-----------------";
    int id = -1;
    QString title;
    QString artist;
    QString album;
    QString albumArtist;
    QString composer;
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

    int playCount = 0;
    QDateTime lastPlayedAt;
    QDateTime dateAdded;

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
                if (key == kComposer) {
                    composer = content;
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
                    // Convert the location URL to a file path. Note that we intentionally
                    // do not use FileInfo::location here since it would prepend a drive letter
                    // prefix to Unix paths, something we already take care of through the
                    // path mapping below (otherwise we'd end up with duplicate drive letter
                    // prefixes, e.g. C:C:, if the translation rule already substitutes a
                    // path with a drive letter).
                    location = mixxx::FileInfo::fromQUrl(QUrl(content)).asQFileInfo().filePath();
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
                if (key == kPlayCount) {
                    playCount = content.toInt();
                    continue;
                }
                if (key == kPlayDateUTC) {
                    lastPlayedAt = QDateTime::fromString(content, Qt::ISODate);
                    continue;
                }
                if (key == kDateAdded) {
                    dateAdded = QDateTime::fromString(content, Qt::ISODate);
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
    ITunesTrack track = {
            .id = id,
            .artist = artist,
            .title = title,
            .album = album,
            .albumArtist = albumArtist,
            .composer = composer,
            .genre = genre,
            .grouping = grouping,
            .year = year.toInt(),
            .duration = playtime,
            .location = location,
            .rating = rating,
            .comment = comment,
            .trackNumber = tracknumber.toInt(),
            .bpm = bpm,
            .bitrate = bitrate,
            .playCount = playCount,
            .lastPlayedAt = lastPlayedAt,
            .dateAdded = dateAdded,
    };

    if (!m_dao->importTrack(track)) {
        return;
    }
}

void ITunesXMLImporter::parsePlaylists() {
    qDebug() << "Parse iTunes playlists";

    while (!m_xml.atEnd() && !canceled()) {
        m_xml.readNext();
        // We process and iterate the <dict> tags holding playlist summary information here
        if (m_xml.isStartElement() && m_xml.name() == kDict) {
            parsePlaylist();
            continue;
        }
        if (m_xml.isEndElement()) {
            if (m_xml.name() == QLatin1String("array")) {
                break;
            }
        }
    }
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

void ITunesXMLImporter::parsePlaylist() {
    // qDebug() << "Parse Playlist";

    ITunesPlaylist playlist = {.id = -1, .name{}};
    QString persistentId;
    QString parentPersistentId;
    int trackPosition = -1;
    int trackReference = -1;
    // indicates that we haven't found the <
    bool isSystemPlaylist = false;
    bool isPlaylistItemsStarted = false;

    // We process and iterate the <dict> tags holding playlist summary information here
    while (!m_xml.atEnd() && !canceled()) {
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
                    playlist.name = m_xml.readElementText();
                    continue;
                }
                // When parsing the ID, the playlistname has already been found
                if (key == "Playlist ID") {
                    readNextStartElement();
                    playlist.id = m_xml.readElementText().toInt();
                    trackPosition = 1;
                    continue;
                }
                if (key == "Playlist Persistent ID") {
                    readNextStartElement();
                    persistentId = m_xml.readElementText();
                    continue;
                }
                if (key == "Parent Persistent ID") {
                    readNextStartElement();
                    parentPersistentId = m_xml.readElementText();
                    continue;
                }
                // Hide playlists that are system playlists
                if (key == "Distinguished Kind") {
                    readNextStartElement();
                    isSystemPlaylist = true;
                    continue;
                }
                if (key == "Visible") {
                    readNextStartElement();
                    if (m_xml.name() == QString("false")) {
                        isSystemPlaylist = true;
                    }
                    continue;
                }

                if (key == "Playlist Items") {
                    isPlaylistItemsStarted = true;

                    // if the playlist is prebuilt don't hit the database
                    if (!isSystemPlaylist) {
                        if (!m_dao->importPlaylist(playlist)) {
                            // unexpected error
                            break;
                        }
                    }

                    continue;
                }
                // When processing playlist entries, playlist name and id have
                // already been processed and persisted
                if (key == kTrackId) {
                    readNextStartElement();
                    trackReference = m_xml.readElementText().toInt();

                    // Insert tracks if we are not in a pre-built playlist
                    if (!isSystemPlaylist) {
                        m_dao->importPlaylistTrack(playlist.id,
                                trackReference,
                                trackPosition++);
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

    if (!isSystemPlaylist) {
        // Make sure empty playlists are imported too
        if (!isPlaylistItemsStarted) {
            m_dao->importPlaylist(playlist);
        }

        m_playlistIdByPersistentId[persistentId] = playlist.id;

        int parentId = kRootITunesPlaylistId;
        if (!parentPersistentId.isNull()) {
            auto found = m_playlistIdByPersistentId.find(parentPersistentId);
            if (found != m_playlistIdByPersistentId.end()) {
                parentId = found.value();
            }
        }

        m_dao->importPlaylistRelation(parentId, playlist.id);
    }
}
