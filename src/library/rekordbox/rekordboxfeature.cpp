// rekordboxfeature.cpp
// Created 05/24/2019 by Evan Dekker

#include <QtDebug>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QMap>
#include <QSettings>
#include <QStandardPaths>

#include "library/rekordbox/rekordboxfeature.h"

#include "library/librarytablemodel.h"
#include "library/missingtablemodel.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/treeitem.h"
#include "track/keyfactory.h"
#include "waveform/waveform.h"
#include "util/sandbox.h"
#include "util/file.h"

#include "widget/wlibrary.h"
#include "widget/wlibrarytextbrowser.h"

#define IS_RECORDBOX_DEVICE "::isRecordboxDevice::"
#define IS_NOT_RECORDBOX_DEVICE "::isNotRecordboxDevice::"

RekordboxPlaylistModel::RekordboxPlaylistModel(QObject* parent,
                                           TrackCollection* trackCollection,
                                           QSharedPointer<BaseTrackCache> trackSource)
        : BaseExternalPlaylistModel(parent, trackCollection,
                                    "mixxx.db.model.rekordbox.playlistmodel",
                                    "rekordbox_playlists",
                                    "rekordbox_playlist_tracks",
                                    trackSource) {
}

TrackPointer RekordboxPlaylistModel::getTrack(const QModelIndex& index) const {
    qDebug() << "RekordboxTrackModel::getTrack";    

    TrackPointer track = BaseExternalPlaylistModel::getTrack(index);

    // Assume that the key of the file the has been analyzed in Recordbox is correct
    // and prevent the AnalyzerKey from re-analyzing.
    track->setKeys(KeyFactory::makeBasicKeysFromText(index.sibling(
            index.row(), fieldIndex("key")).data().toString(), mixxx::track::io::key::USER));

    return track;
}

bool RekordboxPlaylistModel::isColumnHiddenByDefault(int column) {
    if (
        column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE) ||
        column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ID)
    ) {
        return true;
    }
    return BaseSqlTableModel::isColumnHiddenByDefault(column);
}

RekordboxFeature::RekordboxFeature(QObject* parent, TrackCollection* trackCollection)
        : BaseExternalLibraryFeature(parent, trackCollection),
          m_pTrackCollection(trackCollection),
          m_icon(":/images/library/ic_library_rekordbox.svg") {
    QString tableName = "rekordbox_library";
    QString idColumn = "id";
    QStringList columns;
    columns << "id"
            << "artist"
            << "title"
            << "album"
            << "year"
            << "genre"
            << "tracknumber"
            << "location"
            << "comment"
            << "rating"
            << "duration"
            << "bitrate"
            << "bpm"
            << "key";
    m_trackSource = QSharedPointer<BaseTrackCache>(
            new BaseTrackCache(m_pTrackCollection, tableName, idColumn,
                           columns, false));
    QStringList searchColumns;
    searchColumns
            << "artist"
            << "title"
            << "album"
            << "year"
            << "genre"
            << "tracknumber"
            << "location"
            << "comment"
            << "duration"
            << "bitrate"
            << "bpm"
            << "key";
    m_trackSource->setSearchColumns(searchColumns);

    m_pRekordboxPlaylistModel = new RekordboxPlaylistModel(this, m_pTrackCollection, m_trackSource);

    m_title = tr("Rekordbox");

    m_database = QSqlDatabase::cloneDatabase(trackCollection->database(),
                                             "REKORDBOX_SCANNER");

    //Open the database connection in this thread.
    if (!m_database.open()) {
        qDebug() << "Failed to open database for Rekordbox scanner."
                 << m_database.lastError();
    } else {
        //Clear any previous Rekordbox device entries if they exist
        ScopedTransaction transaction(m_database);    
        clearTable("rekordbox_playlist_tracks");
        clearTable("rekordbox_library");
        clearTable("rekordbox_playlists");
        transaction.commit();        
    }

    connect(&m_devicesFutureWatcher, SIGNAL(finished()),
            this, SLOT(onRekordboxDevicesFound()));
    connect(&m_tracksFutureWatcher, SIGNAL(finished()),
            this, SLOT(onTracksFound()));
    // initialize the model
    m_childModel.setRootItem(std::make_unique<TreeItem>(this));

}

RekordboxFeature::~RekordboxFeature() {
    m_database.close();
    m_devicesFuture.waitForFinished();
    m_tracksFuture.waitForFinished();
    delete m_pRekordboxPlaylistModel;
}

