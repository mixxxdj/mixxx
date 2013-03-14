// traktorfeature.cpp
// Created 9/26/2010 by Tobias Rafreider

#include <QtDebug>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QMap>
#include <QSettings>
#include <QDesktopServices>

#include "library/traktor/traktorfeature.h"

#include "library/librarytablemodel.h"
#include "library/missingtablemodel.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/treeitem.h"

TraktorTrackModel::TraktorTrackModel(QObject* parent,
                                     TrackCollection* pTrackCollection)
        : BaseExternalTrackModel(parent, pTrackCollection,
                                 "mixxx.db.model.traktor_tablemodel",
                                 "traktor_library",
                                 "traktor") {
}

bool TraktorTrackModel::isColumnHiddenByDefault(int column) {
    if (column == fieldIndex(LIBRARYTABLE_KEY) ||
        column == fieldIndex(LIBRARYTABLE_BITRATE)) {
        return true;
    }
    return false;
}

TraktorPlaylistModel::TraktorPlaylistModel(QObject* parent,
                                           TrackCollection* pTrackCollection)
        : BaseExternalPlaylistModel(parent, pTrackCollection,
                                    "mixxx.db.model.traktor.playlistmodel",
                                    "traktor_playlists",
                                    "traktor_playlist_tracks",
                                    "traktor") {
}

bool TraktorPlaylistModel::isColumnHiddenByDefault(int column) {
    if (column == fieldIndex(LIBRARYTABLE_KEY) ||
        column == fieldIndex(LIBRARYTABLE_BITRATE)) {
        return true;
    }
    return false;
}

TraktorFeature::TraktorFeature(QObject* parent, TrackCollection* pTrackCollection)
        : BaseExternalLibraryFeature(parent, pTrackCollection),
          m_pTrackCollection(pTrackCollection),
          m_cancelImport(false) {
    QString tableName = "traktor_library";
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
    pTrackCollection->addTrackSource(QString("traktor"), QSharedPointer<BaseTrackCache>(
        new BaseTrackCache(m_pTrackCollection, tableName, idColumn,
                           columns, false)));

    m_isActivated = false;
    m_pTraktorTableModel = new TraktorTrackModel(this, m_pTrackCollection);
    m_pTraktorPlaylistModel = new TraktorPlaylistModel(this, m_pTrackCollection);

    m_title = tr("Traktor");

    m_database = QSqlDatabase::cloneDatabase(pTrackCollection->getDatabase(),
                                             "TRAKTOR_SCANNER");

    //Open the database connection in this thread.
    if (!m_database.open()) {
        qDebug() << "Failed to open database for iTunes scanner."
                 << m_database.lastError();
    }
    connect(&m_future_watcher, SIGNAL(finished()),
            this, SLOT(onTrackCollectionLoaded()));
}

TraktorFeature::~TraktorFeature() {
    m_database.close();
    m_cancelImport = true;
    m_future.waitForFinished();
    delete m_pTraktorTableModel;
    delete m_pTraktorPlaylistModel;
}

BaseSqlTableModel* TraktorFeature::getPlaylistModelForPlaylist(QString playlist) {
    TraktorPlaylistModel* pModel = new TraktorPlaylistModel(this, m_pTrackCollection);
    pModel->setPlaylist(playlist);
    return pModel;
}

QVariant TraktorFeature::title() {
    return m_title;
}

QIcon TraktorFeature::getIcon() {
    return QIcon(":/images/library/ic_library_traktor.png");
}

bool TraktorFeature::isSupported() {
    return (QFile::exists(getTraktorMusicDatabase()));
}

TreeItemModel* TraktorFeature::getChildModel() {
    return &m_childModel;
}

void TraktorFeature::refreshLibraryModels() {
}

