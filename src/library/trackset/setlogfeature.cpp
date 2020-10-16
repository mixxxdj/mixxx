#include "library/trackset/setlogfeature.h"

#include <QDateTime>
#include <QMenu>
#include <QtDebug>

#include "control/controlobject.h"
#include "library/library.h"
#include "library/playlisttablemodel.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/treeitem.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "track/track.h"
#include "util/compatibility.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wtracktableview.h"

SetlogFeature::SetlogFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig)
        : BasePlaylistFeature(
                  pLibrary,
                  pConfig,
                  new PlaylistTableModel(
                          nullptr,
                          pLibrary->trackCollections(),
                          "mixxx.db.model.setlog",
                          /*keep deleted tracks*/ true),
                  QStringLiteral("SETLOGHOME")),
          m_playlistId(-1),
          m_libraryWidget(nullptr),
          m_icon(QStringLiteral(":/images/library/ic_library_history.svg")) {
    //construct child model
    m_childModel.setRootItem(TreeItem::newRoot(this));
    constructChildModel(-1);

    m_pJoinWithPreviousAction = new QAction(tr("Join with previous"), this);
    connect(m_pJoinWithPreviousAction,
            &QAction::triggered,
            this,
            &SetlogFeature::slotJoinWithPrevious);

    m_pGetNewPlaylist = new QAction(tr("Create new history playlist"), this);
    connect(m_pGetNewPlaylist,
            &QAction::triggered,
            this,
            &SetlogFeature::slotGetNewPlaylist);

    // initialized in a new generic slot(get new history playlist purpose)
    slotGetNewPlaylist();
}

SetlogFeature::~SetlogFeature() {
    // If the history playlist we created doesn't have any tracks in it then
    // delete it so we don't end up with tons of empty playlists. This is mostly
    // for developers since they regularly open Mixxx without loading a track.
    if (m_playlistId != -1 &&
            m_playlistDao.tracksInPlaylist(m_playlistId) == 0) {
        m_playlistDao.deletePlaylist(m_playlistId);
    }
}

QVariant SetlogFeature::title() {
    return tr("History");
}

QIcon SetlogFeature::getIcon() {
    return m_icon;
}

void SetlogFeature::bindLibraryWidget(
        WLibrary* libraryWidget, KeyboardEventFilter* keyboard) {
    BasePlaylistFeature::bindLibraryWidget(libraryWidget, keyboard);
    connect(&PlayerInfo::instance(),
            &PlayerInfo::currentPlayingTrackChanged,
            this,
            &SetlogFeature::slotPlayingTrackChanged);
    m_libraryWidget = libraryWidget;
}

void SetlogFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    // store the sidebar widget pointer for later use in onRightClickChild
    m_pSidebarWidget = pSidebarWidget;
}

void SetlogFeature::onRightClick(const QPoint& globalPos) {
    Q_UNUSED(globalPos);
    m_lastRightClickedIndex = QModelIndex();

    // Create the right-click menu
    // QMenu menu(NULL);
    // menu.addAction(m_pCreatePlaylistAction);
    // TODO(DASCHUER) add something like disable logging
    // menu.exec(globalPos);
}

void SetlogFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
    //Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;

    QString playlistName = index.data().toString();
    int playlistId = m_playlistDao.getPlaylistIdFromName(playlistName);

    bool locked = m_playlistDao.isPlaylistLocked(playlistId);
    m_pDeletePlaylistAction->setEnabled(!locked);
    m_pRenamePlaylistAction->setEnabled(!locked);
    m_pJoinWithPreviousAction->setEnabled(!locked);

    m_pLockPlaylistAction->setText(locked ? tr("Unlock") : tr("Lock"));

    QMenu menu(m_pSidebarWidget);
    //menu.addAction(m_pCreatePlaylistAction);
    //menu.addSeparator();
    menu.addAction(m_pAddToAutoDJAction);
    menu.addAction(m_pAddToAutoDJTopAction);
    menu.addSeparator();
    menu.addAction(m_pRenamePlaylistAction);
    if (playlistId != m_playlistId) {
        // Todays playlist should not be locked or deleted
        menu.addAction(m_pDeletePlaylistAction);
        menu.addAction(m_pLockPlaylistAction);
    }
    if (index.row() > 0) {
        // The very first setlog cannot be joint
        menu.addAction(m_pJoinWithPreviousAction);
    }
    if (playlistId == m_playlistId && m_playlistDao.tracksInPlaylist(m_playlistId) != 0) {
        // Todays playlists can change !
        menu.addAction(m_pGetNewPlaylist);
    }
    menu.addSeparator();
    menu.addAction(m_pExportPlaylistAction);
    menu.exec(globalPos);
}