void RekordboxFeature::bindWidget(WLibrary* libraryWidget,
                              KeyboardEventFilter* keyboard) {
    Q_UNUSED(keyboard);
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    edit->setHtml(formatRootViewHtml());
    edit->setOpenLinks(false);
    connect(edit, SIGNAL(anchorClicked(const QUrl)),
            this, SLOT(htmlLinkClicked(const QUrl)));
    libraryWidget->registerView("REKORDBOXHOME", edit);
}

void RekordboxFeature::htmlLinkClicked(const QUrl& link) {
    if (QString(link.path()) == "refresh") {
        activate();
    } else {
        qDebug() << "Unknown link clicked" << link;
    }
}

BaseSqlTableModel* RekordboxFeature::getPlaylistModelForPlaylist(QString playlist) {
    RekordboxPlaylistModel* model = new RekordboxPlaylistModel(this, m_pTrackCollection, m_trackSource);
    model->setPlaylist(playlist);
    return model;
}

QVariant RekordboxFeature::title() {
    return m_title;
}

QIcon RekordboxFeature::getIcon() {
    return m_icon;
}

bool RekordboxFeature::isSupported() {
    return true;
}

TreeItemModel* RekordboxFeature::getChildModel() {
    return &m_childModel;
}

QString RekordboxFeature::formatRootViewHtml() const {
    QString title = tr("Rekordbox");
    QString summary = tr("Reads playlists and folders from Rekordbox prepared removable devices.");

    QString html;
    QString refreshLink = tr("Check for attached Rekordbox devices (refresh)");
    html.append(QString("<h2>%1</h2>").arg(title));
    html.append(QString("<p>%1</p>").arg(summary));

    //Colorize links in lighter blue, instead of QT default dark blue.
    //Links are still different from regular text, but readable on dark/light backgrounds.
    //https://bugs.launchpad.net/mixxx/+bug/1744816
    html.append(QString("<a style=\"color:#0496FF;\" href=\"refresh\">%1</a>")
                .arg(refreshLink));
    return html;
}

void RekordboxFeature::refreshLibraryModels() {
}

void RekordboxFeature::activate() {    
    qDebug() << "RekordboxFeature::activate()";
    
    // Usually the maximum number of threads
    // is > 2 depending on the CPU cores
    // Unfortunately, within VirtualBox
    // the maximum number of allowed threads
    // is 1 at all times We'll need to increase
    // the number to > 1, otherwise importing the music collection
    // takes place when the GUI threads terminates, i.e., on
    // Mixxx shutdown.
    QThreadPool::globalInstance()->setMaxThreadCount(4); //Tobias decided to use 4
    // Let a worker thread do the XML parsing
    m_devicesFuture = QtConcurrent::run(this, &RekordboxFeature::findRekordboxDevices);
    m_devicesFutureWatcher.setFuture(m_devicesFuture);
    m_title = tr("(loading) Rekordbox");
    //calls a slot in the sidebar model such that 'Rekordbox (isLoading)' is displayed.
    emit(featureIsLoading(this, true));

    emit(enableCoverArtDisplay(true));  
    emit(switchToView("REKORDBOXHOME"));
   
}

void RekordboxFeature::activateChild(const QModelIndex& index) {
    if (!index.isValid()) return;

    //access underlying TreeItem object
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    if (!(item && item->getData().isValid())) {
        return;
    }

    // TreeItem list data holds 2 values in a QList and have different meanings.
    // If the 2nd QList element IS_RECORDBOX_DEVICE, the 1st element is the 
    // filesystem device path, and the parseDeviceDB concurrent thread to parse
    // the Rekcordbox database is initiated. If the 2nd element is 
    // IS_NOT_RECORDBOX_DEVICE, the 1st element is the playlist path and it is
    // activated.
    QList<QVariant> data = item->getData().toList();
    QString playlist = data[0].toString();
    bool doParseDeviceDB = data[1].toString() == IS_RECORDBOX_DEVICE;

    qDebug() << "RekordboxFeature::activateChild " << item->getLabel()
             << " playlist: " << playlist << " doParseDeviceDB: " << doParseDeviceDB;

    if (doParseDeviceDB) {
        qDebug() << "Parse Rekordbox Device DB: " << playlist;

        // Usually the maximum number of threads
        // is > 2 depending on the CPU cores
        // Unfortunately, within VirtualBox
        // the maximum number of allowed threads
        // is 1 at all times We'll need to increase
        // the number to > 1, otherwise importing the music collection
        // takes place when the GUI threads terminates, i.e., on
        // Mixxx shutdown.
        QThreadPool::globalInstance()->setMaxThreadCount(4); //Tobias decided to use 4
        // Let a worker thread do the XML parsing
        m_tracksFuture = QtConcurrent::run(this, &RekordboxFeature::parseDeviceDB, item);
        m_tracksFutureWatcher.setFuture(m_tracksFuture);

        // This device is now a playlist element, future activations should treat is
        // as such
        data[1] = QVariant(IS_NOT_RECORDBOX_DEVICE);
        item->setData(QVariant(data));
    } else {
        qDebug() << "Activate Rekordbox Playlist: " << playlist;
        m_pRekordboxPlaylistModel->setPlaylist(playlist);
        emit(showTrackModel(m_pRekordboxPlaylistModel));
    }
}