void TraktorFeature::activate() {
    qDebug() << "TraktorFeature::activate()";

    if (!m_isActivated) {
        m_isActivated =  true;
        // Ususally the maximum number of threads
        // is > 2 depending on the CPU cores
        // Unfortunately, within VirtualBox
        // the maximum number of allowed threads
        // is 1 at all times We'll need to increase
        // the number to > 1, otherwise importing the music collection
        // takes place when the GUI threads terminates, i.e., on
        // Mixxx shutdown.
        QThreadPool::globalInstance()->setMaxThreadCount(4); //Tobias decided to use 4
        // Let a worker thread do the XML parsing
        m_future = QtConcurrent::run(this, &TraktorFeature::importLibrary,
                                     getTraktorMusicDatabase());
        m_future_watcher.setFuture(m_future);
        m_title = tr("(loading) Traktor");
        //calls a slot in the sidebar model such that 'iTunes (isLoading)' is displayed.
        emit (featureIsLoading(this));
    }

    emit(showTrackModel(m_pTraktorTableModel));
}

void TraktorFeature::activateChild(const QModelIndex& index) {

    if (!index.isValid()) return;

    //access underlying TreeItem object
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    if (item->isPlaylist()) {
        qDebug() << "Activate Traktor Playlist: " << item->dataPath().toString();
        m_pTraktorPlaylistModel->setPlaylist(item->dataPath().toString());
        emit(showTrackModel(m_pTraktorPlaylistModel));
    }
}

TreeItem* TraktorFeature::importLibrary(QString file) {
    //Give thread a low priority
    QThread* thisThread = QThread::currentThread();
    thisThread->setPriority(QThread::LowestPriority);
    //Invisible root item of Traktor's child model
    TreeItem* root = NULL;
    //Delete all table entries of Traktor feature
    ScopedTransaction transaction(m_database);
    clearTable("traktor_playlist_tracks");
    clearTable("traktor_library");
    clearTable("traktor_playlists");
    transaction.commit();

    transaction.transaction();
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO traktor_library (artist, title, album, year,"
                  "genre,comment,tracknumber,bpm, bitrate,duration, location,"
                  "rating,key) VALUES (:artist, :title, :album, :year,:genre,"
                  ":comment, :tracknumber,:bpm, :bitrate,:duration, :location,"
                  ":rating,:key)");

    //Parse Trakor XML file using SAX (for performance)
    QFile traktor_file(file);
    if (!traktor_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open Traktor music collection";
        return NULL;
    }
    QXmlStreamReader xml(&traktor_file);
    bool inCollectionTag = false;
    //TODO(XXX) is this still needed to parse the library correctly?
    bool inEntryTag = false;
    bool inPlaylistsTag = false;
    bool isRootFolderParsed = false;
    int nAudioFiles = 0;

    while (!xml.atEnd() && !m_cancelImport) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == "COLLECTION") {
                inCollectionTag = true;
            }
            // Each "ENTRY" tag in <COLLECTION> represents a track
            if (inCollectionTag && xml.name() == "ENTRY" ) {
                inEntryTag = true;
                //parse track
                parseTrack(xml, query);
                ++nAudioFiles; //increment number of files in the music collection
            }
            if (xml.name() == "PLAYLISTS") {
                inPlaylistsTag = true;
            } if (inPlaylistsTag && !isRootFolderParsed && xml.name() == "NODE") {
                QXmlStreamAttributes attr = xml.attributes();
                QString nodetype = attr.value("TYPE").toString();
                QString name = attr.value("NAME").toString();

                if (nodetype == "FOLDER" && name == "$ROOT") {
                    //process all playlists
                    root = parsePlaylists(xml);
                    isRootFolderParsed = true;
                }
            }
        }
        if (xml.isEndElement()) {
            if (xml.name() == "COLLECTION") {
                inCollectionTag = false;
            }
            if (xml.name() == "ENTRY" && inCollectionTag) {
                inEntryTag = false;
            }
            if (xml.name() == "PLAYLISTS" && inPlaylistsTag) {
                inPlaylistsTag = false;
            }
        }
    }
    if (xml.hasError()) {
         // do error handling
         qDebug() << "Cannot process Traktor music collection";
         if (root)
             delete root;
         return NULL;
    }

    qDebug() << "Found: " << nAudioFiles << " audio files in Traktor";
    //initialize TraktorTableModel
    transaction.commit();

    return root;
}

