#include "library/clementine/clementinefeature.h"

#include <QList>
#include <QMessageBox>
#include <QtDebug>

#include "library/baseexternalplaylistmodel.h"
#include "library/clementine/clementinedbconnection.h"
#include "library/clementine/clementineplaylistmodel.h"
#include "library/dao/settingsdao.h"
#include "library/library.h"
#include "track/track.h"

ClementineFeature::ClementineFeature(
        Library* pLibrary, UserSettingsPointer pConfig)
        : BaseExternalLibraryFeature(pLibrary, pConfig, QStringLiteral("clementine")),
          m_connection(std::make_shared<ClementineDbConnection>()),
          m_isActivated(false),
          m_pClementinePlaylistModel(make_parented<ClementinePlaylistModel>(
                  this, m_pLibrary->trackCollectionManager(), m_connection)),
          m_pSidebarModel(make_parented<TreeItemModel>(this)),
          m_playlists(),
          m_future(),
          m_title(tr("Clementine")) {
}

ClementineFeature::~ClementineFeature() {
    qDebug() << "~ClementineFeature()";
    if (m_future.isRunning()) {
        qDebug() << "m_future still running";
        m_future.waitForFinished();
        qDebug() << "m_future finished";
    }
}

// static
bool ClementineFeature::isSupported() {
    return ClementineDbConnection::getDatabaseFile().exists();
}

QVariant ClementineFeature::title() {
    return m_title;
}

void ClementineFeature::activate() {
    qDebug() << "ClementineFeature::activate()";

    m_title = tr("(loading) Clementine");
    //calls a slot in the sidebar model such that 'Clementine (isLoading)' is displayed.
    emit featureIsLoading(this, true);

    if (!m_isActivated) {
        QFileInfo databaseFile = ClementineDbConnection::getDatabaseFile();

        if (!databaseFile.exists()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Error loading Clementine database"),
                    tr("Clementine database file not found at\n") +
                            databaseFile.filePath());
            qInfo() << "Clementine database file" << databaseFile << "does not exist";
            return;
        }

        if (!m_connection->open(databaseFile)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Error loading Clementine database"),
                    tr("There was an error loading your Clementine database at\n") +
                            databaseFile.filePath());
            return;
        }
    }
    m_isActivated = true;

    std::unique_ptr<TreeItem> pRootItem = TreeItem::newRoot(this);
    QList<ClementinePlaylist> playlists = m_connection->getPlaylists();
    for (const ClementinePlaylist& playlist : playlists) {
        // append the playlist to the child model
        pRootItem->appendChild(playlist.name, playlist.playlistId);
    }
    m_pSidebarModel->setRootItem(std::move(pRootItem));

    m_pClementinePlaylistModel->setTableModel(0); // Gets the master playlist
    m_pClementinePlaylistModel->select();

    //calls a slot in the sidebarmodel such that 'isLoading' is removed from the feature title.
    m_title = tr("Clementine");
    emit featureLoadingFinished(this);
    qDebug() << "Clementine library loaded: success";

    emit showTrackModel(m_pClementinePlaylistModel);
    emit enableCoverArtDisplay(false);
}

void ClementineFeature::activateChild(const QModelIndex& index) {
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    int playlistID = item->getData().toInt();
    VERIFY_OR_DEBUG_ASSERT(playlistID >= 0) {
        return;
    }
    qDebug() << "Activating " << item->getLabel();
    m_pClementinePlaylistModel->setTableModel(playlistID);
    m_pClementinePlaylistModel->select();
    emit showTrackModel(m_pClementinePlaylistModel);
    emit enableCoverArtDisplay(false);
}

TreeItemModel* ClementineFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void ClementineFeature::appendTrackIdsFromRightClickIndex(
        QList<TrackId>* trackIds, QString* pPlaylist) {
    VERIFY_OR_DEBUG_ASSERT(lastRightClickedIndex().isValid()) {
        return;
    }

    TreeItem* item = static_cast<TreeItem*>(lastRightClickedIndex().internalPointer());
    *pPlaylist = item->getLabel();

    int playlistID = item->getData().toInt();
    VERIFY_OR_DEBUG_ASSERT(playlistID >= 0) {
        return;
    }

    m_pClementinePlaylistModel->setTableModel(playlistID);

    // Copy Tracks
    int rows = m_pClementinePlaylistModel->rowCount();
    for (int i = 0; i < rows; ++i) {
        QModelIndex index = m_pClementinePlaylistModel->index(i, 0);
        if (!index.isValid()) {
            continue;
        }
        //Check if track file really exists, otherwise a segfault is happening
        QString trackLocation = m_pClementinePlaylistModel->getTrackLocation(index);
        bool fileExists = QFile::exists(trackLocation);
        if (!fileExists) {
            continue;
        }
        TrackPointer track = m_pClementinePlaylistModel->getTrack(index);
        trackIds->append(track->getId());
    }
}
