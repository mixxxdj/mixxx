#include "library/itunes/itunesfeature.h"

#include <QAction>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QStandardPaths>
#include <QUrl>
#include <QXmlStreamReader>
#include <QtDebug>

#include "library/baseexternalplaylistmodel.h"
#include "library/baseexternaltrackmodel.h"
#include "library/basetrackcache.h"
#include "library/dao/settingsdao.h"
#include "library/library.h"
#include "library/queryutil.h"
#include "library/trackcollectionmanager.h"
#include "moc_itunesfeature.cpp"
#include "util/lcs.h"
#include "util/sandbox.h"
#include "widget/wlibrarysidebar.h"

#ifdef __SQLITE3__
#include <sqlite3.h>
#else // __SQLITE3__
#define SQLITE_CONSTRAINT  19 // Abort due to constraint violation
#endif // __SQLITE3__

namespace {

const QString ITDB_PATH_KEY = "mixxx.itunesfeature.itdbpath";

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

QString localhost_token() {
#if defined(__WINDOWS__)
    return "//localhost/";
#else
    return "//localhost";
#endif
}

} // anonymous namespace

ITunesFeature::ITunesFeature(Library* pLibrary, UserSettingsPointer pConfig)
        : BaseExternalLibraryFeature(pLibrary, pConfig),
          m_cancelImport(false),
          m_icon(":/images/library/ic_library_itunes.svg") {
    QString tableName = "itunes_library";
    QString idColumn = "id";
    QStringList columns;
    columns << "id"
            << "artist"
            << "title"
            << "album"
            << "album_artist"
            << "year"
            << "genre"
            << "grouping"
            << "tracknumber"
            << "location"
            << "comment"
            << "duration"
            << "bitrate"
            << "bpm"
            << "rating";

    m_trackSource = QSharedPointer<BaseTrackCache>(
            new BaseTrackCache(m_pLibrary->trackCollections()->internalCollection(), tableName, idColumn,
                               columns, false));
    m_pITunesTrackModel = new BaseExternalTrackModel(
        this, m_pLibrary->trackCollections(),
        "mixxx.db.model.itunes",
        "itunes_library",
        m_trackSource);
    m_pITunesPlaylistModel = new BaseExternalPlaylistModel(
        this, m_pLibrary->trackCollections(),
        "mixxx.db.model.itunes_playlist",
        "itunes_playlists",
        "itunes_playlist_tracks",
        m_trackSource);
    m_isActivated = false;
    m_title = tr("iTunes");

    m_database = QSqlDatabase::cloneDatabase(m_pLibrary->trackCollections()->internalCollection()->database(), "ITUNES_SCANNER");

    // Open the database connection in this thread.
    if (!m_database.open()) {
        qDebug() << "Failed to open database for iTunes scanner." << m_database.lastError();
    }
    connect(&m_future_watcher,
            &QFutureWatcher<TreeItem*>::finished,
            this,
            &ITunesFeature::onTrackCollectionLoaded);
}

ITunesFeature::~ITunesFeature() {
    m_database.close();
    m_cancelImport = true;
    m_future.waitForFinished();
    delete m_pITunesTrackModel;
    delete m_pITunesPlaylistModel;
}

BaseSqlTableModel* ITunesFeature::getPlaylistModelForPlaylist(const QString& playlist) {
    BaseExternalPlaylistModel* pModel = new BaseExternalPlaylistModel(
        this, m_pLibrary->trackCollections(),
        "mixxx.db.model.itunes_playlist",
        "itunes_playlists",
        "itunes_playlist_tracks",
        m_trackSource);
    pModel->setPlaylist(playlist);
    return pModel;
}

// static
bool ITunesFeature::isSupported() {
    // itunes db might just be elsewhere, don't rely on it being in its
    // normal place. And since we will load an itdb on any platform...
    return true;
}


QVariant ITunesFeature::title() {
    return m_title;
}

QIcon ITunesFeature::getIcon() {
    return m_icon;
}

void ITunesFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    // store the sidebar widget pointer for later use in onRightClick()
    m_pSidebarWidget = pSidebarWidget;
    // send it to BaseExternalLibraryFeature for onRightClickChild()
    BaseExternalLibraryFeature::bindSidebarWidget(pSidebarWidget);
}

void ITunesFeature::activate() {
    activate(false);
    emit enableCoverArtDisplay(false);
}