void TraktorFeature::parseTrack(QXmlStreamReader &xml, QSqlQuery &query) {
    QString title;
    QString artist;
    QString album;
    QString year;
    QString genre;
    //drive letter
    QString volume;
    QString path;
    QString filename;
    QString location;
    float bpm = 0.0;
    int bitrate = 0;
    QString key;
    //duration of a track
    int playtime = 0;
    int rating = 0;
    QString comment;
    QString tracknumber;

    //get XML attributes of starting ENTRY tag
    QXmlStreamAttributes attr = xml.attributes ();
    title = attr.value("TITLE").toString();
    artist = attr.value("ARTIST").toString();

    //read all sub tags of ENTRY until we reach the closing ENTRY tag
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == "ALBUM") {
                QXmlStreamAttributes attr = xml.attributes ();
                album = attr.value("TITLE").toString();
                tracknumber = attr.value("TRACK").toString();
                continue;
            }
            if (xml.name() == "LOCATION") {
                QXmlStreamAttributes attr = xml.attributes ();
                volume = attr.value("VOLUME").toString();
                path = attr.value("DIR").toString();
                filename = attr.value("FILE").toString();
                // compute the location, i.e, combining all the values
                // On Windows the volume holds the drive letter e.g., d:
                // On OS X, the volume is supposed to be "Macintosh HD" at all times,
                // which is a folder in /Volumes/
                #if defined(__APPLE__)
                location = "/Volumes/"+volume;
                #else
                location = volume;
                #endif
                location += path.replace(QString(":"), QString(""));
                location += filename;
                continue;
            }
            if (xml.name() == "INFO") {
                QXmlStreamAttributes attr = xml.attributes();
                key = attr.value("KEY").toString();
                bitrate = attr.value("BITRATE").toString().toInt() / 1000;
                playtime = attr.value("PLAYTIME").toString().toInt();
                genre = attr.value("GENRE").toString();
                year = attr.value("RELEASE_DATE").toString();
                comment = attr.value("COMMENT").toString();
                QString ranking_str = attr.value("RANKING").toString();
                // A ranking in Traktor has ranges between 0 and 255 internally.
                // This is same as the POPULARIMETER tag in IDv2,
                // see http://help.mp3tag.de/main_tags.html
                //
                // Our rating values range from 1 to 5. The mapping is defined as follow
                // ourRatingValue = TraktorRating / 51
                 if (ranking_str != "" && qVariantCanConvert<int>(ranking_str)) {
                    rating = ranking_str.toInt()/51;
                 }
                continue;
            }
            if (xml.name() == "TEMPO") {
                QXmlStreamAttributes attr = xml.attributes ();
                bpm = attr.value("BPM").toString().toFloat();
                continue;
            }
        }
        //We leave the infinte loop, if twe have the closing tag "ENTRY"
        if (xml.name() == "ENTRY" && xml.isEndElement()) {
            break;
        }
    }

    // If we reach the end of ENTRY within the COLLECTION tag
    // Save parsed track to database
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

    bool success = query.exec();
    if (!success) {
        qDebug() << "SQL Error in TraktorTableModel.cpp: line"
                 << __LINE__ << " " << query.lastError();
        return;
    }
}

