#include "library/trackset/preparation/preparationfeature.h"

#include <QDateTime>
#include <QMenu>
#include <QSqlTableModel>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "library/library.h"
#include "library/library_prefs.h"
#include "library/playlisttablemodel.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/treeitem.h"
#include "mixer/playerinfo.h"
#include "moc_preparationfeature.cpp"
#include "track/track.h"
#include "util/make_const_iterator.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wtracktableview.h"

namespace {
constexpr int kNumToplevelHistoryEntries = 5;
} // namespace

using namespace mixxx::library::prefs;

PreparationFeature::PreparationFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig)
        : BasePlaylistFeature(
                  pLibrary,
                  pConfig,
                  new PlaylistTableModel(
                          nullptr,
                          pLibrary->trackCollectionManager(),
                          "mixxx.db.model.preparation",
                          /*keep hidden tracks*/ true),
                  QStringLiteral("PREPARATIONHOME"),
                  QStringLiteral("history"),
                  QStringLiteral("SetlogCountsDurations"),
                  /*keep hidden tracks*/ true),
          m_currentPlaylistId(kInvalidPlaylistId),
          m_yearNodeId(kInvalidPlaylistId),
          m_pLibrary(pLibrary),
          m_pConfig(pConfig) {
    // remove unneeded entries
    deleteAllUnlockedPlaylistsWithFewerTracks();

    QString placeholderName = "preparationPlaceholder";
    // remove previously created placeholder playlists
    const QList<QPair<int, QString>> pls = m_playlistDao.getPlaylists(PlaylistDAO::PLHT_UNKNOWN);
    QStringList plsToDelete;
    for (const QPair<int, QString>& pl : pls) {
        if (pl.second.startsWith(placeholderName)) {
            plsToDelete.append(QString::number(pl.first));
        }
    }
    m_playlistDao.deletePlaylists(plsToDelete);

    // Create empty placeholder playlist for YEAR items
    m_yearNodeId = m_playlistDao.createUniquePlaylist(&placeholderName,
            PlaylistDAO::PLHT_UNKNOWN);
    DEBUG_ASSERT(m_yearNodeId != kInvalidPlaylistId);
    // just to be safe
    m_playlistDao.setPlaylistLocked(m_yearNodeId, true);

    // construct child model
    m_pSidebarModel->setRootItem(TreeItem::newRoot(this));
    constructChildModel(kInvalidPlaylistId);
    m_pShowTrackModelInLibraryWindowAction =
            make_parented<QAction>(tr("Show in Library Window"), this);
    connect(m_pShowTrackModelInLibraryWindowAction,
            &QAction::triggered,
            this,
            &PreparationFeature::slotShowInLibraryWindow);
    m_pJoinWithPreviousAction = new QAction(tr("Join with previous (below)"), this);
    connect(m_pJoinWithPreviousAction,
            &QAction::triggered,
            this,
            &PreparationFeature::slotJoinWithPrevious);

    m_pMarkTracksPlayedAction = new QAction(tr("Mark all tracks played"), this);
    connect(m_pMarkTracksPlayedAction,
            &QAction::triggered,
            this,
            &PreparationFeature::slotMarkAllTracksPlayed);

    m_pStartNewPlaylist = new QAction(tr("Finish current and start new"), this);
    connect(m_pStartNewPlaylist,
            &QAction::triggered,
            this,
            &PreparationFeature::slotGetNewPlaylist);

    m_pLockAllChildPlaylists = new QAction(tr("Lock all child playlists"), this);
    connect(m_pLockAllChildPlaylists,
            &QAction::triggered,
            this,
            &PreparationFeature::slotLockAllChildPlaylists);

    m_pUnlockAllChildPlaylists = new QAction(tr("Unlock all child playlists"), this);
    connect(m_pUnlockAllChildPlaylists,
            &QAction::triggered,
            this,
            &PreparationFeature::slotUnlockAllChildPlaylists);

    m_pDeleteAllChildPlaylists = new QAction(tr("Delete all unlocked child playlists"), this);
    connect(m_pDeleteAllChildPlaylists,
            &QAction::triggered,
            this,
            &PreparationFeature::slotDeleteAllUnlockedChildPlaylists);

    // initialized in a new generic slot(get new preparation playlist purpose)
    slotGetNewPlaylist();
}