void ITunesFeature::activate(bool forceReload) {
    //qDebug("ITunesFeature::activate()");
    if (!m_isActivated || forceReload) {

        //Delete all table entries of iTunes feature
        ScopedTransaction transaction(m_database);
        clearTable("itunes_playlist_tracks");
        clearTable("itunes_library");
        clearTable("itunes_playlists");
        transaction.commit();

        emit showTrackModel(m_pITunesTrackModel);

        SettingsDAO settings(m_pTrackCollection->database());
        QString dbSetting(settings.getValue(ITDB_PATH_KEY));
        // if a path exists in the database, use it
        if (!dbSetting.isEmpty() && QFile::exists(dbSetting)) {
            m_dbfile = dbSetting;
        } else {
            // No Path in settings, try the default
            m_dbfile = getiTunesMusicPath();
        }

        QFileInfo dbFile(m_dbfile);
        if (!m_dbfile.isEmpty() && dbFile.exists()) {
            // Users of Mixxx <1.12.0 didn't support sandboxing. If we are sandboxed
            // and using a custom iTunes path then we have to ask for access to this
            // file.
            Sandbox::askForAccess(m_dbfile);
        } else {
            // if the path we got between the default and the database doesn't
            // exist, ask for a new one and use/save it if it exists
            m_dbfile = QFileDialog::getOpenFileName(
                    nullptr, tr("Select your iTunes library"), QDir::homePath(), "*.xml");
            QFileInfo dbFile(m_dbfile);
            if (m_dbfile.isEmpty() || !dbFile.exists()) {
                return;
            }

            // The user has picked a new directory via a file dialog. This means the
            // system sandboxer (if we are sandboxed) has granted us permission to
            // this folder. Create a security bookmark while we have permission so
            // that we can access the folder on future runs. We need to canonicalize
            // the path so we first wrap the directory string with a QDir.
            Sandbox::createSecurityToken(dbFile);
            settings.setValue(ITDB_PATH_KEY, m_dbfile);
        }
        m_isActivated =  true;
        // Let a worker thread do the XML parsing
        m_future = QtConcurrent::run(this, &ITunesFeature::importLibrary);
        m_future_watcher.setFuture(m_future);
        m_title = tr("(loading) iTunes");
        // calls a slot in the sidebar model such that 'iTunes (isLoading)' is displayed.
        emit featureIsLoading(this, true);
    } else {
        emit showTrackModel(m_pITunesTrackModel);
    }
    emit enableCoverArtDisplay(false);
}

void ITunesFeature::activateChild(const QModelIndex& index) {
    //qDebug() << "ITunesFeature::activateChild()" << index;
    QString playlist = index.data().toString();
    qDebug() << "Activating " << playlist;
    m_pITunesPlaylistModel->setPlaylist(playlist);
    emit showTrackModel(m_pITunesPlaylistModel);
    emit enableCoverArtDisplay(false);
}

TreeItemModel* ITunesFeature::getChildModel() {
    return &m_childModel;
}

void ITunesFeature::onRightClick(const QPoint& globalPos) {
    BaseExternalLibraryFeature::onRightClick(globalPos);
    QMenu menu(m_pSidebarWidget);
    QAction useDefault(tr("Use Default Library"), &menu);
    QAction chooseNew(tr("Choose Library..."), &menu);
    menu.addAction(&useDefault);
    menu.addAction(&chooseNew);
    QAction *chosen(menu.exec(globalPos));
    if (chosen == &useDefault) {
        SettingsDAO settings(m_database);
        settings.setValue(ITDB_PATH_KEY, QString());
        activate(true); // clears tables before parsing
    } else if (chosen == &chooseNew) {
        SettingsDAO settings(m_database);
        QString dbfile = QFileDialog::getOpenFileName(
                nullptr, tr("Select your iTunes library"), QDir::homePath(), "*.xml");

        QFileInfo dbFileInfo(dbfile);
        if (dbfile.isEmpty() || !dbFileInfo.exists()) {
            return;
        }
        // The user has picked a new directory via a file dialog. This means the
        // system sandboxer (if we are sandboxed) has granted us permission to
        // this folder. Create a security bookmark while we have permission so
        // that we can access the folder on future runs. We need to canonicalize
        // the path so we first wrap the directory string with a QDir.
        Sandbox::createSecurityToken(dbFileInfo);

        settings.setValue(ITDB_PATH_KEY, dbfile);
        activate(true); // clears tables before parsing
    }
}

