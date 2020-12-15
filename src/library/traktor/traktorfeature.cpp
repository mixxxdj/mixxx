#include "library/traktor/traktorfeature.h"

#include <QMap>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QXmlStreamReader>
#include <QtDebug>

#include "library/library.h"
#include "library/librarytablemodel.h"
#include "library/missingtablemodel.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/treeitem.h"
#include "moc_traktorfeature.cpp"
#include "util/sandbox.h"

namespace {

QString fromTraktorSeparators(QString path) {
    // Traktor uses /: instead of just / as delimiting character for some reasons
    return path.replace("/:", "/");
}


} // anonymous namespace


TraktorTrackModel::TraktorTrackModel(QObject* parent,
                                     TrackCollectionManager* pTrackCollectionManager,
                                     QSharedPointer<BaseTrackCache> trackSource)
        : BaseExternalTrackModel(parent, pTrackCollectionManager,
                                 "mixxx.db.model.traktor_tablemodel",
                                 "traktor_library",
                                 trackSource) {
}

bool TraktorTrackModel::isColumnHiddenByDefault(int column) {
    if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE)) {
        return true;
    }
    return BaseSqlTableModel::isColumnHiddenByDefault(column);
}

TraktorPlaylistModel::TraktorPlaylistModel(QObject* parent,
                                           TrackCollectionManager* pTrackCollectionManager,
                                           QSharedPointer<BaseTrackCache> trackSource)
        : BaseExternalPlaylistModel(parent, pTrackCollectionManager,
                                    "mixxx.db.model.traktor.playlistmodel",
                                    "traktor_playlists",
                                    "traktor_playlist_tracks",
                                    trackSource) {
}

bool TraktorPlaylistModel::isColumnHiddenByDefault(int column) {
    if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE)) {
        return true;
    }
    return BaseSqlTableModel::isColumnHiddenByDefault(column);
}

TraktorFeature::TraktorFeature(Library* pLibrary, UserSettingsPointer pConfig)
        : BaseExternalLibraryFeature(pLibrary, pConfig),
          m_cancelImport(false),
          m_icon(":/images/library/ic_library_traktor.svg") {
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
    m_trackSource = QSharedPointer<BaseTrackCache>(
            new BaseTrackCache(pLibrary->trackCollections()->internalCollection(), tableName, idColumn,
                           columns, false));
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album"
                  << "location"
                  << "comment"
                  << "title"
                  << "genre";
    m_trackSource->setSearchColumns(searchColumns);

    m_isActivated = false;
    m_pTraktorTableModel = new TraktorTrackModel(this, pLibrary->trackCollections(), m_trackSource);
    m_pTraktorPlaylistModel = new TraktorPlaylistModel(this, pLibrary->trackCollections(), m_trackSource);

    m_title = tr("Traktor");

    m_database = QSqlDatabase::cloneDatabase(pLibrary->trackCollections()->internalCollection()->database(),
                                             "TRAKTOR_SCANNER");

    //Open the database connection in this thread.
    if (!m_database.open()) {
        qDebug() << "Failed to open database for iTunes scanner."
                 << m_database.lastError();
    }
    connect(&m_future_watcher,
            &QFutureWatcher<TreeItem*>::finished,
            this,
            &TraktorFeature::onTrackCollectionLoaded);
}

TraktorFeature::~TraktorFeature() {
    m_database.close();
    m_cancelImport = true;
    m_future.waitForFinished();
    delete m_pTraktorTableModel;
    delete m_pTraktorPlaylistModel;
}

BaseSqlTableModel* TraktorFeature::getPlaylistModelForPlaylist(const QString& playlist) {
    TraktorPlaylistModel* pModel = new TraktorPlaylistModel(this, m_pLibrary->trackCollections(), m_trackSource);
    pModel->setPlaylist(playlist);
    return pModel;
}

QVariant TraktorFeature::title() {
    return m_title;
}

QIcon TraktorFeature::getIcon() {
    return m_icon;
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
        // Let a worker thread do the XML parsing
        m_future = QtConcurrent::run(this, &TraktorFeature::importLibrary,
                                     getTraktorMusicDatabase());
        m_future_watcher.setFuture(m_future);
        m_title = tr("(loading) Traktor");
        //calls a slot in the sidebar model such that 'iTunes (isLoading)' is displayed.
        emit featureIsLoading(this, true);
    }

    emit showTrackModel(m_pTraktorTableModel);
    emit enableCoverArtDisplay(false);
}

void TraktorFeature::activateChild(const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }

    //access underlying TreeItem object
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    if (!item->hasChildren()) {
        qDebug() << "Activate Traktor Playlist: " << item->getData().toString();
        m_pTraktorPlaylistModel->setPlaylist(item->getData().toString());
        emit showTrackModel(m_pTraktorPlaylistModel);
        emit enableCoverArtDisplay(false);
    }
}