QList<BasePlaylistFeature::IdAndLabel> SetlogFeature::createPlaylistLabels() {
    QList<BasePlaylistFeature::IdAndLabel> playlistLabels;
    // Setup the sidebar playlist model
    QSqlTableModel playlistTableModel(this,
            m_pLibrary->trackCollections()->internalCollection()->database());
    playlistTableModel.setTable("Playlists");
    playlistTableModel.setFilter("hidden=2"); // PLHT_SET_LOG
    playlistTableModel.setSort(
            playlistTableModel.fieldIndex("id"), Qt::AscendingOrder);
    playlistTableModel.select();
    while (playlistTableModel.canFetchMore()) {
        playlistTableModel.fetchMore();
    }
    QSqlRecord record = playlistTableModel.record();
    int nameColumn = record.indexOf("name");
    int idColumn = record.indexOf("id");

    for (int row = 0; row < playlistTableModel.rowCount(); ++row) {
        int id =
                playlistTableModel
                        .data(playlistTableModel.index(row, idColumn))
                        .toInt();
        QString name =
                playlistTableModel
                        .data(playlistTableModel.index(row, nameColumn))
                        .toString();
        BasePlaylistFeature::IdAndLabel idAndLabel;
        idAndLabel.id = id;
        idAndLabel.label = name;
        playlistLabels.append(idAndLabel);
    }
    return playlistLabels;
}

QString SetlogFeature::fetchPlaylistLabel(int playlistId) {
    // Setup the sidebar playlist model
    QSqlTableModel playlistTableModel(this,
            m_pLibrary->trackCollections()->internalCollection()->database());
    playlistTableModel.setTable("Playlists");
    QString filter = "id=" + QString::number(playlistId);
    playlistTableModel.setFilter(filter);
    playlistTableModel.select();
    while (playlistTableModel.canFetchMore()) {
        playlistTableModel.fetchMore();
    }
    QSqlRecord record = playlistTableModel.record();
    int nameColumn = record.indexOf("name");

    DEBUG_ASSERT(playlistTableModel.rowCount() <= 1);
    if (playlistTableModel.rowCount() > 0) {
        return playlistTableModel.data(playlistTableModel.index(0, nameColumn))
                .toString();
    }
    return QString();
}

void SetlogFeature::decorateChild(TreeItem* item, int playlistId) {
    if (playlistId == m_playlistId) {
        item->setIcon(QIcon(":/images/library/ic_library_history_current.svg"));
    } else if (m_playlistDao.isPlaylistLocked(playlistId)) {
        item->setIcon(QIcon(":/images/library/ic_library_locked.svg"));
    } else {
        item->setIcon(QIcon());
    }
}

void SetlogFeature::slotGetNewPlaylist() {
    //qDebug() << "slotGetNewPlaylist() successfully triggered !";

    // create a new playlist for today
    QString set_log_name_format;
    QString set_log_name;

    set_log_name = QDate::currentDate().toString(Qt::ISODate);
    set_log_name_format = set_log_name + " (%1)";
    int i = 1;

    // calculate name of the todays setlog
    while (m_playlistDao.getPlaylistIdFromName(set_log_name) != -1) {
        set_log_name = set_log_name_format.arg(++i);
    }

    //qDebug() << "Creating session history playlist name:" << set_log_name;
    m_playlistId = m_playlistDao.createPlaylist(
            set_log_name, PlaylistDAO::PLHT_SET_LOG);

    if (m_playlistId == -1) {
        qDebug() << "Setlog playlist Creation Failed";
        qDebug() << "An unknown error occurred while creating playlist: "
                 << set_log_name;
    }

    reloadChildModel(m_playlistId); // For moving selection
    emit showTrackModel(m_pPlaylistTableModel);
}