QString ITunesFeature::getiTunesMusicPath() {
    QString musicFolder;
#if defined(__APPLE__)
    musicFolder = QStandardPaths::writableLocation(QStandardPaths::MusicLocation)
                  + "/iTunes/iTunes Music Library.xml";
#elif defined(__WINDOWS__)
    musicFolder = QStandardPaths::writableLocation(QStandardPaths::MusicLocation)
                  + "\\iTunes\\iTunes Music Library.xml";
#else
    musicFolder = "";
#endif
    qDebug() << "ITunesLibrary=[" << musicFolder << "]";
    return musicFolder;
}

void ITunesFeature::guessMusicLibraryMountpoint(QXmlStreamReader& xml) {
    // Normally the Folder Layout it some thing like that
    // iTunes/
    // iTunes/Album Artwork
    // iTunes/iTunes Media <- this is the "Music Folder"
    // iTunes/iTunes Music Library.xml <- this location we already knew
    QString music_folder = QUrl(xml.readElementText()).toLocalFile();

    QString music_folder_test = music_folder;
    music_folder_test.replace(localhost_token(), "");
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
        qDebug() << "ERROR: Couldn't find a suitable transformation to load iTunes data files. Leaving defaults intact.";
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

    m_dbItunesRoot = music_folder.left(musicFolderLcsIndex);
    m_mixxxItunesRoot = m_dbfile.left(dbfileLcsIndex);
    qDebug() << "Detected translation rule for iTunes files:"
             << m_dbItunesRoot << "->" << m_mixxxItunesRoot;
}

// This method is executed in a separate thread
// via QtConcurrent::run
TreeItem* ITunesFeature::importLibrary() {
    bool isTracksParsed=false;
    bool isMusicFolderLocatedAfterTracks=false;

    //Give thread a low priority
    QThread* thisThread = QThread::currentThread();
    thisThread->setPriority(QThread::LowPriority);

    qDebug() << "ITunesFeature::importLibrary() ";

    ScopedTransaction transaction(m_database);

    // By default set m_mixxxItunesRoot and m_dbItunesRoot to strip out
    // file://localhost/ from the URL. When we load the user's iTunes XML
    // configuration we may replace this with something based on the detected
    // location of the user's iTunes path but the defaults are necessary in case
    // their iTunes XML does not include the "Music Folder" key.
    m_mixxxItunesRoot = "";
    m_dbItunesRoot = localhost_token();

    //Parse iTunes XML file using SAX (for performance)
    QFile itunes_file(m_dbfile);
    if (!itunes_file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open iTunes music collection";
        return nullptr;
    }

    QXmlStreamReader xml(&itunes_file);
    TreeItem* playlist_root = nullptr;
    while (!xml.atEnd() && !m_cancelImport) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == "key") {
                QString key = xml.readElementText();
                if (key == "Music Folder") {
                    if (isTracksParsed) {
                        isMusicFolderLocatedAfterTracks = true;
                    }
                    if (readNextStartElement(xml)) {
                        guessMusicLibraryMountpoint(xml);
                    }
                } else if (key == "Tracks") {
                    parseTracks(xml);
                    if (playlist_root != nullptr) {
                        delete playlist_root;
                    }
                    playlist_root = parsePlaylists(xml);
                    isTracksParsed = true;
                }
            }
        }
    }

    itunes_file.close();

    if (isMusicFolderLocatedAfterTracks) {
        qDebug() << "Updating iTunes real path from " << m_dbItunesRoot << " to " << m_mixxxItunesRoot;
        // In some iTunes files "Music Folder" XML node is located at the end of file. So, we need to
        QSqlQuery query(m_database);
        query.prepare("UPDATE itunes_library SET location = replace( location, :itunes_path, :mixxx_path )");
        query.bindValue(":itunes_path", m_dbItunesRoot.replace(localhost_token(), ""));
        query.bindValue(":mixxx_path", m_mixxxItunesRoot);
        bool success = query.exec();

        if (!success) {
            LOG_FAILED_QUERY(query);
        }
    }

    // Even if an error occurred, commit the transaction. The file may have been
    // half-parsed.
    transaction.commit();

    if (xml.hasError()) {
        // do error handling
        qDebug() << "Abort processing iTunes music collection";
        qDebug() << "line:" << xml.lineNumber() <<
                "column:" << xml.columnNumber() <<
                "error:" << xml.errorString();
        if (playlist_root) {
            delete playlist_root;
        }
        playlist_root = nullptr;
    }
    return playlist_root;
}