QList<TreeItem*> RekordboxFeature::findRekordboxDevices() {
    QThread* thisThread = QThread::currentThread();
    thisThread->setPriority(QThread::LowPriority);

    QList<TreeItem*> foundDevices;

#if defined(__WINDOWS__)
    // Repopulate drive list
    QFileInfoList drives = QDir::drives();
    // show drive letters
    foreach (QFileInfo drive, drives) {
        // Using drive.filePath() instead of drive.canonicalPath() as it
        // freezes interface too much if there is a network share mounted
        // (drive letter assigned) but unavailable
        //
        // drive.canonicalPath() make a system call to the underlying filesystem
        // introducing delay if it is unreadable.
        // drive.filePath() doesn't make any access to the filesystem and consequently
        // shorten the delay

        QFileInfo rbDBFileInfo(drive.filePath() + PDB_PATH);

        if (rbDBFileInfo.exists() && rbDBFileInfo.isFile()) {
            TreeItem *foundDevice = new TreeItem(this);
            QList<QString> data;

            QString displayPath = drive.filePath();
            if (displayPath.endsWith("/")) {
                displayPath.chop(1);
            }

            data << drive.filePath();
            data << IS_RECORDBOX_DEVICE;

            foundDevice->setLabel(displayPath);
            foundDevice->setData(QVariant(data));

            foundDevices << foundDevice;
        }
    }
#elif defined(__LINUX__)
    // To get devices on Linux, we look for directories under /media and
    // /run/media/$USER.
    QFileInfoList devices;

    // Add folders under /media to devices.
    devices += QDir("/media").entryInfoList(
        QDir::AllDirs | QDir::NoDotAndDotDot);

    // Add folders under /media/$USER to devices.
    QDir media_user_dir("/media/" + qgetenv("USER"));
    devices += media_user_dir.entryInfoList(
        QDir::AllDirs | QDir::NoDotAndDotDot);

    // Add folders under /run/media/$USER to devices.
    QDir run_media_user_dir("/run/media/" + qgetenv("USER"));
    devices += run_media_user_dir.entryInfoList(
        QDir::AllDirs | QDir::NoDotAndDotDot);

    foreach(QFileInfo device, devices) {    
         QFileInfo rbDBFileInfo(device.filePath() + "/" + PDB_PATH);

        if (rbDBFileInfo.exists() && rbDBFileInfo.isFile()) {
            TreeItem *foundDevice = new TreeItem(this);
            QList<QString> data;

            data << device.filePath();
            data << IS_RECORDBOX_DEVICE;

            foundDevice->setLabel(device.fileName());
            foundDevice->setData(QVariant(data));

            foundDevices << foundDevice;                     
        }       
    }
#else // __APPLE__
    QFileInfoList devices = QDir("/Volumes").entryInfoList(
        QDir::AllDirs | QDir::NoDotAndDotDot);

    foreach(QFileInfo device, devices) {  
         QFileInfo rbDBFileInfo(device.filePath() + "/" + PDB_PATH);

        if (rbDBFileInfo.exists() && rbDBFileInfo.isFile()) {
            TreeItem *foundDevice = new TreeItem(this);
            QList<QString> data;

            data << device.filePath();
            data << IS_RECORDBOX_DEVICE;

            foundDevice->setLabel(device.fileName());
            foundDevice->setData(QVariant(data));

            foundDevices << foundDevice;               
        }   
    }    
#endif

     return foundDevices;
}

// Functions getText and parseDeviceDB are roughly based on the following Java file:
// https://github.com/Deep-Symmetry/crate-digger/blob/master/src/main/java/org/deepsymmetry/cratedigger/Database.java
// getText is needed because the strings in the PDB file "have a variety of obscure representations".

