#include "library/rhythmbox/rhythmboxfeature.h"

#include <QStringList>
#include <QUrl>
#include <QXmlStreamAttributes>
#include <QXmlStreamReader>
#include <QtConcurrentRun>
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
        : BaseExternalLibraryFeature(pLibrary, pConfig, QStringLiteral("rhythmbox")),
          m_pSidebarModel(make_parented<TreeItemModel>(this)),
          m_cancelImport(false) {
    QString tableName = "rhythmbox_library";
    QString idColumn = "id";
    QStringList columns = {
            "id",
            "artist",
            "title",
            "album",
            "year",
            "genre",
            "tracknumber",
            "location",
            "comment",
            "rating",
            "duration",
            "bitrate",
            "bpm"};
    QStringList searchColumns = {
            "artist",
            "album",
            "location",
            "comment",
            "title",
            "genre"};

    m_trackSource = QSharedPointer<BaseTrackCache>::create(
            m_pTrackCollection,
            tableName,
            std::move(idColumn),
            std::move(columns),
            std::move(searchColumns),
            false);

    m_pRhythmboxTrackModel = new BaseExternalTrackModel(this,
            pLibrary->trackCollectionManager(),
            "mixxx.db.model.rhythmbox",
            "rhythmbox_library",
            m_trackSource);
    m_pRhythmboxPlaylistModel = new BaseExternalPlaylistModel(this,
            pLibrary->trackCollectionManager(),
            "mixxx.db.model.rhythmbox_playlist",
            "rhythmbox_playlists",
            "rhythmbox_playlist_tracks",
            m_trackSource);

    m_isActivated =  false;
    m_title = tr("Rhythmbox");

    m_database =
            QSqlDatabase::cloneDatabase(pLibrary->trackCollectionManager()
                                                ->internalCollection()
                                                ->database(),
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

    // m_pRhythmboxTrackModel->setSearch("", "", "library"); // enable search.
    m_pRhythmboxTrackModel->setSearch("", ""); // enable search.
}

RhythmboxFeature::~RhythmboxFeature() {
    m_database.close();
    // stop import thread, if still running
    m_cancelImport = true;
    m_track_future.waitForFinished();
    delete m_pRhythmboxTrackModel;
    delete m_pRhythmboxPlaylistModel;
}

std::unique_ptr<BaseSqlTableModel>
RhythmboxFeature::createPlaylistModelForPlaylist(const QString& playlist) {
    auto pModel = std::make_unique<BaseExternalPlaylistModel>(this,
            m_pLibrary->trackCollectionManager(),
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

TreeItemModel* RhythmboxFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void RhythmboxFeature::activate() {
    qDebug() << "RhythmboxFeature::activate()";

    if (!m_isActivated) {
        m_isActivated =  true;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_track_future = QtConcurrent::run(&RhythmboxFeature::importMusicCollection, this);
#else
        m_track_future = QtConcurrent::run(this, &RhythmboxFeature::importMusicCollection);
#endif
        m_track_watcher.setFuture(m_track_future);
        m_title = "(loading) Rhythmbox";
        //calls a slot in the sidebar model such that 'Rhythmbox (isLoading)' is displayed.
        emit featureIsLoading(this, true);
    }
    emit saveModelState();
    emit showTrackModel(m_pRhythmboxTrackModel);
    emit enableCoverArtDisplay(false);
}

void RhythmboxFeature::activateChild(const QModelIndex& index) {
    //qDebug() << "RhythmboxFeature::activateChild()" << index;
    QString playlist = index.data().toString();
    qDebug() << "Activating " << playlist;
    emit saveModelState();
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

    mixxx::FileInfo fileInfo(db);
    if (!Sandbox::askForAccess(&fileInfo) ||
            !db.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open Rhythmbox db at" << db.fileName() << db.errorString();
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
        if (xml.isStartElement() && xml.name() == QLatin1String("entry")) {
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
    if (!db.open(QIODevice::ReadOnly)) {
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
        if (xml.isStartElement() && xml.name() == QLatin1String("playlist")) {
            QXmlStreamAttributes attr = xml.attributes();

            //Only parse non built-in playlists
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
            if (xml.name() == QLatin1String("title")) {
                title = xml.readElementText();
                continue;
            }
            if (xml.name() == QLatin1String("artist")) {
                artist = xml.readElementText();
                continue;
            }
            if (xml.name() == QLatin1String("genre")) {
                genre = xml.readElementText();
                continue;
            }
            if (xml.name() == QLatin1String("album")) {
                album = xml.readElementText();
                continue;
            }
            if (xml.name() == QLatin1String("track-number")) {
                tracknumber = xml.readElementText();
                continue;
            }
            if (xml.name() == QLatin1String("duration")) {
                playtime = xml.readElementText().toInt();;
                continue;
            }
            if (xml.name() == QLatin1String("bitrate")) {
                bitrate = xml.readElementText().toInt();
                continue;
            }
            if (xml.name() == QLatin1String("beats-per-minute")) {
                bpm = xml.readElementText().toInt();
                continue;
            }
            if (xml.name() == QLatin1String("comment")) {
                comment = xml.readElementText();
                continue;
            }
            if (xml.name() == QLatin1String("location")) {
                locationUrl = QUrl(xml.readElementText());
                continue;
            }
        }
        //exit the loop if we reach the closing <entry> tag
        if (xml.isEndElement() && xml.name() == QLatin1String("entry")) {
            break;
        }
    }

    const auto fileInfo = mixxx::FileInfo::fromQUrl(locationUrl);
    QString location = fileInfo.location();
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
        if (xml.isStartElement() && xml.name() == QLatin1String("location")) {
            const auto fileInfo = mixxx::FileInfo::fromQUrl(xml.readElementText());

            //get the ID of the file in the rhythmbox_library table
            int track_id = -1;
            QSqlQuery finder_query(m_database);
            finder_query.prepare("select id from rhythmbox_library where location=:path");
            finder_query.bindValue(":path", fileInfo.location());
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
        if (xml.isEndElement() && xml.name() == QLatin1String("playlist")) {
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
        m_pSidebarModel->setRootItem(std::move(root));

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