void SetlogFeature::slotJoinWithPrevious() {
    //qDebug() << "slotJoinWithPrevious() row:" << m_lastRightClickedIndex.data();

    if (m_lastRightClickedIndex.isValid()) {
        int currentPlaylistId = m_playlistDao.getPlaylistIdFromName(
                m_lastRightClickedIndex.data().toString());

        if (currentPlaylistId >= 0) {
            bool locked = m_playlistDao.isPlaylistLocked(currentPlaylistId);

            if (locked) {
                qDebug() << "Skipping playlist deletion because playlist"
                         << currentPlaylistId << "is locked.";
                return;
            }

            // Add every track from right-clicked playlist to that with the next smaller ID
            int previousPlaylistId = m_playlistDao.getPreviousPlaylist(
                    currentPlaylistId, PlaylistDAO::PLHT_SET_LOG);
            if (previousPlaylistId >= 0) {
                m_pPlaylistTableModel->setTableModel(previousPlaylistId);

                if (currentPlaylistId == m_playlistId) {
                    // mark all the Tracks in the previous Playlist as played

                    m_pPlaylistTableModel->select();
                    int rows = m_pPlaylistTableModel->rowCount();
                    for (int i = 0; i < rows; ++i) {
                        QModelIndex index = m_pPlaylistTableModel->index(i, 0);
                        if (index.isValid()) {
                            TrackPointer track =
                                    m_pPlaylistTableModel->getTrack(index);
                            // Do not update the play count, just set played status.
                            PlayCounter playCounter(track->getPlayCounter());
                            playCounter.triggerLastPlayedNow();
                            track->setPlayCounter(playCounter);
                        }
                    }

                    // Change current setlog
                    m_playlistId = previousPlaylistId;
                }
                qDebug() << "slotJoinWithPrevious() current:"
                         << currentPlaylistId
                         << " previous:" << previousPlaylistId;
                if (m_playlistDao.copyPlaylistTracks(
                            currentPlaylistId, previousPlaylistId)) {
                    m_playlistDao.deletePlaylist(currentPlaylistId);
                    reloadChildModel(previousPlaylistId); // For moving selection
                    emit showTrackModel(m_pPlaylistTableModel);
                }
            }
        }
    }
}

void SetlogFeature::slotPlayingTrackChanged(TrackPointer currentPlayingTrack) {
    if (!currentPlayingTrack) {
        return;
    }

    TrackId currentPlayingTrackId(currentPlayingTrack->getId());
    bool track_played_recently = false;
    if (currentPlayingTrackId.isValid()) {
        // Remove the track from the recent tracks list if it's present and put
        // at the front of the list.
        auto it = std::find(std::begin(m_recentTracks),
                std::end(m_recentTracks),
                currentPlayingTrackId);
        if (it == std::end(m_recentTracks)) {
            track_played_recently = false;
        } else {
            track_played_recently = true;
            m_recentTracks.erase(it);
        }
        m_recentTracks.push_front(currentPlayingTrackId);

        // Keep a window of 6 tracks (inspired by 2 decks, 4 samplers)
        const int kRecentTrackWindow = 6;
        while (m_recentTracks.size() > kRecentTrackWindow) {
            m_recentTracks.pop_back();
        }
    }

    // If the track was recently played, don't increment the playcount or
    // add it to the history.
    if (track_played_recently) {
        return;
    }

    // If the track is not present in the recent tracks list, mark it
    // played and update its playcount.
    currentPlayingTrack->updatePlayCounter();

    // We can only add tracks that are Mixxx library tracks, not external
    // sources.
    if (!currentPlayingTrackId.isValid()) {
        return;
    }

    if (m_pPlaylistTableModel->getPlaylist() == m_playlistId) {
        // View needs a refresh

        WTrackTableView* view = dynamic_cast<WTrackTableView*>(
                m_libraryWidget->getActiveView());
        if (view != nullptr) {
            // We have a active view on the history. The user may have some
            // important active selection. For example putting track into crates
            // while the song changes trough autodj. The selection is then lost
            // and dataloss occurs
            const QList<TrackId> trackIds = view->getSelectedTrackIds();
            m_pPlaylistTableModel->appendTrack(currentPlayingTrackId);
            view->setSelectedTracks(trackIds);
        } else {
            m_pPlaylistTableModel->appendTrack(currentPlayingTrackId);
        }
    } else {
        // TODO(XXX): Care whether the append succeeded.
        m_playlistDao.appendTrackToPlaylist(
                currentPlayingTrackId, m_playlistId);
    }
}

