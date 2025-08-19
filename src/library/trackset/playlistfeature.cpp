#include "library/trackset/playlistfeature.h"

#include <QMenu>
#include <QSqlTableModel>
#include <QtDebug>

#include "library/library.h"
#include "library/parser.h"
#include "library/playlisttablemodel.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/treeitem.h"
#include "moc_playlistfeature.cpp"
#include "sources/soundsourceproxy.h"
#include "util/db/dbconnection.h"
#include "util/duration.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wtracktableview.h"

PlaylistFeature::PlaylistFeature(Library* pLibrary, UserSettingsPointer pConfig)
        : BasePlaylistFeature(pLibrary,
                  pConfig,
                  new PlaylistTableModel(nullptr,
                          pLibrary->trackCollectionManager(),
                          "mixxx.db.model.playlist"),
                  QStringLiteral("PLAYLISTHOME"),
                  QStringLiteral("playlist"),
                  QStringLiteral("PlaylistsCountsDurations")) {
    // construct child model
    std::unique_ptr<TreeItem> pRootItem = TreeItem::newRoot(this);
    m_pSidebarModel->setRootItem(std::move(pRootItem));
    constructChildModel(kInvalidPlaylistId);

    m_pShufflePlaylistAction = make_parented<QAction>(tr("Shuffle Playlist"), this);
    connect(m_pShufflePlaylistAction,
            &QAction::triggered,
            this,
            &PlaylistFeature::slotShufflePlaylist);

    m_pUnlockPlaylistsAction =
            make_parented<QAction>(tr("Unlock all playlists"), this);
    connect(m_pUnlockPlaylistsAction,
            &QAction::triggered,
            this,
            &PlaylistFeature::slotUnlockAllPlaylists);

    m_pDeleteAllUnlockedPlaylistsAction =
            make_parented<QAction>(tr("Delete all unlocked playlists"), this);
    connect(m_pDeleteAllUnlockedPlaylistsAction,
            &QAction::triggered,
            this,
            &PlaylistFeature::slotDeleteAllUnlockedPlaylists);
}

QVariant PlaylistFeature::title() {
    return tr("Playlists");
}

void PlaylistFeature::onRightClick(const QPoint& globalPos) {
    m_lastRightClickedIndex = QModelIndex();
    QMenu menu(m_pSidebarWidget);
    menu.addAction(m_pCreatePlaylistAction);
    menu.addSeparator();
    menu.addAction(m_pUnlockPlaylistsAction);
    menu.addAction(m_pDeleteAllUnlockedPlaylistsAction);
    menu.addSeparator();
    menu.addAction(m_pCreateImportPlaylistAction);
#ifdef __ENGINEPRIME__
    menu.addSeparator();
    menu.addAction(m_pExportAllPlaylistsToEngineAction);
#endif
    menu.exec(globalPos);
}

void PlaylistFeature::onRightClickChild(
        const QPoint& globalPos, const QModelIndex& index) {
    //Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;
    int playlistId = playlistIdFromIndex(index);

    bool locked = m_playlistDao.isPlaylistLocked(playlistId);
    m_pDeletePlaylistAction->setEnabled(!locked);
    m_pRenamePlaylistAction->setEnabled(!locked);

    m_pLockPlaylistAction->setText(locked ? tr("Unlock") : tr("Lock"));

    QMenu menu(m_pSidebarWidget);
    menu.addAction(m_pCreatePlaylistAction);
    menu.addSeparator();
    // TODO If playlist is selected and has more than one track selected
    // show "Shuffle selected tracks", else show "Shuffle playlist"?
    menu.addAction(m_pShufflePlaylistAction);
    menu.addSeparator();
    menu.addAction(m_pRenamePlaylistAction);
    menu.addAction(m_pDuplicatePlaylistAction);
    menu.addAction(m_pDeletePlaylistAction);
    menu.addAction(m_pLockPlaylistAction);
    menu.addSeparator();
    menu.addAction(m_pAddToAutoDJAction);
    menu.addAction(m_pAddToAutoDJTopAction);
    menu.addAction(m_pAddToAutoDJReplaceAction);
    menu.addSeparator();
    menu.addAction(m_pAnalyzePlaylistAction);
    menu.addSeparator();
    menu.addAction(m_pImportPlaylistAction);
    menu.addAction(m_pExportPlaylistAction);
    menu.addAction(m_pExportTrackFilesAction);
#ifdef __ENGINEPRIME__
    menu.addAction(m_pExportPlaylistToEngineAction);
#endif
    menu.exec(globalPos);
}