std::string RekordboxFeature::getText(rekordbox_pdb_t::device_sql_string_t *deviceString) {
    if (instanceof<rekordbox_pdb_t::device_sql_short_ascii_t>(deviceString->body())) {
        rekordbox_pdb_t::device_sql_short_ascii_t *shortAsciiString = 
            static_cast<rekordbox_pdb_t::device_sql_short_ascii_t*>(deviceString->body());
        return shortAsciiString->text();
    } else if (instanceof<rekordbox_pdb_t::device_sql_long_ascii_t>(deviceString->body())) {
        rekordbox_pdb_t::device_sql_long_ascii_t *longAsciiString = 
            static_cast<rekordbox_pdb_t::device_sql_long_ascii_t*>(deviceString->body());
        return longAsciiString->text();
    } else if (instanceof<rekordbox_pdb_t::device_sql_long_utf16be_t>(deviceString->body())) {
        rekordbox_pdb_t::device_sql_long_utf16be_t *longUtf16beString = 
            static_cast<rekordbox_pdb_t::device_sql_long_utf16be_t*>(deviceString->body());
        return longUtf16beString->text();
    }

    return std::string("");
}

QString RekordboxFeature::parseDeviceDB(TreeItem *deviceItem) {
    QString device = deviceItem->getLabel();
    QString devicePath = deviceItem->getData().toList()[0].toString();

    qDebug() << "parseDeviceDB device: " << device << " devicePath: " << devicePath;

    std::string pathBase = devicePath.toStdString() + "/";
    std::string dbPath = pathBase + std::string(PDB_PATH);

    if (!QFile(QString::fromStdString(dbPath)).exists()) {
        return devicePath;
    }   

    //Give thread a low priority
    QThread* thisThread = QThread::currentThread();
    thisThread->setPriority(QThread::LowPriority);

    ScopedTransaction transaction(m_database);    
   
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO rekordbox_library (rb_id, artist, title, album, year,"
                  "genre,comment,tracknumber,bpm, bitrate,duration, location,"
                  "rating,key,analyze_path,device) VALUES (:rb_id, :artist, :title, :album, :year,:genre,"
                  ":comment, :tracknumber,:bpm, :bitrate,:duration, :location,"
                  ":rating,:key,:analyze_path,:device)");

    int audioFilesCount = 0;

    // Create a playlist for all the tracks on a device
    QSqlQuery queryInsertIntoDevicePlaylist(m_database);
    queryInsertIntoDevicePlaylist.prepare("INSERT INTO rekordbox_playlists (name) "
        "VALUES (:name)");

    queryInsertIntoDevicePlaylist.bindValue(":name", devicePath);

    if (!queryInsertIntoDevicePlaylist.exec()) {
        LOG_FAILED_QUERY(queryInsertIntoDevicePlaylist)
            << "devicePath: " << devicePath;
        return devicePath;
    }        

    QSqlQuery idQuery(m_database);
    idQuery.prepare("select id from rekordbox_playlists where name=:path");
    idQuery.bindValue(":path", devicePath);

    if (!idQuery.exec()) {
        LOG_FAILED_QUERY(idQuery)
            << "devicePath: " << devicePath;
        return devicePath;
    }        

    int playlistID = -1;
    while (idQuery.next()) {
        playlistID = idQuery.value(idQuery.record().indexOf("id")).toInt();
    }        

    QSqlQuery queryInsertIntoDevicePlaylistTracks(m_database);
    queryInsertIntoDevicePlaylistTracks.prepare(
        "INSERT INTO rekordbox_playlist_tracks (playlist_id, track_id, position) "
        "VALUES (:playlist_id, :track_id, :position)");     

    queryInsertIntoDevicePlaylistTracks.bindValue(":playlist_id", playlistID);

    std::ifstream ifs(dbPath, std::ifstream::binary);
    kaitai::kstream ks(&ifs);

    rekordbox_pdb_t reckordboxDB = rekordbox_pdb_t(&ks);

    // There are other types of tables (eg. COLOR), these are the only ones we are
    // interested at the moment. Perhaps when/if 
    // https://bugs.launchpad.net/mixxx/+bug/1100882
    // is completed, this can be revisted. 
    // Attempt was made to also recover HISTORY
    // playlists (which are found on removable Rekordbox devices), however
    // they didn't appear to contain valid row_ref_t structures.
    const int totalTables = 8;

    rekordbox_pdb_t::page_type_t tableOrder[totalTables] = {
        rekordbox_pdb_t::PAGE_TYPE_KEYS,
        rekordbox_pdb_t::PAGE_TYPE_GENRES,
        rekordbox_pdb_t::PAGE_TYPE_ARTISTS,
        rekordbox_pdb_t::PAGE_TYPE_ALBUMS,
        rekordbox_pdb_t::PAGE_TYPE_PLAYLIST_ENTRIES,
        rekordbox_pdb_t::PAGE_TYPE_TRACKS,
        rekordbox_pdb_t::PAGE_TYPE_PLAYLIST_TREE,
        rekordbox_pdb_t::PAGE_TYPE_HISTORY
    };

    std::map<uint32_t, std::string> keysMap;
    std::map<uint32_t, std::string> genresMap;
    std::map<uint32_t, std::string> artistsMap;
    std::map<uint32_t, std::string> albumsMap;
    std::map<uint32_t, std::string> playlistNameMap;
    std::map<uint32_t, bool> playlistIsFolderMap;
    std::map<uint32_t, std::map<uint32_t, uint32_t>> playlistTreeMap;
    std::map<uint32_t, std::map<uint32_t, uint32_t>> playlistTrackMap;

    bool folderOrPlaylistFound = false;

    for (int tableOrderIndex = 0; tableOrderIndex < totalTables; tableOrderIndex++) {
        bool done = false;

        for (
            std::vector<rekordbox_pdb_t::table_t*>::iterator table = reckordboxDB.tables()->begin(); 
            table != reckordboxDB.tables()->end(); ++table
        ) {
            if ((*table)->type() == tableOrder[tableOrderIndex]) {
                if (done) break;

                uint16_t lastIndex = (*table)->last_page()->index();
                rekordbox_pdb_t::page_ref_t *currentRef = (*table)->first_page();
                bool moreLeft = true;

                do {
                    rekordbox_pdb_t::page_t *page = currentRef->body();

                    if (page->is_data_page()) {
                        for (
                            std::vector<rekordbox_pdb_t::row_group_t*>::iterator rowGroup = page->row_groups()->begin(); 
                            rowGroup != page->row_groups()->end(); ++rowGroup
                        ) {
                            for (
                                std::vector<rekordbox_pdb_t::row_ref_t*>::iterator rowRef = (*rowGroup)->rows()->begin();
                                rowRef != (*rowGroup)->rows()->end(); ++rowRef
                            ) {
                                if ((*rowRef)->present()) {
                                    switch (tableOrder[tableOrderIndex]) {
                                        case rekordbox_pdb_t::PAGE_TYPE_KEYS: {
                                            // Key found, update map
                                            rekordbox_pdb_t::key_row_t *key = 
                                                (rekordbox_pdb_t::key_row_t *)(*rowRef)->body();
                                            keysMap[key->id()] = getText(key->name());
                                        }
                                        break;
                                        case rekordbox_pdb_t::PAGE_TYPE_GENRES: {
                                            // Genre found, update map
                                            rekordbox_pdb_t::genre_row_t *genre =
                                                (rekordbox_pdb_t::genre_row_t *)(*rowRef)->body();
                                            genresMap[genre->id()] = getText(genre->name());
                                        }
                                        break;
                                        case rekordbox_pdb_t::PAGE_TYPE_ARTISTS: {
                                            // Artist found, update map
                                            rekordbox_pdb_t::artist_row_t *artist =
                                                (rekordbox_pdb_t::artist_row_t *)(*rowRef)->body();
                                            artistsMap[artist->id()] = getText(artist->name());
                                        }
                                        break;
                                        case rekordbox_pdb_t::PAGE_TYPE_ALBUMS: {
                                            // Album found, update map
                                            rekordbox_pdb_t::album_row_t *album =
                                                (rekordbox_pdb_t::album_row_t *)(*rowRef)->body();
                                            albumsMap[album->id()] = getText(album->name());
                                        }
                                        break;
                                        case rekordbox_pdb_t::PAGE_TYPE_PLAYLIST_ENTRIES: {
                                            // Playlist to track mapping found, update map
                                            rekordbox_pdb_t::playlist_entry_row_t *playlistEntry =
                                                (rekordbox_pdb_t::playlist_entry_row_t *)(*rowRef)->body();
                                            playlistTrackMap[playlistEntry->playlist_id()][playlistEntry->entry_index()] = 
                                                playlistEntry->track_id();
                                        }
                                        break;                                              
                                        case rekordbox_pdb_t::PAGE_TYPE_TRACKS: {        
                                            // Track found, insert into database
                                            rekordbox_pdb_t::track_row_t *track = (rekordbox_pdb_t::track_row_t *)(*rowRef)->body();

                                            int rbID = (int)track->id();
                                            QString title = QString::fromStdString(getText(track->title()));
                                            QString artist = QString::fromStdString(artistsMap[track->artist_id()]);
                                            QString album = QString::fromStdString(albumsMap[track->album_id()]);
                                            QString year = QString::number(track->year());
                                            QString genre = QString::fromStdString(genresMap[track->genre_id()]);
                                            QString location = QString::fromStdString(pathBase + getText(track->file_path()));
                                            float bpm = (float)track->tempo() / 100.0;
                                            int bitrate = (int)track->bitrate();
                                            QString key = QString::fromStdString(keysMap[track->key_id()]);
                                            int playtime = (int)track->duration();
                                            int rating = (int)track->rating();
                                            QString comment = QString::fromStdString(getText(track->comment()));
                                            QString tracknumber = QString::number(track->track_number());   
                                            QString anlzPath = QString::fromStdString(pathBase + getText(track->analyze_path()));
                                                                                      
                                            query.bindValue(":rb_id", rbID);
                                            query.bindValue(":artist", artist);
                                            query.bindValue(":title", title);
                                            query.bindValue(":album", album);
                                            query.bindValue(":genre", genre);
                                            query.bindValue(":year", year);
                                            query.bindValue(":duration", playtime);
                                            query.bindValue(":location", location);
                                            query.bindValue(":rating", rating);
                                            query.bindValue(":comment", comment);
                                            query.bindValue(":tracknumber", tracknumber);
                                            query.bindValue(":key", key);
                                            query.bindValue(":bpm", bpm);
                                            query.bindValue(":bitrate", bitrate);  
                                            query.bindValue(":analyze_path", anlzPath);   
                                            query.bindValue(":device", device);

                                            if (!query.exec()) {
                                                LOG_FAILED_QUERY(query);
                                            }              

                                            int trackID = -1;
                                            QSqlQuery finderQuery(m_database);
                                            finderQuery.prepare("select id from rekordbox_library where rb_id=:rb_id and device=:device");
                                            finderQuery.bindValue(":rb_id", rbID);
                                            finderQuery.bindValue(":device", device);

                                            if (!finderQuery.exec()) {
                                                LOG_FAILED_QUERY(finderQuery) 
                                                    << "rbID:" << rbID;
                                            }

                                            if (finderQuery.next()) {
                                                trackID = finderQuery.value(finderQuery.record().indexOf("id")).toInt();
                                            }                                            

                                            // Insert into device all tracks playlist
                                            queryInsertIntoDevicePlaylistTracks.bindValue(":track_id", trackID);
                                            queryInsertIntoDevicePlaylistTracks.bindValue(":position", audioFilesCount);

                                            if (!queryInsertIntoDevicePlaylistTracks.exec()) {
                                                LOG_FAILED_QUERY(queryInsertIntoDevicePlaylistTracks)
                                                    << "device playlistID:" << playlistID 
                                                    << "trackID:" << trackID 
                                                    << "position:" << audioFilesCount;
                                            }                                                                               

                                            audioFilesCount++;
                                        }   
                                        break;     
                                        case rekordbox_pdb_t::PAGE_TYPE_PLAYLIST_TREE: {
                                            // Playlist tree node found, update map                                            
                                            rekordbox_pdb_t::playlist_tree_row_t *playlistTree = 
                                                (rekordbox_pdb_t::playlist_tree_row_t *)(*rowRef)->body();

                                            playlistNameMap[playlistTree->id()] = getText(playlistTree->name());
                                            playlistIsFolderMap[playlistTree->id()] = playlistTree->is_folder();
                                            playlistTreeMap[playlistTree->parent_id()][playlistTree->sort_order()] = playlistTree->id();

                                            folderOrPlaylistFound = true;
                                        }
                                        break;                                          
                                        default:
                                        break;                                                                         
                                    }
                                }
                            }
                        }
                    }

                    if (currentRef->index() == lastIndex) {
                        moreLeft = false;
                    } else {
                        currentRef = page->next_page();
                    }
                } while (moreLeft);
                done = true;
            }
        }
    }

    if (audioFilesCount > 0 || folderOrPlaylistFound) {
        // If we have found anything, recursively build playlist/folder TreeItem children
        // for the original device TreeItem
        buildPlaylistTree(deviceItem, 0, playlistNameMap, playlistIsFolderMap, 
            playlistTreeMap, playlistTrackMap, devicePath, device);
    }

    qDebug() << "Found: " << audioFilesCount << " audio files in Rekordbox device " << device;

    transaction.commit();

    return devicePath;
}

