#include "library/rhythmbox/rhythmboxfeature.h"

#include <QMessageBox>
#include <QStringList>
#include <QUrl>
#include <QtDebug>

#include "library/baseexternalplaylistmodel.h"
#include "library/baseexternaltrackmodel.h"
#include "library/library.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/treeitem.h"
#include "moc_rhythmboxfeature.cpp"

RhythmboxFeature::RhythmboxFeature(Library* pLibrary, UserSettingsPointer pConfig)
        : BaseExternalLibraryFeature(pLibrary, pConfig),
          m_cancelImport(false),
          m_icon(":/images/library/ic_library_rhythmbox.svg") {
    QString tableName = "rhythmbox_library";
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
            << "bpm";
    m_trackSource = QSharedPointer<BaseTrackCache>(
            new BaseTrackCache(m_pTrackCollection,
                    tableName, idColumn, columns, false));
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album"
                  << "location"
                  << "comment"
                  << "title"
                  << "genre";
    m_trackSource->setSearchColumns(searchColumns);

    m_pRhythmboxTrackModel = new BaseExternalTrackModel(
        this, pLibrary->trackCollections(),
        "mixxx.db.model.rhythmbox",
        "rhythmbox_library",
        m_trackSource);
    m_pRhythmboxPlaylistModel = new BaseExternalPlaylistModel(
        this, pLibrary->trackCollections(),
        "mixxx.db.model.rhythmbox_playlist",
        "rhythmbox_playlists",
        "rhythmbox_playlist_tracks",
        m_trackSource);

    m_isActivated =  false;
    m_title = tr("Rhythmbox");

    m_database = QSqlDatabase::cloneDatabase(pLibrary->trackCollections()->internalCollection()->database(),
                                             "RHYTHMBOX_SCANNER");

    //Open the database connection in this thread.
    if (!m_database.open()) {
        qDebug() << "Failed to open database for Rhythmbox scanner."
                 << m_database.lastError();
    }
    connect(&m_track_watcher,
            &QFutureWatcher<TreeItem*>::finished,
            this,
            &RhythmboxFeature::onTrackCollectionLoaded,
            Qt::QueuedConnection);
}

RhythmboxFeature::~RhythmboxFeature() {
    m_database.close();
    // stop import thread, if still running
    m_cancelImport = true;
    m_track_future.waitForFinished();
    delete m_pRhythmboxTrackModel;
    delete m_pRhythmboxPlaylistModel;
}

BaseSqlTableModel* RhythmboxFeature::getPlaylistModelForPlaylist(const QString& playlist) {
    BaseExternalPlaylistModel* pModel = new BaseExternalPlaylistModel(
                                            this, m_pLibrary->trackCollections(),
                                            "mixxx.db.model.rhythmbox_playlist",
                                            "rhythmbox_playlists",
                                            "rhythmbox_playlist_tracks",
                                            m_trackSource);
    pModel->setPlaylist(playlist);
    return pModel;
}

bool RhythmboxFeature::isSupported() {
    return (QFile::exists(QDir::homePath() + "/.gnome2/rhythmbox/rhythmdb.xml") ||
            QFile::exists(QDir::homePath() + "/.local/share/rhythmbox/rhythmdb.xml"));
}

QVariant RhythmboxFeature::title() {
    return m_title;
}

QIcon RhythmboxFeature::getIcon() {
    return m_icon;
}

TreeItemModel* RhythmboxFeature::getChildModel() {
    return &m_childModel;
}

void RhythmboxFeature::activate() {
    qDebug() << "RhythmboxFeature::activate()";

    if (!m_isActivated) {
        m_isActivated =  true;
        m_track_future = QtConcurrent::run(this, &RhythmboxFeature::importMusicCollection);
        m_track_watcher.setFuture(m_track_future);
        m_title = "(loading) Rhythmbox";
        //calls a slot in the sidebar model such that 'Rhythmbox (isLoading)' is displayed.
        emit featureIsLoading(this, true);
    }

    emit showTrackModel(m_pRhythmboxTrackModel);
    emit enableCoverArtDisplay(false);
}

void RhythmboxFeature::activateChild(const QModelIndex& index) {
    //qDebug() << "RhythmboxFeature::activateChild()" << index;
    QString playlist = index.data().toString();
    qDebug() << "Activating " << playlist;
    m_pRhythmboxPlaylistModel->setPlaylist(playlist);
    emit showTrackModel(m_pRhythmboxPlaylistModel);
    emit enableCoverArtDisplay(false);
}