bool PlaylistFeature::dropAcceptChild(
        const QModelIndex& index, const QList<QUrl>& urls, QObject* pSource) {
    int playlistId = playlistIdFromIndex(index);
    VERIFY_OR_DEBUG_ASSERT(playlistId >= 0) {
        return false;
    }
    // If a track is dropped onto a playlist's name, but the track isn't in the
    // library, then add the track to the library before adding it to the
    // playlist.
    // pSource != nullptr it is a drop from inside Mixxx and indicates all
    // tracks already in the DB
    QList<TrackId> trackIds = m_pLibrary->trackCollectionManager()
                                      ->resolveTrackIdsFromUrls(urls, !pSource);
    if (trackIds.isEmpty()) {
        return false;
    }

    // Return whether appendTracksToPlaylist succeeded.
    return m_playlistDao.appendTracksToPlaylist(trackIds, playlistId);
}

bool PlaylistFeature::dragMoveAcceptChild(const QModelIndex& index, const QUrl& url) {
    int playlistId = playlistIdFromIndex(index);
    bool locked = m_playlistDao.isPlaylistLocked(playlistId);

    bool formatSupported = SoundSourceProxy::isUrlSupported(url) ||
            Parser::isPlaylistFilenameSupported(url.toLocalFile());
    return !locked && formatSupported;
}

QList<BasePlaylistFeature::IdAndLabel> PlaylistFeature::createPlaylistLabels() {
    QSqlDatabase database =
            m_pLibrary->trackCollectionManager()->internalCollection()->database();

    QList<BasePlaylistFeature::IdAndLabel> playlistLabels;
    QString queryString = QStringLiteral(
            "CREATE TEMPORARY VIEW IF NOT EXISTS %1 "
            "AS SELECT "
            "  Playlists.id AS id, "
            "  Playlists.name AS name, "
            "  LOWER(Playlists.name) AS sort_name, "
            "  COUNT(case library.mixxx_deleted when 0 then 1 else null end) "
            "    AS count, "
            "  SUM(case library.mixxx_deleted "
            "    when 0 then library.duration else 0 end) AS durationSeconds "
            "FROM Playlists "
            "LEFT JOIN PlaylistTracks "
            "  ON PlaylistTracks.playlist_id = Playlists.id "
            "LEFT JOIN library "
            "  ON PlaylistTracks.track_id = library.id "
            "  WHERE Playlists.hidden = %2 "
            "  GROUP BY Playlists.id")
                                  .arg(m_countsDurationTableName,
                                          QString::number(
                                                  PlaylistDAO::PLHT_NOT_HIDDEN));
    queryString.append(
            mixxx::DbConnection::collateLexicographically(
                    " ORDER BY sort_name"));
    QSqlQuery query(database);
    if (!query.exec(queryString)) {
        LOG_FAILED_QUERY(query);
    }

    // Setup the sidebar playlist model
    QSqlTableModel playlistTableModel(this, database);
    playlistTableModel.setTable("PlaylistsCountsDurations");
    playlistTableModel.select();
    while (playlistTableModel.canFetchMore()) {
        playlistTableModel.fetchMore();
    }
    QSqlRecord record = playlistTableModel.record();
    int nameColumn = record.indexOf("name");
    int idColumn = record.indexOf("id");
    int countColumn = record.indexOf("count");
    int durationColumn = record.indexOf("durationSeconds");

    for (int row = 0; row < playlistTableModel.rowCount(); ++row) {
        int id =
                playlistTableModel
                        .data(playlistTableModel.index(row, idColumn))
                        .toInt();
        QString name =
                playlistTableModel
                        .data(playlistTableModel.index(row, nameColumn))
                        .toString();
        int count =
                playlistTableModel
                        .data(playlistTableModel.index(row, countColumn))
                        .toInt();
        int duration =
                playlistTableModel
                        .data(playlistTableModel.index(row, durationColumn))
                        .toInt();
        BasePlaylistFeature::IdAndLabel idAndLabel;
        idAndLabel.id = id;
        idAndLabel.label = createPlaylistLabel(name, count, duration);
        playlistLabels.append(idAndLabel);
    }
    return playlistLabels;
}

