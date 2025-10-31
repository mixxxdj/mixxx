#include "library/trackset/playlist/groupedplaylistsfeature.h"

#include <QMenu>
#include <QSqlTableModel>
#include <QtDebug>

#include "library/library.h"
#include "library/parser.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/playlist/groupedplayliststablemodel.h"
#include "library/treeitem.h"
#include "moc_groupedplaylistsfeature.cpp"
#include "sources/soundsourceproxy.h"
#include "util/db/dbconnection.h"
#include "util/duration.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wtracktableview.h"

namespace {
constexpr bool sDebug = true;

} // anonymous namespace

GroupedPlaylistsFeature::GroupedPlaylistsFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig)
        : BaseGroupedPlaylistsFeature(pLibrary,
                  pConfig,
                  new GroupedPlaylistsTableModel(nullptr,
                          pLibrary->trackCollectionManager(),
                          pConfig,
                          "mixxx.db.model.groupedplaylists"),
                  QStringLiteral("GROUPEDPLAYLISTSHOME"),
                  QStringLiteral("playlist"),
                  QStringLiteral("PlaylistsCountsDurations")) {
    // construct child model
    std::unique_ptr<TreeItem> pRootItem = TreeItem::newRoot(this);
    m_pSidebarModel->setRootItem(std::move(pRootItem));
    // constructChildModel(kInvalidPlaylistId);
    rebuildChildModel(kInvalidPlaylistId);

    m_pShufflePlaylistAction = new QAction(tr("Shuffle Playlist"), this);
    connect(m_pShufflePlaylistAction,
            &QAction::triggered,
            this,
            &GroupedPlaylistsFeature::slotShufflePlaylist);
}

QVariant GroupedPlaylistsFeature::title() {
    return tr("Playlists (Grouped)");
}

void GroupedPlaylistsFeature::onRightClick(const QPoint& globalPos) {
    m_lastRightClickedIndex = QModelIndex();
    QMenu menu(m_pSidebarWidget);
    menu.addAction(m_pCreateGroupedPlaylistsAction);
    menu.addSeparator();
    menu.addAction(m_pCreateImportGroupedPlaylistsAction);
    menu.exec(globalPos);
}

void GroupedPlaylistsFeature::onRightClickChild(
        const QPoint& globalPos, const QModelIndex& index) {
    // Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;
    int playlistId = playlistIdFromIndex(index);
    if (sDebug) {
        qDebug() << "[BaseGroupedPlaylistsFeature] toggle of RightClickChild " << playlistId;
    }
    bool locked = m_playlistDao.isPlaylistLocked(playlistId);
    m_pDeleteGroupedPlaylistsAction->setEnabled(!locked);
    m_pRenameGroupedPlaylistsAction->setEnabled(!locked);

    m_pLockGroupedPlaylistsAction->setText(locked ? tr("Unlock") : tr("Lock"));

    QMenu menu(m_pSidebarWidget);
    menu.addAction(m_pCreateGroupedPlaylistsAction);
    menu.addSeparator();
    // TODO If playlist is selected and has more than one track selected
    // show "Shuffle selected tracks", else show "Shuffle playlist"?
    menu.addAction(m_pShufflePlaylistAction);
    menu.addSeparator();
    menu.addAction(m_pRenameGroupedPlaylistsAction);
    menu.addAction(m_pDuplicateGroupedPlaylistsAction);
    menu.addAction(m_pDeleteGroupedPlaylistsAction);
    menu.addAction(m_pLockGroupedPlaylistsAction);
    menu.addSeparator();
    menu.addAction(m_pAddToAutoDJAction);
    menu.addAction(m_pAddToAutoDJTopAction);
    menu.addAction(m_pAddToAutoDJReplaceAction);
    menu.addSeparator();
    menu.addAction(m_pAnalyzeGroupedPlaylistsAction);
    menu.addSeparator();
    menu.addAction(m_pImportGroupedPlaylistsAction);
    menu.addAction(m_pExportGroupedPlaylistsAction);
    menu.addAction(m_pExportTrackFilesAction);
    menu.exec(globalPos);
}