void SetlogFeature::slotPlaylistTableChanged(int playlistId) {
    reloadChildModel(playlistId);
}

void SetlogFeature::reloadChildModel(int playlistId) {
    //qDebug() << "updateChildModel() playlistId:" << playlistId;
    PlaylistDAO::HiddenType type = m_playlistDao.getHiddenType(playlistId);
    if (type == PlaylistDAO::PLHT_SET_LOG ||
            type == PlaylistDAO::PLHT_UNKNOWN) { // In case of a deleted Playlist
        clearChildModel();
        m_lastRightClickedIndex = constructChildModel(playlistId);
    }
}

void SetlogFeature::slotPlaylistContentChanged(QSet<int> playlistIds) {
    for (const auto playlistId : qAsConst(playlistIds)) {
        enum PlaylistDAO::HiddenType type =
                m_playlistDao.getHiddenType(playlistId);
        if (type == PlaylistDAO::PLHT_SET_LOG ||
                type == PlaylistDAO::PLHT_UNKNOWN) { // In case of a deleted Playlist
            updateChildModel(playlistId);
        }
    }
}

void SetlogFeature::slotPlaylistTableRenamed(int playlistId, QString newName) {
    Q_UNUSED(newName);
    //qDebug() << "slotPlaylistTableRenamed() playlistId:" << playlistId;
    enum PlaylistDAO::HiddenType type = m_playlistDao.getHiddenType(playlistId);
    if (type == PlaylistDAO::PLHT_SET_LOG ||
            type == PlaylistDAO::PLHT_UNKNOWN) { // In case of a deleted Playlist
        clearChildModel();
        m_lastRightClickedIndex = constructChildModel(playlistId);
        if (type != PlaylistDAO::PLHT_UNKNOWN) {
            activatePlaylist(playlistId);
        }
    }
}

QString SetlogFeature::getRootViewHtml() const {
    QString playlistsTitle = tr("History");
    QString playlistsSummary =
            tr("The history section automatically keeps a list of tracks you "
               "play in your DJ sets.");
    QString playlistsSummary2 =
            tr("This is handy for remembering what worked in your DJ sets, "
               "posting set-lists, or reporting your plays to licensing "
               "organizations.");
    QString playlistsSummary3 =
            tr("Every time you start Mixxx, a new history section is created. "
               "You can export it as a playlist in various formats or play it "
               "again with Auto DJ.");
    QString playlistsSummary4 =
            tr("You can join the current history session with a previous one "
               "by right-clicking and selecting \"Join with previous\".");

    QString html;
    html.append(QStringLiteral("<h2>%1</h2>").arg(playlistsTitle));
    html.append(QStringLiteral("<p>%1</p>").arg(playlistsSummary));
    html.append(QStringLiteral("<p>%1</p>").arg(playlistsSummary2));
    html.append(QStringLiteral("<p>%1</p>").arg(playlistsSummary3));
    html.append(QStringLiteral("<p>%1</p>").arg(playlistsSummary4));
    return html;
}