void PlaylistFeature::slotShufflePlaylist() {
    int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (playlistId == kInvalidPlaylistId) {
        return;
    }

    if (m_playlistDao.isPlaylistLocked(playlistId)) {
        qDebug() << "Can't shuffle locked playlist" << playlistId
                 << m_playlistDao.getPlaylistName(playlistId);
        return;
    }

    // Shuffle all tracks
    // If the playlist is loaded/visible shuffle only selected tracks
    QModelIndexList selection;
    if (isChildIndexSelectedInSidebar(m_lastRightClickedIndex) &&
            m_pPlaylistTableModel->getPlaylist() == playlistId) {
        if (m_pLibraryWidget) {
            WTrackTableView* view = dynamic_cast<WTrackTableView*>(
                    m_pLibraryWidget->getActiveView());
            if (view != nullptr) {
                selection = view->selectionModel()->selectedIndexes();
            }
        }
        m_pPlaylistTableModel->shuffleTracks(selection, QModelIndex());
    } else {
        // Create a temp model so we don't need to select the playlist
        // in the persistent model in order to shuffle it
        std::unique_ptr<PlaylistTableModel> pPlaylistTableModel =
                std::make_unique<PlaylistTableModel>(this,
                        m_pLibrary->trackCollectionManager(),
                        "mixxx.db.model.playlist_shuffle");
        pPlaylistTableModel->selectPlaylist(playlistId);
        pPlaylistTableModel->setSort(
                pPlaylistTableModel->fieldIndex(
                        ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION),
                Qt::AscendingOrder);
        pPlaylistTableModel->select();

        pPlaylistTableModel->shuffleTracks(selection, QModelIndex());
    }
}

void PlaylistFeature::slotUnlockAllPlaylists() {
    m_playlistDao.setPlaylistsLockedByType(PlaylistDAO::PLHT_NOT_HIDDEN, false);
}