TreeItem* RhythmboxFeature::importMusicCollection() {
    qDebug() << "importMusicCollection Thread Id: " << QThread::currentThread();
     // Try and open the Rhythmbox DB. An API call which tells us where
     // the file is would be nice.
    QFile db(QDir::homePath() + "/.gnome2/rhythmbox/rhythmdb.xml");
    if (!db.exists()) {
        db.setFileName(QDir::homePath() + "/.local/share/rhythmbox/rhythmdb.xml");
        if (!db.exists()) {
            return nullptr;
        }
    }

    if (!db.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return nullptr;
    }

    //Delete all table entries of Traktor feature
    ScopedTransaction transaction(m_database);
    clearTable("rhythmbox_playlist_tracks");
    clearTable("rhythmbox_library");
    clearTable("rhythmbox_playlists");
    transaction.commit();

    transaction.transaction();
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO rhythmbox_library (artist, title, album, year, "
                  "genre, comment, tracknumber, bpm, bitrate,"
                  "duration, location, rating ) "
                  "VALUES (:artist, :title, :album, :year, :genre, :comment, "
                  ":tracknumber, :bpm, :bitrate, :duration, :location, :rating )");


    QXmlStreamReader xml(&db);
    while (!xml.atEnd() && !m_cancelImport) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == "entry") {
            QXmlStreamAttributes attr = xml.attributes();
            //Check if we really parse a track and not album art information
            if (attr.value("type").toString() == "song") {
                importTrack(xml, query);
            }
        }
    }
    transaction.commit();

    if (xml.hasError()) {
        // do error handling
        qDebug() << "Cannot process Rhythmbox music collection";
        qDebug() << "XML ERROR: " << xml.errorString();
        return nullptr;
    }

    db.close();
    if (m_cancelImport) {
        return nullptr;
    }
    return importPlaylists();
}

TreeItem* RhythmboxFeature::importPlaylists() {
    QFile db(QDir::homePath() + "/.gnome2/rhythmbox/playlists.xml");
    if (!db.exists()) {
        db.setFileName(QDir::homePath() + "/.local/share/rhythmbox/playlists.xml");
        if (!db.exists()) {
            return nullptr;
        }
    }
    //Open file
    if (!db.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return nullptr;
    }

    QSqlQuery query_insert_to_playlists(m_database);
    query_insert_to_playlists.prepare("INSERT INTO rhythmbox_playlists (id, name) "
                                      "VALUES (:id, :name)");

    QSqlQuery query_insert_to_playlist_tracks(m_database);
    query_insert_to_playlist_tracks.prepare(
            "INSERT INTO rhythmbox_playlist_tracks (playlist_id, track_id, position) "
            "VALUES (:playlist_id, :track_id, :position)");
    //The tree structure holding the playlists
    std::unique_ptr<TreeItem> rootItem = TreeItem::newRoot(this);

    QXmlStreamReader xml(&db);
    while (!xml.atEnd() && !m_cancelImport) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == "playlist") {
            QXmlStreamAttributes attr = xml.attributes();

            //Only parse non build-in playlists
            if (attr.value("type").toString() == "static") {
                QString playlist_name = attr.value("name").toString();

                //Construct the childmodel
                rootItem->appendChild(playlist_name);

                //Execute SQL statement
                query_insert_to_playlists.bindValue(":name", playlist_name);

                if (!query_insert_to_playlists.exec()) {
                    LOG_FAILED_QUERY(query_insert_to_playlists)
                            << "Couldn't insert playlist:" << playlist_name;
                    continue;
                }

                // get playlist_id
                int playlist_id = query_insert_to_playlists.lastInsertId().toInt();

                //Process playlist entries
                importPlaylist(xml, query_insert_to_playlist_tracks, playlist_id);
            }
        }
    }

    if (xml.hasError()) {
        // do error handling
        qDebug() << "Cannot process Rhythmbox music collection";
        qDebug() << "XML ERROR: " << xml.errorString();
        return nullptr;
    }
    db.close();

    return rootItem.release();
}

