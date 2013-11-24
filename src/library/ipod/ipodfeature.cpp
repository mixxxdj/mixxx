#include <QMessageBox>
#include <QtDebug>
#include <QXmlStreamReader>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMenu>
#include <QAction>

#include "library/ipod/ipodfeature.h"
#include "library/ipod/gpoditdb.h"

//#include "library/itunes/itunestrackmodel.h"
#include "library/ipod/ipodplaylistmodel.h"
#include "library/dao/settingsdao.h"

extern "C" {
#include <glib-object.h> // g_type_init
}

const QString IPodFeature::IPOD_MOUNT_KEY = "mixxx.IPodFeature.mount";

IPodFeature::IPodFeature(QObject* parent, TrackCollection* pTrackCollection)
        : BaseExternalLibraryFeature(parent, pTrackCollection),
          m_pTrackCollection(pTrackCollection),
          m_isActivated(false),
          m_cancelImport(false),
          m_itdb(NULL)
{
    m_title = tr("iPod");
    m_pIPodPlaylistModel = new IPodPlaylistModel(this, m_pTrackCollection);

    m_gPodItdb = new GPodItdb();

    connect(&m_future_watcher, SIGNAL(finished()), this, SLOT(onTrackCollectionLoaded()));
}

IPodFeature::~IPodFeature() {
    qDebug() << "~IPodFeature()";
    // stop import thread, if still running
    m_cancelImport = true;
    if (m_future.isRunning()) {
        qDebug() << "m_future still running";
        m_future.waitForFinished();
        qDebug() << "m_future finished";
    }
    if (m_itdb) {
        itdb_free(m_itdb);
    }
    delete m_pIPodPlaylistModel;
    delete m_gPodItdb;
}

// static
bool IPodFeature::isSupported() {
    // The iPod might be mount at any time at any location
    return true; //m_gPodItdb->isSupported();
}


QVariant IPodFeature::title() {
    return m_title;
}

QIcon IPodFeature::getIcon() {
    return QIcon(":/images/library/ic_library_ipod.png");
}

void IPodFeature::activate() {
    activate(false);
}

void IPodFeature::activate(bool forceReload) {
    //qDebug("IPodFeature::activate()");

    if (!m_isActivated || forceReload) {

        SettingsDAO settings(m_pTrackCollection->getDatabase());

        QString dbSetting(settings.getValue(IPOD_MOUNT_KEY));
        // if a path exists in the database, use it
        if (!dbSetting.isEmpty() && QFile::exists(dbSetting)) {
            m_dbfile = dbSetting;
        } else {
            // No Path in settings
            m_dbfile = "";
        }

        m_dbfile = detectMountPoint(m_dbfile);

        if (!QFile::exists(m_dbfile)) {
            m_dbfile = QFileDialog::getExistingDirectory(
                    NULL,
                    tr("Select your iPod mount"));
            if (m_dbfile.isEmpty()) {
                emit(showTrackModel(m_pIPodPlaylistModel));
                return;
            }
        }

        m_isActivated =  true;

        QThreadPool::globalInstance()->setMaxThreadCount(4); // Tobias decided to use 4
        // Let a worker thread do the iPod parsing
        m_future = QtConcurrent::run(this, &IPodFeature::importLibrary);
        m_future_watcher.setFuture(m_future);
        m_title = tr("(loading) iPod"); // (loading) at start in respect to small displays
        //calls a slot in the sidebar model such that '(loading)iPod' is displayed.
        emit (featureIsLoading(this));
    } else {
        m_pIPodPlaylistModel->setPlaylist(itdb_playlist_mpl(m_itdb)); // Gets the master playlist
    }
    emit(showTrackModel(m_pIPodPlaylistModel));
}