// Purpose: Parsing all the folder and playlists of Traktor
// This is a complex operation since Traktor uses the concept of folders and playlist.
// A folder can contain folders and playlists. A playlist contains entries but no folders.
// In other words, Traktor uses a tree structure to organize music.
// Inner nodes represent folders while leaves are playlists.
TreeItem* TraktorFeature::parsePlaylists(QXmlStreamReader &xml) {

    qDebug() << "Process RootFolder";
    //Each playlist is unique and can be identified by a path in the tree structure.
    QString current_path = "";
    QMap<QString,QString> map;

    QString delimiter = "-->";

    TreeItem *rootItem = new TreeItem();
    TreeItem * parent = rootItem;

    bool inPlaylistTag = false;

    QSqlQuery query_insert_to_playlists(m_database);
    query_insert_to_playlists.prepare("INSERT INTO traktor_playlists (name) "
                  "VALUES (:name)");

    QSqlQuery query_insert_to_playlist_tracks(m_database);
    query_insert_to_playlist_tracks.prepare(
        "INSERT INTO traktor_playlist_tracks (playlist_id, track_id, position) "
        "VALUES (:playlist_id, :track_id, :position)");

    while (!xml.atEnd() && !m_cancelImport) {
        //read next XML element
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == "NODE") {
                QXmlStreamAttributes attr = xml.attributes();
                QString name = attr.value("NAME").toString();
                QString type = attr.value("TYPE").toString();
               //TODO: What happens if the folder node is a leaf (empty folder)
               // Idea: Hide empty folders :-)
               if (type == "FOLDER") {
                    current_path += delimiter;
                    current_path += name;
                    //qDebug() << "Folder: " +current_path << " has parent " << parent->data().toString();
                    map.insert(current_path, "FOLDER");
                    TreeItem * item = new TreeItem(name,current_path, this, parent);
                    parent->appendChild(item);
                    parent = item;
               }
               if (type == "PLAYLIST") {
                    current_path += delimiter;
                    current_path += name;
                    //qDebug() << "Playlist: " +current_path << " has parent " << parent->data().toString();
                    map.insert(current_path, "PLAYLIST");

                    TreeItem * item = new TreeItem(name,current_path, this, parent);
                    parent->appendChild(item);
                    // process all the entries within the playlist 'name' having path 'current_path'
                    parsePlaylistEntries(xml, current_path,
                                         query_insert_to_playlists,
                                         query_insert_to_playlist_tracks);
                }
            }
            if (xml.name() == "ENTRY" && inPlaylistTag) {
            }
        }

        if (xml.isEndElement()) {
            if (xml.name() == "NODE") {
                if (map.value(current_path) == "FOLDER") {
                    parent = parent->parent();
                }

                //Whenever we find a closing NODE, remove the last component of the path
                int lastSlash = current_path.lastIndexOf (delimiter);
                int path_length = current_path.size();

                current_path.remove(lastSlash, path_length - lastSlash);
            }
            if (xml.name() == "PLAYLIST") {
                inPlaylistTag = false;
            }
            //We leave the infinte loop, if twe have the closing "PLAYLIST" tag
            if (xml.name() == "PLAYLISTS") {
                break;
            }
        }
    }
    return rootItem;
}

void TraktorFeature::parsePlaylistEntries(
    QXmlStreamReader &xml,
    QString playlist_path,
    QSqlQuery query_insert_into_playlist,
    QSqlQuery query_insert_into_playlisttracks) {
    // In the database, the name of a playlist is specified by the unique path,
    // e.g., /someFolderA/someFolderB/playlistA"
    query_insert_into_playlist.bindValue(":name", playlist_path);

    if (!query_insert_into_playlist.exec()) {
        LOG_FAILED_QUERY(query_insert_into_playlist)
                << "Failed to insert playlist in TraktorTableModel:"
                << playlist_path;
        return;
    }

    // Get playlist id
    QSqlQuery id_query(m_database);
    id_query.prepare("select id from traktor_playlists where name=:path");
    id_query.bindValue(":path", playlist_path);

    if (!id_query.exec()) {
        LOG_FAILED_QUERY(id_query) << "Could not get inserted playlist id for Traktor playlist::"
                                   << playlist_path;
        return;
    }

    //playlist_id = id_query.lastInsertId().toInt();
    int playlist_id = -1;
    while (id_query.next()) {
        playlist_id = id_query.value(id_query.record().indexOf ("id")).toInt();
    }

    int playlist_position = 1;
    while (!xml.atEnd() && !m_cancelImport) {
        //read next XML element
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == "PRIMARYKEY") {
                QXmlStreamAttributes attr = xml.attributes();
                QString key = attr.value("KEY").toString();
                QString type = attr.value("TYPE").toString();
                if (type == "TRACK") {
                    key.replace(QString(":"), QString(""));
                    //TODO: IFDEF
                    #if defined(__WINDOWS__)
                    key.insert(1,":");
                    #else
                    key.prepend("/Volumes/");
                    #endif

                    //insert to database
                    int track_id = -1;
                    QSqlQuery finder_query(m_database);
                    finder_query.prepare("select id from traktor_library where location=:path");
                    finder_query.bindValue(":path", key);

                    if (!finder_query.exec()) {
                        LOG_FAILED_QUERY(finder_query) << "Could not get track id:" << key;
                        continue;
                    }

                    if (finder_query.next()) {
                        track_id = finder_query.value(finder_query.record().indexOf ("id")).toInt();
                    }

                    query_insert_into_playlisttracks.bindValue(":playlist_id", playlist_id);
                    query_insert_into_playlisttracks.bindValue(":track_id", track_id);
                    query_insert_into_playlisttracks.bindValue(":position", playlist_position++);
                    if (!query_insert_into_playlisttracks.exec()) {
                        LOG_FAILED_QUERY(query_insert_into_playlisttracks)
                                << "trackid" << track_id << " with path " << key
                                << "playlistname; " << playlist_path <<" with ID " << playlist_id;
                    }
                }
            }
        }
        if (xml.isEndElement()) {
            //We leave the infinte loop, if twe have the closing "PLAYLIST" tag
            if (xml.name() == "PLAYLIST") {
                break;
            }
        }
    }
}