PreparationFeature::~PreparationFeature() {
    // Clean up preparation when shutting down in case the track threshold changed,
    // incl. potentially empty current playlist
    deleteAllUnlockedPlaylistsWithFewerTracks();
    // Delete the placeholder
    m_playlistDao.deletePlaylist(m_yearNodeId);
}

QVariant PreparationFeature::title() {
    return tr("Preparation");
}

void PreparationFeature::bindLibraryWidget(
        WLibrary* pLibraryWidget, KeyboardEventFilter* pKeyboard) {
    BasePlaylistFeature::bindLibraryWidget(pLibraryWidget, pKeyboard);
    connect(&PlayerInfo::instance(),
            &PlayerInfo::trackChanged,
            this,
            &PreparationFeature::slotAddLoadedTrackToPreparation);
    m_pLibraryWidget = QPointer(pLibraryWidget);
}

// void PreparationFeature::bindLibraryPreparationWindowWidget(
//         WLibraryPreparationWindow* pLibraryPreparationWindowWidget,
//         KeyboardEventFilter* pKeyboard) {
//     BasePlaylistFeature::bindLibraryPreparationWindowWidget(pLibraryPreparationWindowWidget,
//     pKeyboard); connect(&PlayerInfo::instance(),
//             &PlayerInfo::currentPlayingTrackChanged,
//             this,
//             &PreparationFeature::slotPlayingTrackChanged);
//     m_pLibraryPreparationWindowWidget =
//     QPointer(pLibraryPreparationWindowWidget);
// }

void PreparationFeature::deleteAllUnlockedPlaylistsWithFewerTracks() {
    ScopedTransaction transaction(m_pLibrary->trackCollectionManager()
                    ->internalCollection()
                    ->database());
    int minTrackCount = m_pConfig->getValue(
            kHistoryMinTracksToKeepConfigKey,
            kHistoryMinTracksToKeepDefault);
    m_playlistDao.deleteAllUnlockedPlaylistsWithFewerTracks(PlaylistDAO::PLHT_SET_PREPARATION,
            minTrackCount);
    transaction.commit();
}

void PreparationFeature::slotDeletePlaylist() {
    if (!m_lastRightClickedIndex.isValid()) {
        return;
    }
    int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (playlistId == m_currentPlaylistId) {
        // the current preparationlist must not be deleted
        return;
    } else if (playlistId == m_yearNodeId) {
        // this is a YEAR node
        slotDeleteAllUnlockedChildPlaylists();
    } else {
        // regular preparationlist, call the base implementation
        BasePlaylistFeature::slotDeletePlaylist();
    }
}

void PreparationFeature::onRightClick(const QPoint& globalPos) {
    Q_UNUSED(globalPos);
    m_lastRightClickedIndex = QModelIndex();

    // Create the right-click menu
    // QMenu menu(NULL);
    // menu.addAction(m_pCreatePlaylistAction);
    // TODO(DASCHUER) add something like disable logging
    // menu.exec(globalPos);
}