void RhythmboxFeature::importTrack(QXmlStreamReader &xml, QSqlQuery &query) {
    QString title;
    QString artist;
    QString album;
    QString year;
    QString genre;
    QUrl locationUrl;

    int bpm = 0;
    int bitrate = 0;

    //duration of a track
    int playtime = 0;
    int rating = 0;
    QString comment;
    QString tracknumber;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == "title") {
                title = xml.readElementText();
                continue;
            }
            if (xml.name() == "artist") {
                artist = xml.readElementText();
                continue;
            }
            if (xml.name() == "genre") {
                genre = xml.readElementText();
                continue;
            }
            if (xml.name() == "album") {
                album = xml.readElementText();
                continue;
            }
            if (xml.name() == "track-number") {
                tracknumber = xml.readElementText();
                continue;
            }
            if (xml.name() == "duration") {
                playtime = xml.readElementText().toInt();;
                continue;
            }
            if (xml.name() == "bitrate") {
                bitrate = xml.readElementText().toInt();
                continue;
            }
            if (xml.name() == "beats-per-minute") {
                bpm = xml.readElementText().toInt();
                continue;
            }
            if (xml.name() == "comment") {
                comment = xml.readElementText();
                continue;
            }
            if (xml.name() == "location") {
                locationUrl = QUrl(xml.readElementText());
                continue;
            }
        }
        //exit the loop if we reach the closing <entry> tag
        if (xml.isEndElement() && xml.name() == "entry") {
            break;
        }
    }

    const auto trackFile = TrackFile::fromUrl(locationUrl);
    QString location = trackFile.location();
    if (location.isEmpty()) {
        // here in case of smb:// location
        // TODO(XXX) QUrl does not support SMB:// locations does Mixxx?
        // use ~/.gvfs location instead
        return;
    }

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
    query.bindValue(":bpm", bpm);
    query.bindValue(":bitrate", bitrate);

    bool success = query.exec();

    if (!success) {
        qDebug() << "SQL Error in rhythmboxfeature.cpp: line" << __LINE__
                 << " " << query.lastError();
        return;
    }
}

// reads all playlist entries and executes a SQL statement
void RhythmboxFeature::importPlaylist(QXmlStreamReader &xml,
                                      QSqlQuery &query_insert_to_playlist_tracks,
                                      int playlist_id) {
    int playlist_position = 1;
    while (!xml.atEnd()) {
        //read next XML element
        xml.readNext();
        if (xml.isStartElement() && xml.name() == "location") {
            const auto trackFile = TrackFile::fromUrl(xml.readElementText());

            //get the ID of the file in the rhythmbox_library table
            int track_id = -1;
            QSqlQuery finder_query(m_database);
            finder_query.prepare("select id from rhythmbox_library where location=:path");
            finder_query.bindValue(":path", trackFile.location());
            bool success = finder_query.exec();

            if (success) {
                const int idColumn = finder_query.record().indexOf("id");
                while (finder_query.next()) {
                    track_id = finder_query.value(idColumn).toInt();
                }
             } else {
                qDebug() << "SQL Error in RhythmboxFeature.cpp: line"
                         << __LINE__ << " " << finder_query.lastError();
            }

            query_insert_to_playlist_tracks.bindValue(":playlist_id", playlist_id);
            query_insert_to_playlist_tracks.bindValue(":track_id", track_id);
            query_insert_to_playlist_tracks.bindValue(":position", playlist_position++);
            success = query_insert_to_playlist_tracks.exec();

            if (!success) {
                qDebug() << "SQL Error in RhythmboxFeature.cpp: line" << __LINE__ << " "
                         << query_insert_to_playlist_tracks.lastError()
                         << "trackid" << track_id
                         << "playlis ID " << playlist_id
                         << "-----------------";
            }
        }
        // Exit the the loop if we reach the closing <playlist> tag
        if (xml.isEndElement() && xml.name() == "playlist") {
            break;
        }
    }
}

void RhythmboxFeature::clearTable(const QString& table_name) {
    qDebug() << "clearTable Thread Id: " << QThread::currentThread();
    QSqlQuery query(m_database);
    query.prepare("delete from "+table_name);
    bool success = query.exec();

    if (!success) {
        qDebug() << "Could not delete remove old entries from table "
                 << table_name << " : " << query.lastError();
    } else {
        qDebug() << "Rhythmbox table entries of '" << table_name
                 << "' have been cleared.";
    }
}

void RhythmboxFeature::onTrackCollectionLoaded() {
    std::unique_ptr<TreeItem> root(m_track_future.result());
    if (root) {
        m_childModel.setRootItem(std::move(root));

        // Tell the rhythmbox track source that it should re-build its index.
        m_trackSource->buildIndex();

        //m_pRhythmboxTrackModel->select();
    } else {
         qDebug() << "Rhythmbox Playlists loaded: false";
    }

    // calls a slot in the sidebarmodel such that 'isLoading' is removed from
    // the feature title.
    m_title = tr("Rhythmbox");
    emit featureLoadingFinished(this);
    activate();
}