void TraktorFeature::clearTable(QString table_name) {
    QSqlQuery query(m_database);
    query.prepare("delete from "+table_name);

    if (!query.exec())
        qDebug() << "Could not delete remove old entries from table "
                 << table_name << " : " << query.lastError();
    else
        qDebug() << "Traktor table entries of '" << table_name <<"' have been cleared.";
}

QString TraktorFeature::getTraktorMusicDatabase() {
    QString musicFolder = "";

    // As of version 2, Traktor has changed the path of the collection.nml
    // In general, the path is <Home>/Documents/Native Instruments/Traktor 2.x.y/collection.nml
    //  where x and y denote the bug fix release numbers. For example, Traktor 2.0.3 has the
    // following path: <Home>/Documents/Native Instruments/Traktor 2.0.3/collection.nml

    //Let's try to detect the latest Traktor version and its collection.nml
    QString myDocuments = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    QDir ni_directory(myDocuments +"/Native Instruments/");
    ni_directory.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks) ;

    //Iterate over the subfolders
    QFileInfoList list = ni_directory.entryInfoList();
    QMap<int, QString> installed_ts_map;

    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        QString folder_name = fileInfo.fileName();

        if (folder_name == "Traktor") {
            //We found a Traktor 1 installation
            installed_ts_map.insert(1, fileInfo.absoluteFilePath());
            continue;
        }
        if (folder_name.contains("Traktor")) {
            qDebug() << "Found " << folder_name;
            QVariant sVersion = folder_name.right(5).remove(".");
            if (sVersion.canConvert<int>()) {
                installed_ts_map.insert(sVersion.toInt(), fileInfo.absoluteFilePath());
            }
        }
    }
    //If no Traktor installation has been found, return some default string
    if (installed_ts_map.isEmpty()) {
        musicFolder =  QDir::homePath() + "/collection.nml";
    } else { //Select the folder with the highest version as default Traktor folder
        QList<int> versions = installed_ts_map.keys();
        qSort(versions);
        musicFolder = installed_ts_map.value(versions.last()) + "/collection.nml";
    }
    qDebug() << "Traktor Library Location=[" << musicFolder << "]";
    return musicFolder;
}

void TraktorFeature::onTrackCollectionLoaded() {
    TreeItem* root = m_future.result();
    if (root) {
        m_childModel.setRootItem(root);
        // Tell the rhythmbox track source that it should re-build its index.
        m_pTrackCollection->getTrackSource("traktor")->buildIndex();

        //m_pTraktorTableModel->select();
        emit(showTrackModel(m_pTraktorTableModel));
        qDebug() << "Traktor library loaded successfully";
    } else {
        QMessageBox::warning(
            NULL,
            tr("Error Loading Traktor Library"),
            tr("There was an error loading your Traktor library. Some of "
               "your Traktor tracks or playlists may not have loaded."));
    }

    // calls a slot in the sidebarmodel such that 'isLoading' is removed from the feature title.
    m_title = tr("Traktor");
    emit(featureLoadingFinished(this));
    activate();
}