void PreparationFeature::onRightClickChild(const QPoint& globalPos, const QModelIndex& index) {
    // Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;

    int playlistId = playlistIdFromIndex(index);
    // not a real entry
    if (playlistId == kInvalidPlaylistId) {
        return;
    }

    QMenu menu(m_pSidebarWidget);
    if (playlistId == m_yearNodeId) {
        // this is a YEAR item
        menu.addAction(m_pLockAllChildPlaylists);
        menu.addAction(m_pUnlockAllChildPlaylists);
        menu.addSeparator();
        menu.addAction(m_pDeleteAllChildPlaylists);
    } else {
        // this is a playlist
        bool locked = m_playlistDao.isPlaylistLocked(playlistId);
        m_pDeletePlaylistAction->setEnabled(!locked);
        m_pRenamePlaylistAction->setEnabled(!locked);
        m_pJoinWithPreviousAction->setEnabled(!locked);
        m_pLockPlaylistAction->setText(locked ? tr("Unlock") : tr("Lock"));

        menu.addAction(m_pShowTrackModelInLibraryWindowAction);
        menu.addSeparator();
        menu.addAction(m_pAddToAutoDJAction);
        menu.addAction(m_pAddToAutoDJTopAction);
        menu.addSeparator();
        menu.addAction(m_pRenamePlaylistAction);
        if (playlistId != m_currentPlaylistId) {
            // Todays playlist should not be locked or deleted
            menu.addAction(m_pDeletePlaylistAction);
            menu.addAction(m_pLockPlaylistAction);
            menu.addAction(m_pMarkTracksPlayedAction);
        }
        if (index.sibling(index.row() + 1, index.column()).isValid()) {
            // The very first (oldest) preparationlist cannot be joint
            menu.addAction(m_pJoinWithPreviousAction);
        }
        if (playlistId == m_currentPlaylistId) {
            // Todays playlists can change !
            m_pStartNewPlaylist->setEnabled(
                    m_playlistDao.tracksInPlaylist(m_currentPlaylistId) > 0);
            menu.addAction(m_pStartNewPlaylist);
        }
        menu.addSeparator();
        menu.addAction(m_pExportPlaylistAction);
    }

    menu.exec(globalPos);
}

/// Purpose: When inserting or removing playlists,
/// we require the sidebar model not to reset.
/// This method queries the database and does dynamic insertion
/// Use a custom model in the preparation for grouping by year
/// @param selectedId row which should be selected
QModelIndex PreparationFeature::constructChildModel(int selectedId) {
    // qDebug() << "PreparationFeature::constructChildModel() selected:" << selectedId;
    // Setup the sidebar playlist model
    QSqlDatabase database =
            m_pLibrary->trackCollectionManager()->internalCollection()->database();

    QString queryString = QStringLiteral(
            "CREATE TEMPORARY VIEW IF NOT EXISTS %1 "
            "AS SELECT "
            "  Playlists.id AS id, "
            "  Playlists.name AS name, "
            "  Playlists.date_created AS date_created, "
            "  LOWER(Playlists.name) AS sort_name, "
            "  max(PlaylistTracks.position) AS count,"
            "  SUM(library.duration) AS durationSeconds "
            "FROM Playlists "
            "LEFT JOIN PlaylistTracks "
            "  ON PlaylistTracks.playlist_id = Playlists.id "
            "LEFT JOIN library "
            "  ON PlaylistTracks.track_id = library.id "
            "  WHERE Playlists.hidden = %2 "
            "  GROUP BY Playlists.id")
                                  .arg(m_countsDurationTableName,
                                          QString::number(PlaylistDAO::PLHT_SET_PREPARATION));
    ;
    queryString.append(
            mixxx::DbConnection::collateLexicographically(
                    " ORDER BY sort_name"));
    QSqlQuery query(database);
    if (!query.exec(queryString)) {
        LOG_FAILED_QUERY(query);
    }

    // Setup the sidebar playlist model
    QSqlTableModel playlistTableModel(this, database);
    playlistTableModel.setTable(m_countsDurationTableName);
    playlistTableModel.setSort(playlistTableModel.fieldIndex("id"), Qt::DescendingOrder);
    playlistTableModel.select();
    while (playlistTableModel.canFetchMore()) {
        playlistTableModel.fetchMore();
    }
    QSqlRecord record = playlistTableModel.record();
    int nameColumn = record.indexOf("name");
    int idColumn = record.indexOf("id");
    int createdColumn = record.indexOf("date_created");
    int countColumn = record.indexOf("count");
    int durationColumn = record.indexOf("durationSeconds");

    // Nice to have: restore previous expanded/collapsed state of YEAR items
    clearChildModel();
    QMap<int, TreeItem*> groups;
    std::vector<std::unique_ptr<TreeItem>> itemList;
    // Generous estimate (number of years the db is used ;))
    itemList.reserve(kNumToplevelHistoryEntries + 15);

    for (int row = 0; row < playlistTableModel.rowCount(); ++row) {
        int id =
                playlistTableModel
                        .data(playlistTableModel.index(row, idColumn))
                        .toInt();
        QString name =
                playlistTableModel
                        .data(playlistTableModel.index(row, nameColumn))
                        .toString();
        QDateTime dateCreated =
                playlistTableModel
                        .data(playlistTableModel.index(row, createdColumn))
                        .toDateTime();
        int count = playlistTableModel
                            .data(playlistTableModel.index(row, countColumn))
                            .toInt();
        int duration =
                playlistTableModel
                        .data(playlistTableModel.index(row, durationColumn))
                        .toInt();
        QString label = createPlaylistLabel(name, count, duration);

        // Create the TreeItem whose parent is the invisible root item.
        // Show only [kNumToplevelHistoryEntries] recent playlists at the top level
        // before grouping them by year.
        if (row >= kNumToplevelHistoryEntries) {
            // group by year
            int yearCreated = dateCreated.date().year();

            auto i = groups.find(yearCreated);
            TreeItem* pGroupItem;
            if (i != groups.end() && i.key() == yearCreated) {
                // get YEAR item the playlist will sorted into
                pGroupItem = i.value();
            } else {
                // create YEAR item the playlist will sorted into
                // store id of empty placeholder playlist
                auto pNewGroupItem = std::make_unique<TreeItem>(
                        QString::number(yearCreated), m_yearNodeId);
                pGroupItem = pNewGroupItem.get();
                groups.insert(yearCreated, pGroupItem);
                itemList.push_back(std::move(pNewGroupItem));
            }

            TreeItem* pItem = pGroupItem->appendChild(label, id);
            pItem->setBold(m_playlistIdsOfSelectedTrack.contains(id));
            decorateChild(pItem, id);
        } else {
            // add most recent top-level playlist
            auto pItem = std::make_unique<TreeItem>(label, id);
            pItem->setBold(m_playlistIdsOfSelectedTrack.contains(id));
            decorateChild(pItem.get(), id);

            itemList.push_back(std::move(pItem));
        }
    }

    // Append all the newly created TreeItems in a dynamic way to the childmodel
    m_pSidebarModel->insertTreeItemRows(std::move(itemList), 0);

    return indexFromPlaylistId(selectedId);
}