void RekordboxFeature::buildPlaylistTree(
    TreeItem *parent, 
    uint32_t parentID, 
    std::map<uint32_t, std::string> &playlistNameMap, 
    std::map<uint32_t, bool> &playlistIsFolderMap, 
    std::map<uint32_t, std::map<uint32_t, uint32_t>> &playlistTreeMap,
    std::map<uint32_t, std::map<uint32_t, uint32_t>> &playlistTrackMap,
    QString playlistPath,
    QString device) {

    for (uint32_t childIndex = 0; childIndex < playlistTreeMap[parentID].size(); childIndex++) {
        uint32_t childID = playlistTreeMap[parentID][childIndex];
        QString playlistItemName = QString::fromStdString(playlistNameMap[childID]);

        QString delimiter = "-->";
        QString currentPath = playlistPath + delimiter + playlistItemName;
        
        QList<QString> data;

        data << currentPath;
        data << IS_NOT_RECORDBOX_DEVICE;        

        TreeItem *child = parent->appendChild(playlistItemName, QVariant(data));

        // Create a playlist for this child
        QSqlQuery queryInsertIntoPlaylist(m_database);
        queryInsertIntoPlaylist.prepare("INSERT INTO rekordbox_playlists (name) "
            "VALUES (:name)");

        queryInsertIntoPlaylist.bindValue(":name", currentPath);

        if (!queryInsertIntoPlaylist.exec()) {
            LOG_FAILED_QUERY(queryInsertIntoPlaylist) 
                << "currentPath" << currentPath;
            return;
        }        

        QSqlQuery idQuery(m_database);
        idQuery.prepare("select id from rekordbox_playlists where name=:path");
        idQuery.bindValue(":path", currentPath);

        if (!idQuery.exec()) {
            LOG_FAILED_QUERY(idQuery)
                << "currentPath" << currentPath;
            return;
        }        

        int playlistID = -1;
        while (idQuery.next()) {
            playlistID = idQuery.value(idQuery.record().indexOf("id")).toInt();
        }        

        QSqlQuery queryInsertIntoPlaylistTracks(m_database);
        queryInsertIntoPlaylistTracks.prepare(
            "INSERT INTO rekordbox_playlist_tracks (playlist_id, track_id, position) "
            "VALUES (:playlist_id, :track_id, :position)");        

        if (playlistTrackMap.count(childID)) {
            // Add playlist tracks for children
            for (uint32_t trackIndex = 1; trackIndex <= playlistTrackMap[childID].size(); trackIndex++) {
                uint32_t rbTrackID = playlistTrackMap[childID][trackIndex];

                int trackID = -1;
                QSqlQuery finderQuery(m_database);
                finderQuery.prepare("select id from rekordbox_library where rb_id=:rb_id and device=:device");
                finderQuery.bindValue(":rb_id", rbTrackID);
                finderQuery.bindValue(":device", device);

                if (!finderQuery.exec()) {
                    LOG_FAILED_QUERY(finderQuery) 
                        << "rbTrackID:" << rbTrackID
                        << "device:" << device;
                    return;
                }

                if (finderQuery.next()) {
                    trackID = finderQuery.value(finderQuery.record().indexOf("id")).toInt();
                }

                queryInsertIntoPlaylistTracks.bindValue(":playlist_id", playlistID);
                queryInsertIntoPlaylistTracks.bindValue(":track_id", trackID);
                queryInsertIntoPlaylistTracks.bindValue(":position", (int)trackIndex);

                if (!queryInsertIntoPlaylistTracks.exec()) {
                    LOG_FAILED_QUERY(queryInsertIntoPlaylistTracks)
                        << "playlistID:" << playlistID
                        << "trackID:" << trackID
                        << "trackIndex:" << trackIndex;

                    return;
                }                              
            }
        }

        if (playlistIsFolderMap[childID]) {
            // If this child is a folder (playlists are only leaf nodes), build playlist tree for it
            buildPlaylistTree(child, childID, playlistNameMap, playlistIsFolderMap, 
                playlistTreeMap, playlistTrackMap, currentPath, device);
        }
    }
}                                                