TreeItem* TraktorFeature::importLibrary(const QString& file) {
    //Give thread a low priority
    QThread* thisThread = QThread::currentThread();
    thisThread->setPriority(QThread::LowPriority);
    //Invisible root item of Traktor's child model
    TreeItem* root = nullptr;
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
        return nullptr;
    }
    QXmlStreamReader xml(&traktor_file);
    bool inCollectionTag = false;
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
            if (inCollectionTag && xml.name() == "ENTRY") {
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
            if (xml.name() == "PLAYLISTS" && inPlaylistsTag) {
                inPlaylistsTag = false;
            }
        }
    }
    if (xml.hasError()) {
         // do error handling
         qDebug() << "Cannot process Traktor music collection";
         if (root) {
             delete root;
         }
         return nullptr;
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
                // On OS X, the volume is supposed to be "Macintosh HD" or "Macintosh SSD",
                // which is a folder in /Volumes/ symlinked to root folder /
                #if defined(__APPLE__)
                location = "/Volumes/" + volume;
                #else
                location = volume;
                #endif
                location += fromTraktorSeparators(path);
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
                bool ok = false;
                int parsed_rating = ranking_str.toInt(&ok) / 51;
                if (ok) {
                    rating = parsed_rating;
                }
                continue;
            }
            if (xml.name() == "TEMPO") {
                QXmlStreamAttributes attr = xml.attributes ();
                bpm = attr.value("BPM").toString().toFloat();
                continue;
            }
        }
        //We leave the infinite loop, if twe have the closing tag "ENTRY"
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

    std::unique_ptr<TreeItem> rootItem = TreeItem::newRoot(this);
    TreeItem* parent = rootItem.get();

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
                    //qDebug() << "Folder: " +current_path << " has parent " << parent->getData().toString();
                    map.insert(current_path, "FOLDER");
                    parent = parent->appendChild(name, current_path);
               } else if (type == "PLAYLIST") {
                    current_path += delimiter;
                    current_path += name;
                    //qDebug() << "Playlist: " +current_path << " has parent " << parent->getData().toString();
                    map.insert(current_path, "PLAYLIST");

                    parent->appendChild(name, current_path);
                    // process all the entries within the playlist 'name' having path 'current_path'
                    parsePlaylistEntries(xml, current_path,
                                         query_insert_to_playlists,
                                         query_insert_to_playlist_tracks);
                }
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
            //We leave the infinite loop, if twe have the closing "PLAYLIST" tag
            if (xml.name() == "PLAYLISTS") {
                break;
            }
        }
    }
    return rootItem.release();
}

void TraktorFeature::parsePlaylistEntries(
        QXmlStreamReader& xml,
        const QString& playlist_path,
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
    const int idColumn = id_query.record().indexOf("id");
    while (id_query.next()) {
        playlist_id = id_query.value(idColumn).toInt();
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
                    key = fromTraktorSeparators(key);
                    #if defined(__APPLE__)
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
                        track_id = finder_query.value(finder_query.record().indexOf("id")).toInt();
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
            //We leave the infinite loop, if twe have the closing "PLAYLIST" tag
            if (xml.name() == "PLAYLIST") {
                break;
            }
        }
    }
}

void TraktorFeature::clearTable(const QString& table_name) {
    QSqlQuery query(m_database);
    query.prepare("delete from "+table_name);

    if (!query.exec()) {
        qDebug() << "Could not delete remove old entries from table "
                 << table_name << " : " << query.lastError();
    } else {
        qDebug() << "Traktor table entries of '" << table_name << "' have been cleared.";
    }
}

QString TraktorFeature::getTraktorMusicDatabase() {
    QString musicFolder = "";

    // As of version 2, Traktor has changed the path of the collection.nml
    // In general, the path is <Home>/Documents/Native Instruments/Traktor 2.x.y/collection.nml
    //  where x and y denote the bug fix release numbers. For example, Traktor 2.0.3 has the
    // following path: <Home>/Documents/Native Instruments/Traktor 2.0.3/collection.nml

    //Let's try to detect the latest Traktor version and its collection.nml
    QString myDocuments = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QDir ni_directory(myDocuments +"/Native Instruments/");
    ni_directory.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    // We may not have access to this directory since it is in the user's
    // Documents folder. Ask for access if we don't have it.
    if (ni_directory.exists()) {
        Sandbox::askForAccess(ni_directory.canonicalPath());
    }

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
            if (sVersion.canConvert(QMetaType::Int)) {
                installed_ts_map.insert(sVersion.toInt(), fileInfo.absoluteFilePath());
            }
        }
    }
    //If no Traktor installation has been found, return some default string
    if (installed_ts_map.isEmpty()) {
        musicFolder =  QDir::homePath() + "/collection.nml";
    } else { //Select the folder with the highest version as default Traktor folder
        QList<int> versions = installed_ts_map.keys();
        std::sort(versions.begin(), versions.end());
        musicFolder = installed_ts_map.value(versions.last()) + "/collection.nml";
    }
    qDebug() << "Traktor Library Location=[" << musicFolder << "]";
    return musicFolder;
}

void TraktorFeature::onTrackCollectionLoaded() {
    std::unique_ptr<TreeItem> root(m_future.result());
    if (root) {
        m_childModel.setRootItem(std::move(root));
        // Tell the traktor track source that it should re-build its index.
        m_trackSource->buildIndex();

        //m_pTraktorTableModel->select();
        emit showTrackModel(m_pTraktorTableModel);
        qDebug() << "Traktor library loaded successfully";
    } else {
        QMessageBox::warning(
                nullptr,
                tr("Error Loading Traktor Library"),
                tr("There was an error loading your Traktor library. Some of "
                   "your Traktor tracks or playlists may not have loaded."));
    }

    // calls a slot in the sidebarmodel such that 'isLoading' is removed from the feature title.
    m_title = tr("Traktor");
    emit featureLoadingFinished(this);
    activate();
}