void PreparationFeature::decorateChild(TreeItem* item, int playlistId) {
    if (playlistId == m_currentPlaylistId) {
        item->setIcon(QIcon(":/images/library/ic_library_history_current.svg"));
    } else if (m_playlistDao.isPlaylistLocked(playlistId)) {
        item->setIcon(QIcon(":/images/library/ic_library_locked.svg"));
    } else {
        item->setIcon(QIcon());
    }
}

/// Invoked on startup to create new current playlist and by "Finish current and start new"
void PreparationFeature::slotGetNewPlaylist() {
    // qDebug() << "slotGetNewPlaylist() successfully triggered !";

    // create a new playlist for today
    QString preparation_name_format;
    QString preparation_name;

    preparation_name = QString("Preparation %1").arg(QDate::currentDate().toString(Qt::ISODate));
    preparation_name_format = preparation_name + " #%1";
    int i = 1;

    // calculate name of the todays preparationlist
    while (m_playlistDao.getPlaylistIdFromName(preparation_name) != kInvalidPlaylistId) {
        preparation_name = preparation_name_format.arg(++i);
    }

    // qDebug() << "Creating session preparation playlist name:" << preparation_name;
    m_currentPlaylistId = m_playlistDao.createPlaylist(
            preparation_name, PlaylistDAO::PLHT_SET_PREPARATION);

    if (m_currentPlaylistId == kInvalidPlaylistId) {
        qDebug() << "Setlog playlist Creation Failed";
        qDebug() << "An unknown error occurred while creating playlist: "
                 << preparation_name;
    } else {
        m_recentTracks.clear();
    }

    // reload child model again because the 'added' signal fired by PlaylistDAO
    // might have triggered slotPlaylistTableChanged() before m_currentPlaylistId was set,
    // which causes the wrong playlist being decorated as 'current'
    slotPlaylistTableChanged(m_currentPlaylistId);
}

