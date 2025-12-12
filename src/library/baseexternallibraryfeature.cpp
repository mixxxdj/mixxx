#include "library/baseexternallibraryfeature.h"

#include <QMenu>

#include "library/basesqltablemodel.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/crate/crate.h"
#include "library/treeitem.h"
#include "moc_baseexternallibraryfeature.cpp"
#include "util/logger.h"
#include "widget/wlibrarysidebar.h"

namespace {

const mixxx::Logger kLogger("BaseExternalLibraryFeature");

} // namespace

BaseExternalLibraryFeature::BaseExternalLibraryFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig,
        const QString& iconName)
        : LibraryFeature(pLibrary, pConfig, iconName),
          m_pTrackCollection(pLibrary->trackCollectionManager()->internalCollection()) {
    m_pAddToAutoDJAction = make_parented<QAction>(tr("Add to Auto DJ Queue (bottom)"), this);
    connect(m_pAddToAutoDJAction,
            &QAction::triggered,
            this,
            &BaseExternalLibraryFeature::slotAddToAutoDJ);

    m_pAddToAutoDJTopAction = make_parented<QAction>(tr("Add to Auto DJ Queue (top)"), this);
    connect(m_pAddToAutoDJTopAction,
            &QAction::triggered,
            this,
            &BaseExternalLibraryFeature::slotAddToAutoDJTop);

    m_pAddToAutoDJReplaceAction = make_parented<QAction>(tr("Add to Auto DJ Queue (replace)"), this);
    connect(m_pAddToAutoDJReplaceAction,
            &QAction::triggered,
            this,
            &BaseExternalLibraryFeature::slotAddToAutoDJReplace);

    m_pImportAsMixxxPlaylistAction = make_parented<QAction>(tr("Import as Playlist"), this);
    connect(m_pImportAsMixxxPlaylistAction,
            &QAction::triggered,
            this,
            &BaseExternalLibraryFeature::slotImportAsMixxxPlaylist);

    m_pImportAsMixxxCrateAction = make_parented<QAction>(tr("Import as Crate"), this);
    connect(m_pImportAsMixxxCrateAction,
            &QAction::triggered,
            this,
            &BaseExternalLibraryFeature::slotImportAsMixxxCrate);
}

void BaseExternalLibraryFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    // store the sidebar widget pointer for later use in onRightClickChild
    m_pSidebarWidget = pSidebarWidget;
}

void BaseExternalLibraryFeature::onRightClick(const QPoint& globalPos) {
    Q_UNUSED(globalPos);
    m_lastRightClickedIndex = QModelIndex();
}

void BaseExternalLibraryFeature::onRightClickChild(
        const QPoint& globalPos, const QModelIndex& index) {
    // Save the model index so we can get it in the action slots...
    // Make sure that this is reset when the related TreeItem is deleted.
    m_lastRightClickedIndex = index;
    QMenu menu(m_pSidebarWidget);
    menu.addAction(m_pAddToAutoDJAction);
    menu.addAction(m_pAddToAutoDJTopAction);
    menu.addAction(m_pAddToAutoDJReplaceAction);
    menu.addSeparator();
    menu.addAction(m_pImportAsMixxxPlaylistAction);
    menu.addAction(m_pImportAsMixxxCrateAction);
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
    // qDebug() << "slotImportAsMixxxPlaylist() row:" << m_lastRightClickedIndex.data();

    QList<TrackId> trackIds;
    QString playlist;
    appendTrackIdsFromRightClickIndex(&trackIds, &playlist);
    if (trackIds.isEmpty()) {
        return;
    }

    PlaylistDAO& playlistDao = m_pTrackCollection->getPlaylistDAO();

    int playlistId = playlistDao.createUniquePlaylist(&playlist);

    if (playlistId != kInvalidPlaylistId) {
        playlistDao.appendTracksToPlaylist(trackIds, playlistId);
    } else {
        // Do not change strings here without also changing strings in
        // src/library/trackset/baseplaylistfeature.cpp
        QMessageBox::warning(nullptr,
                tr("Playlist Creation Failed"),
                tr("An unknown error occurred while creating playlist: ") + playlist);
    }
}

void BaseExternalLibraryFeature::slotImportAsMixxxCrate() {
    // qDebug() << "slotImportAsMixxxCrate() row:" << m_lastRightClickedIndex.data();

    QList<TrackId> trackIds;
    QString playlist;
    appendTrackIdsFromRightClickIndex(&trackIds, &playlist);
    if (trackIds.isEmpty()) {
        return;
    }

    Crate crate;
    crate.setName(playlist);

    CrateId crateId;

    if (m_pTrackCollection->insertCrate(crate, &crateId)) {
        m_pTrackCollection->addCrateTracks(crateId, trackIds);
    } else {
        QMessageBox::warning(nullptr,
                tr("Crate Creation Failed"),
                tr("Could not create crate, it most likely already exists: ") + playlist);
    }
}

// This is a common function for all external libraries copied to Mixxx DB
void BaseExternalLibraryFeature::appendTrackIdsFromRightClickIndex(
        QList<TrackId>* trackIds, QString* pPlaylist) {
    if (!m_lastRightClickedIndex.isValid()) {
        return;
    }

    const auto* pTreeItem = static_cast<TreeItem*>(
            m_lastRightClickedIndex.internalPointer());
    VERIFY_OR_DEBUG_ASSERT(pTreeItem) {
        return;
    }

    DEBUG_ASSERT(pPlaylist);
    *pPlaylist = pTreeItem->getLabel();
    const std::unique_ptr<BaseSqlTableModel> pPlaylistModelToAdd =
            createPlaylistModelForPlaylist(pTreeItem->getData());

    if (!pPlaylistModelToAdd || !pPlaylistModelToAdd->initialized()) {
        qDebug() << "BaseExternalLibraryFeature::"
                    "appendTrackIdsFromRightClickIndex "
                    "could not initialize a playlist model for "
                    "playlist:"
                 << *pPlaylist;
        return;
    }

    pPlaylistModelToAdd->setSort(
            pPlaylistModelToAdd->fieldIndex(
                    ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION),
            Qt::AscendingOrder);
    pPlaylistModelToAdd->select();

    // Copy Tracks
    const int rows = pPlaylistModelToAdd->rowCount();
    for (int i = 0; i < rows; ++i) {
        QModelIndex index = pPlaylistModelToAdd->index(i, 0);
        VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
            continue;
        }
        const TrackId trackId = pPlaylistModelToAdd->getTrackId(index);
        if (!trackId.isValid()) {
            kLogger.warning()
                    << "Failed to add track"
                    << pPlaylistModelToAdd->getTrackUrl(index)
                    << "to playlist"
                    << *pPlaylist;
            continue;
        }
        if (kLogger.traceEnabled()) {
            kLogger.trace()
                    << "Adding track"
                    << pPlaylistModelToAdd->getTrackUrl(index)
                    << "to playlist"
                    << *pPlaylist;
        }
        trackIds->append(trackId);
    }
}

std::unique_ptr<BaseSqlTableModel>
BaseExternalLibraryFeature::createPlaylistModelForPlaylist(
        const QVariant&) {
    return {};
}