void RekordboxFeature::clearTable(QString tableName) {
    QSqlQuery query(m_database);
    query.prepare("delete from " + tableName);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
            << "tableName:" << tableName;
    } else {
        query.prepare("delete from sqlite_sequence where name='" + tableName + "'");

        if (!query.exec()) {
            LOG_FAILED_QUERY(query)
                << "tableName:" << tableName;
        } else {
            qDebug() << "Rekordbox table entries of '" << tableName << "' have been cleared.";
        }
    }
}

void RekordboxFeature::onRekordboxDevicesFound() {
    QList<TreeItem*> foundDevices = m_devicesFuture.result();
    TreeItem *root = m_childModel.getRootItem();

    if (foundDevices.size() == 0) {
        // No Rekordbox devices found
        ScopedTransaction transaction(m_database);    
        clearTable("rekordbox_playlist_tracks");
        clearTable("rekordbox_library");
        clearTable("rekordbox_playlists");
        transaction.commit();          

        if (root->childRows() > 0) {
            // Devices have since been unmounted
            m_childModel.removeRows(0, root->childRows()); 
        }   
    } else {
        for (int deviceIndex = 0; deviceIndex < root->childRows(); deviceIndex++) {
            TreeItem *child = root->child(deviceIndex);
            bool removeChild = true;
        
            for (int foundDeviceIndex = 0; foundDeviceIndex < foundDevices.size(); foundDeviceIndex++) {
                TreeItem *deviceFound = foundDevices[foundDeviceIndex];

                if (deviceFound->getLabel() == child->getLabel()) {
                    removeChild = false;
                    break;
                }
            }

            if (removeChild) {   
                // Device has since been unmounted, cleanup DB             
                ScopedTransaction transaction(m_database); 

                int trackID = -1;
                int playlistID = -1;
                QSqlQuery tracksQuery(m_database);
                tracksQuery.prepare("select id from rekordbox_library where device=:device");
                tracksQuery.bindValue(":device", child->getLabel());

                QSqlQuery deletePlaylistsQuery(m_database);
                deletePlaylistsQuery.prepare("delete from rekordbox_playlists where id=:id");

                QSqlQuery deletePlaylistTracksQuery(m_database);
                deletePlaylistTracksQuery.prepare("delete from rekordbox_playlist_tracks where playlist_id=:playlist_id");                

                if (!tracksQuery.exec()) {
                    LOG_FAILED_QUERY(tracksQuery) 
                        << "device:" << child->getLabel();
                }

                while (tracksQuery.next()) {                    
                    trackID = tracksQuery.value(tracksQuery.record().indexOf("id")).toInt();

                    QSqlQuery playlistTracksQuery(m_database);
                    playlistTracksQuery.prepare("select playlist_id from rekordbox_playlist_tracks where track_id=:track_id");
                    playlistTracksQuery.bindValue(":track_id", trackID);

                    if (!playlistTracksQuery.exec()) {
                        LOG_FAILED_QUERY(playlistTracksQuery) 
                            << "trackID:" << trackID;
                    }        

                    while (playlistTracksQuery.next()) { 
                        playlistID = playlistTracksQuery.value(playlistTracksQuery.record().indexOf("playlist_id")).toInt();

                        deletePlaylistsQuery.bindValue(":id", playlistID);

                        if (!deletePlaylistsQuery.exec()) {
                            LOG_FAILED_QUERY(deletePlaylistsQuery) 
                                << "playlistID:" << playlistID;
                        }                    
                                 
                        deletePlaylistTracksQuery.bindValue(":playlist_id", playlistID);

                        if (!deletePlaylistTracksQuery.exec()) {
                            LOG_FAILED_QUERY(deletePlaylistTracksQuery) 
                                << "playlistID:" << playlistID;
                        }                    

                    }
                    
                }

                QSqlQuery deleteTracksQuery(m_database);
                deleteTracksQuery.prepare("delete from rekordbox_library where device=:device"); 
                deleteTracksQuery.bindValue(":device", child->getLabel());
                
                if (!deleteTracksQuery.exec()) {
                    LOG_FAILED_QUERY(deleteTracksQuery)
                        << "device:" << child->getLabel();                    
                }                                 

                transaction.commit(); 

                m_childModel.removeRows(deviceIndex, 1);
            }
        }

        QList<TreeItem*> childrenToAdd;

        for (int foundDeviceIndex = 0; foundDeviceIndex < foundDevices.size(); foundDeviceIndex++) {
            TreeItem *deviceFound = foundDevices[foundDeviceIndex];
            bool addNewChild = true;

            for (int deviceIndex = 0; deviceIndex < root->childRows(); deviceIndex++) {
                TreeItem *child = root->child(deviceIndex);

                if (deviceFound->getLabel() == child->getLabel()) {
                    // This device already exists in the TreeModel, don't add or parse is again
                    addNewChild = false;
                }
            }

            if (addNewChild) {
                childrenToAdd << deviceFound;
            }
        }

        if (!childrenToAdd.empty()) {
            m_childModel.insertTreeItemRows(childrenToAdd, 0);
        }
    }

    // calls a slot in the sidebarmodel such that 'isLoading' is removed from the feature title.
    m_title = tr("Rekordbox");
    emit(featureLoadingFinished(this));
}

void RekordboxFeature::onTracksFound() {
    qDebug() << "onTracksFound";
    m_childModel.triggerRepaint();

    QString devicePlaylist = m_tracksFuture.result();

    qDebug() << "Show Rekordbox Device Playlist: " << devicePlaylist;

    m_pRekordboxPlaylistModel->setPlaylist(devicePlaylist);
    emit(showTrackModel(m_pRekordboxPlaylistModel));    
}