void ITunesFeature::parseTracks(QXmlStreamReader& xml) {
    bool in_container_dictionary = false;
    bool in_track_dictionary = false;
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO itunes_library (id, artist, title, album, album_artist, year, genre, grouping, comment, tracknumber,"
                  "bpm, bitrate,"
                  "duration, location,"
                  "rating ) "
                  "VALUES (:id, :artist, :title, :album, :album_artist, :year, :genre, :grouping, :comment, :tracknumber,"
                  ":bpm, :bitrate,"
                  ":duration, :location," ":rating )");

    qDebug() << "Parse iTunes music collection";

    // read all sunsequent <dict> until we reach the closing ENTRY tag
    while (!xml.atEnd() && !m_cancelImport) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == kDict) {
                if (!in_track_dictionary && !in_container_dictionary) {
                    in_container_dictionary = true;
                    continue;
                } else if (in_container_dictionary && !in_track_dictionary) {
                    // We are in a <dict> tag that holds track information
                    in_track_dictionary = true;
                    // Parse track here
                    parseTrack(xml, query);
                }
            }
        }

        if (xml.isEndElement() && xml.name() == kDict) {
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

void ITunesFeature::parseTrack(QXmlStreamReader& xml, QSqlQuery& query) {
    //qDebug() << "----------------TRACK-----------------";
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

    //duration of a track
    int playtime = 0;
    int rating = 0;
    QString comment;
    QString tracknumber;
    QString tracktype;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == kKey) {
                QString key = xml.readElementText();

                QString content;
                if (readNextStartElement(xml)) {
                    content = xml.readElementText();
                }

                //qDebug() << "Key: " << key << " Content: " << content;

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
                    bitrate =  content.toInt();
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
                    location = TrackFile::fromUrl(QUrl(content)).location();
                    // Replace first part of location with the mixxx iTunes Root
                    // on systems where iTunes installed it only strips //localhost
                    // on iTunes from foreign systems the mount point is replaced
                    if (!m_dbItunesRoot.isEmpty()) {
                        location.replace(m_dbItunesRoot, m_mixxxItunesRoot);
                    }
                    continue;
                }
                if (key == kTrackNumber) {
                    tracknumber = content;
                    continue;
                }
                if (key == kRating) {
                    //value is an integer and ranges from 0 to 100
                    rating = (content.toInt() / 20);
                    continue;
                }
                if (key == kTrackType) {
                    tracktype = content;
                    continue;
                }
            }
        }
        //exit loop on closing </dict>
        if (xml.isEndElement() && xml.name() == kDict) {
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

TreeItem* ITunesFeature::parsePlaylists(QXmlStreamReader& xml) {
    qDebug() << "Parse iTunes playlists";
    std::unique_ptr<TreeItem> pRootItem = TreeItem::newRoot(this);
    QSqlQuery query_insert_to_playlists(m_database);
    query_insert_to_playlists.prepare("INSERT INTO itunes_playlists (id, name) "
                                      "VALUES (:id, :name)");

    QSqlQuery query_insert_to_playlist_tracks(m_database);
    query_insert_to_playlist_tracks.prepare(
        "INSERT INTO itunes_playlist_tracks (playlist_id, track_id, position) "
        "VALUES (:playlist_id, :track_id, :position)");

    while (!xml.atEnd() && !m_cancelImport) {
        xml.readNext();
        //We process and iterate the <dict> tags holding playlist summary information here
        if (xml.isStartElement() && xml.name() == kDict) {
            parsePlaylist(xml,
                          query_insert_to_playlists,
                          query_insert_to_playlist_tracks,
                          pRootItem.get());
            continue;
        }
        if (xml.isEndElement()) {
            if (xml.name() == "array") {
                break;
            }
        }
    }
    return pRootItem.release();
}

bool ITunesFeature::readNextStartElement(QXmlStreamReader& xml) {
    QXmlStreamReader::TokenType token = QXmlStreamReader::NoToken;
    while (token != QXmlStreamReader::EndDocument && token != QXmlStreamReader::Invalid) {
        token = xml.readNext();
        if (token == QXmlStreamReader::StartElement) {
            return true;
        }
    }
    return false;
}

void ITunesFeature::parsePlaylist(QXmlStreamReader& xml, QSqlQuery& query_insert_to_playlists,
                                  QSqlQuery& query_insert_to_playlist_tracks, TreeItem* root) {
    //qDebug() << "Parse Playlist";

    QString playlistname;
    int playlist_id = -1;
    int playlist_position = -1;
    int track_reference = -1;
    //indicates that we haven't found the <
    bool isSystemPlaylist = false;
    bool isPlaylistItemsStarted = false;

    //We process and iterate the <dict> tags holding playlist summary information here
    while (!xml.atEnd() && !m_cancelImport) {
        xml.readNext();

        if (xml.isStartElement()) {

            if (xml.name() == kKey) {
                QString key = xml.readElementText();
                // The rules are processed in sequence
                // That is, XML is ordered.
                // For iTunes Playlist names are always followed by the ID.
                // Afterwars the playlist entries occur
                if (key == "Name") {
                    readNextStartElement(xml);
                    playlistname = xml.readElementText();
                    continue;
                }
                //When parsing the ID, the playlistname has already been found
                if (key == "Playlist ID") {
                    readNextStartElement(xml);
                    playlist_id = xml.readElementText().toInt();
                    playlist_position = 1;
                    continue;
                }
                //Hide playlists that are system playlists
                if (key == "Master" || key == "Movies" || key == "TV Shows" ||
                    key == "Music" || key == "Books" || key == "Purchased") {
                    isSystemPlaylist = true;
                    continue;
                }

                if (key == "Playlist Items") {
                    isPlaylistItemsStarted = true;

                    //if the playlist is prebuild don't hit the database
                    if (isSystemPlaylist) {
                        continue;
                    }
                    query_insert_to_playlists.bindValue(":id", playlist_id);
                    query_insert_to_playlists.bindValue(":name", playlistname);

                    bool success = query_insert_to_playlists.exec();
                    if (!success) {
                        if (query_insert_to_playlists.lastError().nativeErrorCode() == QString::number(SQLITE_CONSTRAINT)) {
                            // We assume a duplicate Playlist name
                            playlistname += QString(" #%1").arg(playlist_id);
                            query_insert_to_playlists.bindValue(":name", playlistname );

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
                    //append the playlist to the child model
                    root->appendChild(playlistname);
                }
                // When processing playlist entries, playlist name and id have
                // already been processed and persisted
                if (key == kTrackId) {

                    readNextStartElement(xml);
                    track_reference = xml.readElementText().toInt();

                    query_insert_to_playlist_tracks.bindValue(":playlist_id", playlist_id);
                    query_insert_to_playlist_tracks.bindValue(":track_id", track_reference);
                    query_insert_to_playlist_tracks.bindValue(":position", playlist_position++);

                    //Insert tracks if we are not in a pre-build playlist
                    if (!isSystemPlaylist && !query_insert_to_playlist_tracks.exec()) {
                        qDebug() << "SQL Error in ITunesFeature.cpp: line" << __LINE__ << " "
                                 << query_insert_to_playlist_tracks.lastError();
                        qDebug() << "trackid" << track_reference;
                        qDebug() << "playlistname; " << playlistname;
                        qDebug() << "-----------------";
                    }
                }
            }
        }
        if (xml.isEndElement()) {
            if (xml.name() == "array") {
                //qDebug() << "exit playlist";
                break;
            }
            if (xml.name() == kDict && !isPlaylistItemsStarted){
                // Some playlists can be empty, so we need to exit.
                break;
            }
        }
    }
}

void ITunesFeature::clearTable(const QString& table_name) {
    QSqlQuery query(m_database);
    query.prepare("delete from "+table_name);
    bool success = query.exec();

    if (!success) {
        qDebug() << "Could not delete remove old entries from table "
                 << table_name << " : " << query.lastError();
    } else {
        qDebug() << "iTunes table entries of '"
                 << table_name <<"' have been cleared.";
    }
}

void ITunesFeature::onTrackCollectionLoaded() {
    std::unique_ptr<TreeItem> root(m_future.result());
    if (root) {
        m_childModel.setRootItem(std::move(root));

        // Tell the rhythmbox track source that it should re-build its index.
        m_trackSource->buildIndex();

        //m_pITunesTrackModel->select();
        emit showTrackModel(m_pITunesTrackModel);
        qDebug() << "Itunes library loaded: success";
    } else {
        QMessageBox::warning(
                nullptr,
                tr("Error Loading iTunes Library"),
                tr("There was an error loading your iTunes library. Some of "
                   "your iTunes tracks or playlists may not have loaded."));
    }
    // calls a slot in the sidebarmodel such that 'isLoading' is removed from the feature title.
    m_title = tr("iTunes");
    emit featureLoadingFinished(this);
    activate();
}
