#include "library/baseexternallibraryfeature.h"

#include <QMenu>

#include "library/basesqltablemodel.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "widget/wlibrarysidebar.h"

BaseExternalLibraryFeature::BaseExternalLibraryFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig)
        : LibraryFeature(pLibrary, pConfig),
          m_pTrackCollection(pLibrary->trackCollections()->internalCollection()) {
    m_pAddToAutoDJAction = new QAction(tr("Add to Auto DJ Queue (bottom)"), this);
    connect(m_pAddToAutoDJAction,
            &QAction::triggered,
            this,
            &BaseExternalLibraryFeature::slotAddToAutoDJ);

    m_pAddToAutoDJTopAction = new QAction(tr("Add to Auto DJ Queue (top)"), this);
    connect(m_pAddToAutoDJTopAction,
            &QAction::triggered,
            this,
            &BaseExternalLibraryFeature::slotAddToAutoDJTop);

    m_pAddToAutoDJReplaceAction = new QAction(tr("Add to Auto DJ Queue (replace)"), this);
    connect(m_pAddToAutoDJReplaceAction,
            &QAction::triggered,
            this,
            &BaseExternalLibraryFeature::slotAddToAutoDJReplace);

    m_pImportAsMixxxPlaylistAction = new QAction(tr("Import Playlist"), this);
    connect(m_pImportAsMixxxPlaylistAction,
            &QAction::triggered,
            this,
            &BaseExternalLibraryFeature::slotImportAsMixxxPlaylist);
}

BaseExternalLibraryFeature::~BaseExternalLibraryFeature() {
    delete m_pAddToAutoDJAction;
    delete m_pAddToAutoDJTopAction;
    delete m_pAddToAutoDJReplaceAction;
    delete m_pImportAsMixxxPlaylistAction;
}

void BaseExternalLibraryFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    // store the sidebar widget pointer for later use in onRightClickChild
    m_pSidebarWidget = pSidebarWidget;
}

void BaseExternalLibraryFeature::onRightClick(const QPoint& globalPos) {
    Q_UNUSED(globalPos);
    m_lastRightClickedIndex = QModelIndex();
}

void BaseExternalLibraryFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
    //Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;
    QMenu menu(m_pSidebarWidget);
    menu.addAction(m_pAddToAutoDJAction);
    menu.addAction(m_pAddToAutoDJTopAction);
    menu.addAction(m_pAddToAutoDJReplaceAction);
    menu.addSeparator();
    menu.addAction(m_pImportAsMixxxPlaylistAction);
    menu.exec(globalPos);
}

void BaseExternalLibraryFeature::slotAddToAutoDJ() {
    //qDebug() << "slotAddToAutoDJ() row:" << m_lastRightClickedIndex.data();
    addToAutoDJ(PlaylistDAO::AutoDJSendLoc::BOTTOM);
}

void BaseExternalLibraryFeature::slotAddToAutoDJTop() {
    //qDebug() << "slotAddToAutoDJTop() row:" << m_lastRightClickedIndex.data();
    addToAutoDJ(PlaylistDAO::AutoDJSendLoc::TOP);
}

void BaseExternalLibraryFeature::slotAddToAutoDJReplace() {
    //qDebug() << "slotAddToAutoDJReplace() row:" << m_lastRightClickedIndex.data();
    addToAutoDJ(PlaylistDAO::AutoDJSendLoc::REPLACE);
}

void BaseExternalLibraryFeature::addToAutoDJ(PlaylistDAO::AutoDJSendLoc loc) {
    //qDebug() << "slotAddToAutoDJ() row:" << m_lastRightClickedIndex.data();

    QList<TrackId> trackIds;
    QString playlist;
    appendTrackIdsFromRightClickIndex(&trackIds, &playlist);
    if (trackIds.isEmpty()) {
        return;
    }

    PlaylistDAO &playlistDao = m_pTrackCollection->getPlaylistDAO();
    playlistDao.addTracksToAutoDJQueue(trackIds, loc);
}

void BaseExternalLibraryFeature::slotImportAsMixxxPlaylist() {
    // qDebug() << "slotAddToAutoDJ() row:" << m_lastRightClickedIndex.data();

    QList<TrackId> trackIds;
    QString playlist;
    appendTrackIdsFromRightClickIndex(&trackIds, &playlist);
    if (trackIds.isEmpty()) {
        return;
    }

    PlaylistDAO& playlistDao = m_pTrackCollection->getPlaylistDAO();

    int playlistId = playlistDao.createUniquePlaylist(&playlist);

    if (playlistId != -1) {
        playlistDao.appendTracksToPlaylist(trackIds, playlistId);
    } else {
        // Do not change strings here without also changing strings in
        // src/library/baseplaylistfeature.cpp
        QMessageBox::warning(NULL,
                             tr("Playlist Creation Failed"),
                             tr("An unknown error occurred while creating playlist: ")
                             + playlist);
    }
}

// This is a common function for all external Librarys copied to Mixxx DB
void BaseExternalLibraryFeature::appendTrackIdsFromRightClickIndex(
        QList<TrackId>* trackIds, QString* pPlaylist) {
    if (!m_lastRightClickedIndex.isValid()) {
        return;
    }

    DEBUG_ASSERT(pPlaylist);
    *pPlaylist = m_lastRightClickedIndex.data().toString();
    QScopedPointer<BaseSqlTableModel> pPlaylistModelToAdd(
            getPlaylistModelForPlaylist(*pPlaylist));

    if (!pPlaylistModelToAdd || !pPlaylistModelToAdd->initialized()) {
        qDebug() << "BaseExternalLibraryFeature::appendTrackIdsFromRightClickIndex "
                "could not initialize a playlist model for playlist:" << *pPlaylist;
        return;
    }

    pPlaylistModelToAdd->setSort(pPlaylistModelToAdd->fieldIndex(
            ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION), Qt::AscendingOrder);
    pPlaylistModelToAdd->select();

    // Copy Tracks
    int rows = pPlaylistModelToAdd->rowCount();
    for (int i = 0; i < rows; ++i) {
        QModelIndex index = pPlaylistModelToAdd->index(i,0);
        if (index.isValid()) {
            qDebug() << pPlaylistModelToAdd->getTrackLocation(index);
            TrackPointer track = pPlaylistModelToAdd->getTrack(index);
            if (!track) {
                continue;
            }

            TrackId trackId(track->getId());
            if (!trackId.isValid()) {
                continue;
            }

            trackIds->append(trackId);
        }
    }
}