void PreparationFeature::slotJoinWithPrevious() {
    // qDebug() << "PreparationFeature::slotJoinWithPrevious() row:" <<
    // m_lastRightClickedIndex.data();
    if (!m_lastRightClickedIndex.isValid()) {
        return;
    }

    int clickedPlaylistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (clickedPlaylistId == kInvalidPlaylistId) {
        return;
    }

    bool locked = m_playlistDao.isPlaylistLocked(clickedPlaylistId);
    if (locked) {
        qDebug() << "Aborting playlist join because playlist"
                 << clickedPlaylistId << "is locked.";
        return;
    }

    // Add every track from right-clicked playlist to that with the next smaller ID
    int previousPlaylistId = m_playlistDao.getPreviousPlaylist(
            clickedPlaylistId, PlaylistDAO::PLHT_SET_PREPARATION);
    if (previousPlaylistId == kInvalidPlaylistId) {
        qDebug() << "Aborting playlist join because playlist"
                 << clickedPlaylistId << "because there's no previous playlist.";
        return;
    }

    // Right-clicked playlist may not be loaded. Use a temporary model to
    // keep sidebar selection and table view in sync
    std::unique_ptr<PlaylistTableModel> pPlaylistTableModel =
            std::make_unique<PlaylistTableModel>(this,
                    m_pLibrary->trackCollectionManager(),
                    "mixxx.db.model.playlist_export");
    pPlaylistTableModel->selectPlaylist(previousPlaylistId);

    if (clickedPlaylistId == m_currentPlaylistId) {
        // mark all the Tracks in the previous Playlist as played
        pPlaylistTableModel->select();
        int rows = pPlaylistTableModel->rowCount();
        for (int i = 0; i < rows; ++i) {
            QModelIndex index = pPlaylistTableModel->index(i, 0);
            if (index.isValid()) {
                TrackPointer pTrack = pPlaylistTableModel->getTrack(index);
                DEBUG_ASSERT(pTrack != nullptr);
                // Do not update the play count, just set played status.
                pTrack->updatePlayedStatusKeepPlayCount(true);
            }
        }

        // Change current preparationlist
        m_currentPlaylistId = previousPlaylistId;
    }
    qDebug() << "slotJoinWithPrevious() current:"
             << clickedPlaylistId
             << " previous:" << previousPlaylistId;
    if (m_playlistDao.copyPlaylistTracks(clickedPlaylistId, previousPlaylistId)) {
        m_playlistDao.deletePlaylist(clickedPlaylistId);
    }
}

void PreparationFeature::slotMarkAllTracksPlayed() {
    // qDebug() << "PreparationFeature::slotMarkAllTracksPlayed()";
    if (!m_lastRightClickedIndex.isValid()) {
        return;
    }

    int clickedPlaylistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (clickedPlaylistId == kInvalidPlaylistId) {
        return;
    }

    if (clickedPlaylistId == m_currentPlaylistId) {
        return;
    }

    // Right-clicked playlist may not be loaded. Use a temporary model to
    // keep sidebar selection and table view in sync
    std::unique_ptr<PlaylistTableModel> pPlaylistTableModel =
            std::make_unique<PlaylistTableModel>(this,
                    m_pLibrary->trackCollectionManager(),
                    "mixxx.db.model.playlist_export");
    pPlaylistTableModel->selectPlaylist(clickedPlaylistId);
    // mark all the Tracks in the previous Playlist as played
    pPlaylistTableModel->select();
    int rows = pPlaylistTableModel->rowCount();
    for (int i = 0; i < rows; ++i) {
        QModelIndex index = pPlaylistTableModel->index(i, 0);
        if (index.isValid()) {
            TrackPointer pTrack = pPlaylistTableModel->getTrack(index);
            DEBUG_ASSERT(pTrack != nullptr);
            // Do not update the play count, just set played status.
            pTrack->updatePlayedStatusKeepPlayCount(true);
        }
    }
}

void PreparationFeature::slotLockAllChildPlaylists() {
    lockOrUnlockAllChildPlaylists(true);
}