void PlaylistFeature::slotDeleteAllUnlockedPlaylists() {
    // Collect playlists to display the count
    const QList<QPair<int, QString>> playlists =
            m_playlistDao.getUnlockedPlaylists(PlaylistDAO::PLHT_NOT_HIDDEN);
    if (playlists.size() <= 0) {
        return;
    }

    QMessageBox::StandardButton btn = QMessageBox::question(nullptr,
            tr("Confirm Deletion"),
            tr("Do you really want to delete all unlocked playlists?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
    if (btn != QMessageBox::Yes) {
        return;
    }

    btn = QMessageBox::question(nullptr,
            tr("Confirm Deletion"),
            tr("Deleting %1 unlocked playlists.<br>"
               "This operation can not be undone!")
                    .arg(playlists.size()),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
    if (btn != QMessageBox::Yes) {
        return;
    }

    QStringList ids;
    for (const auto& playlist : std::as_const(playlists)) {
        ids << QString::number(playlist.first);
    }

    m_playlistDao.deleteUnlockedPlaylists(std::move(ids));
}

/// Purpose: When inserting or removing playlists,
/// we require the sidebar model not to reset.
/// This method queries the database and does dynamic insertion
/// @param selectedId entry which should be selected
QModelIndex PlaylistFeature::constructChildModel(int selectedId) {
    // qDebug() << "PlaylistFeature::constructChildModel() id:" << selectedId;

    std::vector<std::unique_ptr<TreeItem>> childrenToAdd;
    int selectedRow = -1;

    int row = 0;
    const QList<IdAndLabel> playlistLabels = createPlaylistLabels();
    for (const auto& idAndLabel : playlistLabels) {
        int playlistId = idAndLabel.id;
        QString playlistLabel = idAndLabel.label;

        if (selectedId == playlistId) {
            // save index for selection
            selectedRow = row;
        }

        // Create the TreeItem whose parent is the invisible root item
        auto pItem = std::make_unique<TreeItem>(playlistLabel, playlistId);
        pItem->setBold(m_playlistIdsOfSelectedTrack.contains(playlistId));

        decorateChild(pItem.get(), playlistId);
        childrenToAdd.push_back(std::move(pItem));
        ++row;
    }

    // Append all the newly created TreeItems in a dynamic way to the childmodel
    m_pSidebarModel->insertTreeItemRows(std::move(childrenToAdd), 0);
    if (selectedRow == -1) {
        return QModelIndex();
    }
    return m_pSidebarModel->index(selectedRow, 0);
}

QModelIndex PlaylistFeature::constructSearchCrateChildModel(int selectedId) {
    qDebug() << "[SMARTIES] [EDIT] "
                "constructSearchCrateChildModelPlaylistFeature::"
                "constructChildModel() id:"
             << selectedId;

    std::vector<std::unique_ptr<TreeItem>> childrenToAdd;
    int selectedRow = -1;

    int row = 0;
    const QList<IdAndLabel> playlistLabels = createPlaylistLabels();
    for (const auto& idAndLabel : playlistLabels) {
        int playlistId = idAndLabel.id;
        QString playlistLabel = idAndLabel.label;

        if (selectedId == playlistId) {
            // save index for selection
            selectedRow = row;
        }

        // Create the TreeItem whose parent is the invisible root item
        auto pItem = std::make_unique<TreeItem>(playlistLabel, playlistId);
        pItem->setBold(m_playlistIdsOfSelectedTrack.contains(playlistId));

        decorateChild(pItem.get(), playlistId);
        childrenToAdd.push_back(std::move(pItem));
        ++row;
    }

    // Append all the newly created TreeItems in a dynamic way to the childmodel
    m_pSidebarModel->insertTreeItemRows(std::move(childrenToAdd), 0);
    if (selectedRow == -1) {
        return QModelIndex();
    }
    return m_pSidebarModel->index(selectedRow, 0);
}

void PlaylistFeature::decorateChild(TreeItem* item, int playlistId) {
    if (m_playlistDao.isPlaylistLocked(playlistId)) {
        item->setIcon(
                QIcon(":/images/library/ic_library_locked_tracklist.svg"));
    } else {
        item->setIcon(QIcon());
    }
}

void PlaylistFeature::slotPlaylistTableChanged(int playlistId) {
    // qDebug() << "PlaylistFeature::slotPlaylistTableChanged() playlistId:" << playlistId;
    enum PlaylistDAO::HiddenType type = m_playlistDao.getHiddenType(playlistId);
    if (type != PlaylistDAO::PLHT_NOT_HIDDEN &&  // not a regular playlist
            type != PlaylistDAO::PLHT_UNKNOWN) { // not a deleted playlist
        return;
    }

    // Store current selection
    int selectedPlaylistId = kInvalidPlaylistId;
    if (isChildIndexSelectedInSidebar(m_lastClickedIndex)) {
        if (playlistId == playlistIdFromIndex(m_lastClickedIndex) &&
                type == PlaylistDAO::PLHT_UNKNOWN) {
            // if the selected playlist was deleted, find a sibling to select
            selectedPlaylistId = getSiblingPlaylistIdOf(m_lastClickedIndex);
        } else {
            // just restore the current selection
            selectedPlaylistId = playlistIdFromIndex(m_lastClickedIndex);
        }
    }

    clearChildModel();
    QModelIndex newIndex = constructChildModel(selectedPlaylistId);
    if (selectedPlaylistId != kInvalidPlaylistId && newIndex.isValid()) {
        // If a child index was selected and we got a new valid index select that.
        // Else (root item was selected or for some reason no index could be created)
        // there's nothing to do: either no child was selected earlier, or the root
        // was selected and will remain selected after the child model was rebuilt.
        selectAndActivate(newIndex);
    }
}

void PlaylistFeature::slotSearchCrateContentOrLockChanged(const QSet<int>& playlistIds) {
    // qDebug() << "PlaylistFeature::slotPlaylistContentOrLockChanged() playlistId:" << playlistIds;
    QSet<int> idsToBeUpdated;
    for (const auto playlistId : std::as_const(playlistIds)) {
        if (m_playlistDao.getHiddenType(playlistId) == PlaylistDAO::PLHT_NOT_HIDDEN) {
            idsToBeUpdated.insert(playlistId);
        }
    }
    // Update the playlists set to allow toggling bold correctly after
    // tracks have been dropped on sidebar items
    m_playlistDao.getPlaylistsTrackIsIn(m_selectedTrackId, &m_playlistIdsOfSelectedTrack);
    updateChildModel(idsToBeUpdated);
}

void PlaylistFeature::slotPlaylistContentOrLockChanged(const QSet<int>& playlistIds) {
    // qDebug() << "PlaylistFeature::slotPlaylistContentOrLockChanged() playlistId:" << playlistIds;
    QSet<int> idsToBeUpdated;
    for (const auto playlistId : std::as_const(playlistIds)) {
        if (m_playlistDao.getHiddenType(playlistId) == PlaylistDAO::PLHT_NOT_HIDDEN) {
            idsToBeUpdated.insert(playlistId);
        }
    }
    // Update the playlists set to allow toggling bold correctly after
    // tracks have been dropped on sidebar items
    m_playlistDao.getPlaylistsTrackIsIn(m_selectedTrackId, &m_playlistIdsOfSelectedTrack);
    updateChildModel(idsToBeUpdated);
}

void PlaylistFeature::slotPlaylistTableRenamed(int playlistId, const QString& newName) {
    Q_UNUSED(newName);
    // qDebug() << "PlaylistFeature::slotPlaylistTableRenamed() playlistId:" << playlistId;
    if (m_playlistDao.getHiddenType(playlistId) == PlaylistDAO::PLHT_NOT_HIDDEN) {
        // Maybe we need to re-sort the sidebar items, so call slotPlaylistTableChanged()
        // in order to rebuild the model, not just updateChildModel()
        slotPlaylistTableChanged(playlistId);
    }
}

QString PlaylistFeature::getRootViewHtml() const {
    QString playlistsTitle = tr("Playlists");
    QString playlistsSummary =
            tr("Playlists are ordered lists of tracks that allow you to plan "
               "your DJ sets.");
    QString playlistsSummary2 =
            tr("Some DJs construct playlists before they perform live, but "
               "others prefer to build them on-the-fly.");
    QString playlistsSummary3 =
            tr("When using a playlist during a live DJ set, remember to always "
               "pay close attention to how your audience reacts to the music "
               "you've chosen to play.");
    QString playlistsSummary4 =
            tr("It may be necessary to skip some tracks in your prepared "
               "playlist or add some different tracks in order to maintain the "
               "energy of your audience.");
    QString createPlaylistLink = tr("Create New Playlist");

    QString html;
    html.append(QStringLiteral("<h2>%1</h2>").arg(playlistsTitle));
    html.append(QStringLiteral("<p>%1</p>").arg(playlistsSummary));
    html.append(QStringLiteral("<p>%1</p>").arg(playlistsSummary2));
    html.append(QStringLiteral("<p>%1<br>%2</p>").arg(playlistsSummary3, playlistsSummary4));
    html.append(QStringLiteral("<a style=\"color:#0496FF;\" href=\"create\">%1</a>")
                        .arg(createPlaylistLink));
    return html;
}