QString IPodFeature::detectMountPoint( QString iPodMountPoint) {
    QFileInfoList mountpoints;
    #ifdef __WINDOWS__
      // Windows iPod Detection
      mountpoints = QDir::drives();
    #elif __LINUX__
      // Linux
      mountpoints = QDir("/media").entryInfoList();
      mountpoints += QDir("/mnt").entryInfoList();
    #elif __APPLE__
      // Mac OSX
      mountpoints = QDir("/Volumes").entryInfoList();
    #endif

    QListIterator<QFileInfo> i(mountpoints);
    QFileInfo mp;
    while (i.hasNext()) {
        mp = (QFileInfo) i.next();
        qDebug() << "mp:" << mp.filePath();
        if( mp.filePath() != iPodMountPoint ) {
            if (QDir( QString(mp.filePath() + "/iPod_Control") ).exists() ) {
                qDebug() << "iPod found at" << mp.filePath();

                // Multiple iPods
                if (!iPodMountPoint.isEmpty()) {
                    int ret = QMessageBox::warning(NULL, tr("Multiple iPods Detected"),
                           tr("Mixxx has detected another iPod. \n\n")+
                           tr("Choose Yes to use the newly found iPod @ ")+ mp.filePath()+
                           tr(" or to continue to search for other iPods. \n")+
                           tr("Choose No to use the existing iPod @ ")+ iPodMountPoint+
                           tr( " and end detection. \n"),
                           QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                    if (ret == QMessageBox::No) {
                    break;
                    }
                }
                iPodMountPoint = mp.filePath();
            }
        }
    }
    return iPodMountPoint;
}

void IPodFeature::activateChild(const QModelIndex& index) {
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    qDebug() << "IPodFeature::activateChild " << item->data() << " " << item->dataPath();
    QString playlist = item->dataPath().toString();
    Itdb_Playlist* pPlaylist = (Itdb_Playlist*)playlist.toUInt();

    if (pPlaylist) {
        qDebug() << "Activating " << QString::fromUtf8(pPlaylist->name);
    }
    m_pIPodPlaylistModel->setPlaylist(pPlaylist);
    emit(showTrackModel(m_pIPodPlaylistModel));
}

TreeItemModel* IPodFeature::getChildModel() {
    return &m_childModel;
}


// This method is executed in a separate thread via QtConcurrent::run
TreeItem* IPodFeature::importLibrary() {
    // Give thread a low priority
    QThread* thisThread = QThread::currentThread();
    thisThread->setPriority(QThread::LowestPriority);

    qDebug() << "IPodFeature::importLibrary() ";

    TreeItem* playlist_root = new TreeItem();

    GError* err = 0;
    qDebug() << "Calling the libgpod db parser for:" << m_dbfile;
    if (m_itdb) {
        itdb_free(m_itdb);
        m_itdb = NULL;
    }
    m_itdb = itdb_parse(QDir::toNativeSeparators(m_dbfile).toLocal8Bit(), &err);

    if (err) {
        qDebug() << "There was an error, attempting to free db: "
                 << err->message;
        QMessageBox::warning(NULL, tr("Error Loading iPod database"),
                err->message);
        g_error_free(err);
        if (m_itdb) {
            itdb_free(m_itdb);
            m_itdb = 0;
        }
    } else if (m_itdb) {
        GList* playlist_node;

        for (playlist_node = g_list_first(m_itdb->playlists);
             playlist_node != NULL;
             playlist_node = g_list_next(playlist_node))
        {
            Itdb_Playlist* pPlaylist;
            pPlaylist = (Itdb_Playlist*)playlist_node->data;
            if (!itdb_playlist_is_mpl(pPlaylist)) {
                QString playlistname = QString::fromUtf8(pPlaylist->name);
                qDebug() << playlistname;
                // append the playlist to the child model
                TreeItem *item = new TreeItem(playlistname, QString::number((uint)pPlaylist), this, playlist_root);
                playlist_root->appendChild(item);
            }
        }
    }
    return playlist_root;
}

void IPodFeature::onTrackCollectionLoaded(){
    TreeItem* root = m_future.result();
    if(root){
        m_childModel.setRootItem(root);
        if (m_isActivated) {
            activate();
        }
        qDebug() << "iPod library loaded: success";
        SettingsDAO settings(m_pTrackCollection->getDatabase());
        settings.setValue(IPOD_MOUNT_KEY, m_dbfile);
    }
    else{
        QMessageBox::warning(
            NULL,
            tr("Error Loading iPod database"),
            tr("There was an error loading your iPod database. Some of "
               "your iPod tracks or playlists may not have loaded."));
    }
    //calls a slot in the sidebarmodel such that 'isLoading' is removed from the feature title.
    m_title = tr("iPod");
    emit(featureLoadingFinished(this));
    activate();
}




void IPodFeature::appendTrackIdsFromRightClickIndex(QList<int>* trackIds, QString* pPlaylistName) {
    // qDebug() << "slotAddToAutoDJ() row:" << m_lastRightClickedIndex.data();

    if (m_lastRightClickedIndex.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(m_lastRightClickedIndex.internalPointer());
        qDebug() << "IPodFeature::slotImportAsMixxxPlaylist " << item->data() << " " << item->dataPath();
        QString pPlaylistAsSting = item->dataPath().toString();
        Itdb_Playlist* pPlaylist = (Itdb_Playlist*)pPlaylistAsSting.toUInt();
        if (pPlaylist) {
            *pPlaylistName = QString::fromUtf8(pPlaylist->name);
            IPodPlaylistModel* pPlaylistModelToAdd = new IPodPlaylistModel(this, m_pTrackCollection);
            pPlaylistModelToAdd->setPlaylist(pPlaylist);

            // Copy Tracks
            int rows = pPlaylistModelToAdd->rowCount();
            for (int i = 0; i < rows; ++i) {
                QModelIndex index = pPlaylistModelToAdd->index(i,0);
                if (index.isValid()) {
                    //qDebug() << pPlaylistModelToAdd->getTrackLocation(index);
                    TrackPointer track = pPlaylistModelToAdd->getTrack(index);
                    trackIds->append(track->getId());
                }
            }
            delete pPlaylistModelToAdd;
        }
    }
}