void PreparationFeature::slotUnlockAllChildPlaylists() {
    lockOrUnlockAllChildPlaylists(false);
}

void PreparationFeature::lockOrUnlockAllChildPlaylists(bool lock) {
    if (!m_lastRightClickedIndex.isValid()) {
        return;
    }
    if (lock) {
        qWarning() << "lock all child playlists of" << m_lastRightClickedIndex.data().toString();
    } else {
        qWarning() << "unlock all child playlists of" << m_lastRightClickedIndex.data().toString();
    }
    TreeItem* item = static_cast<TreeItem*>(m_lastRightClickedIndex.internalPointer());
    if (!item) {
        return;
    }
    const QList<TreeItem*> yearChildren = item->children();
    if (yearChildren.isEmpty()) {
        return;
    }

    QSet<int> ids;
    for (const auto& pChild : yearChildren) {
        bool ok = false;
        int childId = pChild->getData().toInt(&ok);
        if (ok && childId != kInvalidPlaylistId) {
            ids.insert(childId);
        }
    }
    m_playlistDao.setPlaylistsLocked(ids, lock);
}

void PreparationFeature::slotDeleteAllUnlockedChildPlaylists() {
    if (!m_lastRightClickedIndex.isValid()) {
        return;
    }
    TreeItem* item = static_cast<TreeItem*>(m_lastRightClickedIndex.internalPointer());
    if (!item) {
        return;
    }
    const QList<TreeItem*> yearChildren = item->children();
    if (yearChildren.isEmpty()) {
        return;
    }
    QString year = m_lastRightClickedIndex.data().toString();

    QMessageBox::StandardButton btn = QMessageBox::question(nullptr,
            tr("Confirm Deletion"),
            //: %1 is the year
            //: <b> + </b> are used to make the text in between bold in the popup
            //: <br> is a linebreak
            tr("Do you really want to delete all unlocked playlist from <b>%1</b>?<br><br>")
                    .arg(year),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
    if (btn != QMessageBox::Yes) {
        return;
    }

    QStringList ids;
    int count = 0;
    for (const auto& pChild : yearChildren) {
        bool ok = false;
        int childId = pChild->getData().toInt(&ok);
        if (ok && childId != kInvalidPlaylistId) {
            ids.append(pChild->getData().toString());
            count++;
        }
    }
    // Double-check, this is a weighty decision
    btn = QMessageBox::warning(nullptr,
            tr("Confirm Deletion"),
            //: %1 is the number of playlists to be deleted
            //: %2 is the year
            //: <b> + </b> are used to make the text in between bold in the popup
            //: <br> is a linebreak
            tr("Deleting %1 playlists from <b>%2</b>.<br><br>")
                    .arg(QString::number(count), year),
            QMessageBox::Ok | QMessageBox::Cancel,
            QMessageBox::Cancel);
    if (btn != QMessageBox::Ok) {
        return;
    }
    qDebug() << "preparation: deleting all unlocked playlists of" << year;
    m_playlistDao.deleteUnlockedPlaylists(std::move(ids));
}

// pLoadedTrack
void PreparationFeature::slotAddLoadedTrackToPreparation(const QString& group,
        TrackPointer loadedTrack,
        TrackPointer pOldTrack) {
    Q_UNUSED(group);
    Q_UNUSED(pOldTrack);

    if (loadedTrack) {
        TrackId newLoadedTrack(loadedTrack->getId());
        qDebug() << "[PreparationFeature] -> Received Track ID" << newLoadedTrack;
    } else {
        qDebug() << "[PreparationFeature] -> Received null track!";
        return;
    }

    TrackId newLoadedTrackId(loadedTrack->getId());
    qDebug() << "PreparationFeature: Loaded Track ID received: " << newLoadedTrackId;

    if (m_pPlaylistTableModel->getPlaylist() == m_currentPlaylistId) {
        qDebug() << "PreparationFeature: Loaded Preparation PlayListId "
                    "m_pPlaylistTableModel->getPlaylist() "
                 << m_pPlaylistTableModel->getPlaylist();
        qDebug() << "PreparationFeature: Loaded Preparation PlayListId " << m_currentPlaylistId;
        // View needs a refresh

        bool hasActiveView = false;
        if (m_pLibraryWidget) {
            WTrackTableView* view = dynamic_cast<WTrackTableView*>(
                    m_pLibraryWidget->getActiveView());
            if (view != nullptr) {
                hasActiveView = true;
                const QList<TrackId> trackIds = view->getSelectedTrackIds();
                m_pPlaylistTableModel->appendTrack(newLoadedTrackId);
                view->setSelectedTracks(trackIds);
            }
        }

        if (!hasActiveView) {
            m_pPlaylistTableModel->appendTrack(newLoadedTrackId);
        }
    } else {
        // TODO(XXX): Care whether the append succeeded.
        m_playlistDao.appendTrackToPlaylist(
                newLoadedTrackId, m_currentPlaylistId);
    }
}

void PreparationFeature::slotPlaylistTableChanged(int playlistId) {
    // qDebug() << "PreparationFeature::slotPlaylistTableChanged() id:" << playlistId;
    PlaylistDAO::HiddenType type = m_playlistDao.getHiddenType(playlistId);
    if (type != PlaylistDAO::PLHT_SET_PREPARATION &&
            type != PlaylistDAO::PLHT_UNKNOWN) { // deleted Playlist
        return;
    }

    // save currently selected preparation sidebar item (if any)
    int selectedYearIndexRow = -1;
    int selectedPlaylistId = kInvalidPlaylistId;
    bool rootWasSelected = false;
    if (isChildIndexSelectedInSidebar(m_lastClickedIndex)) {
        // a child index was selected (actual playlist or YEAR item)
        int lastClickedPlaylistId = playlistIdFromIndex(m_lastClickedIndex);
        if (lastClickedPlaylistId == m_yearNodeId) {
            // a YEAR item was selected
            selectedYearIndexRow = m_lastClickedIndex.row();
        } else if (playlistId == lastClickedPlaylistId &&
                type == PlaylistDAO::PLHT_UNKNOWN) {
            // selected playlist was deleted, find a sibling.
            // prev/next works here because preparation playlists are always
            // sorted by date of creation.
            selectedPlaylistId = m_playlistDao.getPreviousPlaylist(
                    lastClickedPlaylistId,
                    PlaylistDAO::PLHT_SET_PREPARATION);
            if (selectedPlaylistId == kInvalidPlaylistId) {
                // no previous playlist, try to get the next playlist
                selectedPlaylistId = m_playlistDao.getNextPlaylist(
                        lastClickedPlaylistId,
                        PlaylistDAO::PLHT_SET_PREPARATION);
            }
        } else {
            selectedPlaylistId = lastClickedPlaylistId;
        }
    } else {
        rootWasSelected = m_pSidebarWidget &&
                m_pSidebarWidget->isFeatureRootIndexSelected(this);
    }

    QModelIndex newIndex = constructChildModel(selectedPlaylistId);

    // restore selection
    if (selectedYearIndexRow != -1) {
        // if row is valid this means newIndex is invalid anyway
        newIndex = m_pSidebarModel->index(selectedYearIndexRow, 0);
        if (!newIndex.isValid()) {
            // seems like we deleted the oldest (bottom) YEAR node while it was
            // selected. Try to pick the row above
            newIndex = m_pSidebarModel->index(selectedYearIndexRow - 1, 0);
        }
    }
    if (newIndex.isValid()) {
        emit featureSelect(this, newIndex);
        activateChild(newIndex);
    } else if (rootWasSelected) {
        // calling featureSelect with invalid index will select the root item
        emit featureSelect(this, newIndex);
        activate(); // to reload the new current playlist
    }
}

void PreparationFeature::slotPlaylistContentOrLockChanged(const QSet<int>& playlistIds) {
    // qDebug() << "PreparationFeature::slotPlaylistContentOrLockChanged() for"
    //          << playlistIds.count() << "playlist(s)";
    QSet<int> idsToBeUpdated;
    for (const auto playlistId : std::as_const(playlistIds)) {
        if (m_playlistDao.getHiddenType(playlistId) == PlaylistDAO::PLHT_SET_PREPARATION) {
            idsToBeUpdated.insert(playlistId);
        }
    }
    updateChildModel(idsToBeUpdated);
}

void PreparationFeature::slotPlaylistTableRenamed(int playlistId, const QString& newName) {
    Q_UNUSED(newName);
    // qDebug() << "PreparationFeature::slotPlaylistTableRenamed() Id:" << playlistId;
    if (m_playlistDao.getHiddenType(playlistId) == PlaylistDAO::PLHT_SET_PREPARATION) {
        updateChildModel(QSet<int>{playlistId});
    }
}

void PreparationFeature::slotShowInLibraryWindow() {
    // qDebug() << "PreparationFeature::slotShowInLibraryWindow()";
    int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
    qDebug() << "PreparationFeature::slotShowInLibraryWindow() playlistId: " << playlistId;
    if (playlistId == kInvalidPlaylistId) {
        // may happen during initialization
        return;
    }

    emit saveModelState();
    m_pPlaylistTableModel->selectPlaylist(playlistId);
    // emit showTrackModel(m_pPlaylistTableModel);
    emit showTrackModel(m_pPlaylistTableModel);
    if (playlistId == m_yearNodeId) {
        // Disable search and cover art for YEAR items
        emit disableSearch();
        emit enableCoverArtDisplay(false);
    } else {
        emit enableCoverArtDisplay(true);
    }
}

void PreparationFeature::activate() {
    // The root item was clicked, so activate the current playlist.
    m_lastClickedIndex = m_pSidebarModel->getRootIndex();
    m_lastRightClickedIndex = QModelIndex();
    activatePlaylist(m_currentPlaylistId);
}

void PreparationFeature::activateChild(const QModelIndex& index) {
    // qDebug() << "PreparationFeature::activateChild()" << index;
    int playlistId = playlistIdFromIndex(index);
    if (playlistId == kInvalidPlaylistId) {
        // may happen during initialization
        return;
    }
    m_lastClickedIndex = index;
    m_lastRightClickedIndex = QModelIndex();

    if (ControlObject::exists(ConfigKey("[Skin]", "show_preparation_window"))) {
        auto proxy = std::make_unique<PollingControlProxy>("[Skin]", "show_preparation_window");
        proxy->set(1);
    }

    emit saveModelState();
    m_pPlaylistTableModel->selectPlaylist(playlistId);
    emit showTrackModelInPreparationWindow(m_pPlaylistTableModel);
    // emit showTrackModelInPreparationWindow(m_pPlaylistTableModel);
    emit enableCoverArtDisplay(true);
}

void PreparationFeature::activatePlaylist(int playlistId) {
    // qDebug() << "PreparationFeature::activatePlaylist()" << playlistId;
    if (playlistId == kInvalidPlaylistId) {
        return;
    }
    QModelIndex index = indexFromPlaylistId(playlistId);
    VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
        return;
    }

    if (ControlObject::exists(ConfigKey("[Skin]", "show_preparation_window"))) {
        auto proxy = std::make_unique<PollingControlProxy>("[Skin]", "show_preparation_window");
        proxy->set(1);
    }

    emit saveModelState();
    m_pPlaylistTableModel->selectPlaylist(playlistId);
    emit showTrackModelInPreparationWindow(m_pPlaylistTableModel);
    // Update sidebar selection only if this is a child, incl. current playlist
    // and YEAR nodes.
    // indexFromPlaylistId() can't be used because, in case the root item was
    // selected, that would switch to the 'current' child.
    if (m_lastClickedIndex != m_pSidebarModel->getRootIndex()) {
        m_lastClickedIndex = index;
        m_lastRightClickedIndex = QModelIndex();
        emit featureSelect(this, index);

        if (playlistId == m_yearNodeId) {
            // Disable search and cover art for YEAR items
            emit disableSearch();
            emit enableCoverArtDisplay(false);
            return;
        }
    }
    emit enableCoverArtDisplay(true);
}

QString PreparationFeature::getRootViewHtml() const {
    // Instead of the help text, the preparation shows the current playlist
    return QString();
}