bool GroupedPlaylistsFeature::dropAcceptChild(
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

bool GroupedPlaylistsFeature::dragMoveAcceptChild(const QModelIndex& index, const QUrl& url) {
    int playlistId = playlistIdFromIndex(index);
    bool locked = m_playlistDao.isPlaylistLocked(playlistId);

    bool formatSupported = SoundSourceProxy::isUrlSupported(url) ||
            Parser::isPlaylistFilenameSupported(url.toLocalFile());
    return !locked && formatSupported;
}

void GroupedPlaylistsFeature::slotShufflePlaylist() {
    int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (playlistId == kInvalidPlaylistId) {
        return;
    }

    if (m_playlistDao.isPlaylistLocked(playlistId)) {
        if (sDebug) {
            qDebug() << "[GROUPEDPLAYLISTSFEATURE] -> Can't shuffle locked playlist"
                     << playlistId << " - "
                     << m_playlistDao.getPlaylistName(playlistId);
        }
        return;
    }

    // Shuffle all tracks
    // If the playlist is loaded/visible shuffle only selected tracks
    QModelIndexList selection;
    if (isChildIndexSelectedInSidebar(m_lastRightClickedIndex) &&
            m_pGroupedPlaylistsTableModel->getPlaylist() == playlistId) {
        if (m_pLibraryWidget) {
            WTrackTableView* view = dynamic_cast<WTrackTableView*>(
                    m_pLibraryWidget->getActiveView());
            if (view != nullptr) {
                selection = view->selectionModel()->selectedIndexes();
            }
        }
        m_pGroupedPlaylistsTableModel->shuffleTracks(selection, QModelIndex());
    } else {
        // Create a temp model so we don't need to select the playlist
        // in the persistent model in order to shuffle it
        std::unique_ptr<GroupedPlaylistsTableModel> pGroupedPlaylistsTableModel =
                std::make_unique<GroupedPlaylistsTableModel>(this,
                        m_pLibrary->trackCollectionManager(),
                        m_pConfig,
                        "mixxx.db.model.playlist_shuffle");
        pGroupedPlaylistsTableModel->selectPlaylist(playlistId);
        pGroupedPlaylistsTableModel->setSort(
                pGroupedPlaylistsTableModel->fieldIndex(
                        ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION),
                Qt::AscendingOrder);
        pGroupedPlaylistsTableModel->select();

        pGroupedPlaylistsTableModel->shuffleTracks(selection, QModelIndex());
    }
}

void GroupedPlaylistsFeature::decorateChild(TreeItem* item, int playlistId) {
    if (m_playlistDao.isPlaylistLocked(playlistId)) {
        item->setIcon(
                QIcon(":/images/library/ic_library_locked_tracklist.svg"));
    } else {
        item->setIcon(QIcon());
    }
}

void GroupedPlaylistsFeature::slotPlaylistTableChanged(int playlistId) {
    if (sDebug) {
        qDebug() << "[GROUPEDPLAYLISTSFEATURE] -> slotPlaylistTableChanged() playlistId:"
                 << playlistId;
    }
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
    rebuildChildModel(selectedPlaylistId);
}

void GroupedPlaylistsFeature::slotPlaylistContentOrLockChanged(const QSet<int>& playlistIds) {
    // qDebug() << "GroupedPlaylistsFeature::slotPlaylistContentOrLockChanged()
    QSet<int> idsToBeUpdated;
    for (const auto playlistId : std::as_const(playlistIds)) {
        if (m_playlistDao.getHiddenType(playlistId) ==
                PlaylistDAO::PLHT_NOT_HIDDEN) {
            idsToBeUpdated.insert(playlistId);
        }
    }
    // Update the playlists set to allow toggling bold correctly after
    // tracks have been dropped on sidebar items
    m_playlistDao.getPlaylistsTrackIsIn(m_selectedTrackId, &m_playlistIdsOfSelectedTrack);
    // oldupdateChildModel(idsToBeUpdated);
    updateChildModel(idsToBeUpdated);
}

void GroupedPlaylistsFeature::slotPlaylistTableRenamed(int playlistId, const QString& newName) {
    Q_UNUSED(newName);
    // qDebug() << "GroupedPlaylistsFeature::slotPlaylistTableRenamed() playlistId:" << playlistId;
    if (m_playlistDao.getHiddenType(playlistId) == PlaylistDAO::PLHT_NOT_HIDDEN) {
        // Maybe we need to re-sort the sidebar items, so call slotPlaylistTableChanged()
        // in order to rebuild the model, not just updateChildModel()
        slotPlaylistTableChanged(playlistId);
    }
}

QString GroupedPlaylistsFeature::getRootViewHtml() const {
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
