#include <QtDebug>
#include <QMenu>
#include <QDateTime>

#include "control/controlobject.h"
#include "library/features/history/historytreemodel.h"
#include "library/features/playlist/playlisttablemodel.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "widget/wlibrarysidebar.h"

#include "library/features/history/historyfeature.h"

HistoryFeature::HistoryFeature(UserSettingsPointer pConfig,
                             Library* pLibrary,
                             QObject* parent,
                             TrackCollection* pTrackCollection)
        : BasePlaylistFeature(pConfig, pLibrary, parent, pTrackCollection),
          m_playlistId(-1) {
    m_pJoinWithNextAction = new QAction(tr("Join with next"), this);
    connect(m_pJoinWithNextAction, SIGNAL(triggered()),
            this, SLOT(slotJoinWithNext()));

    m_pGetNewPlaylist = new QAction(tr("Create new history playlist"), this);
    connect(m_pGetNewPlaylist, SIGNAL(triggered()), this, SLOT(slotGetNewPlaylist()));

    // initialized in a new generic slot(get new history playlist purpose)
    emit(slotGetNewPlaylist());

    //construct child model
    delete m_childModel;
    m_childModel = m_pHistoryTreeModel = new HistoryTreeModel(this, m_pTrackCollection);
    constructChildModel(-1);
    
    connect(&PlayerInfo::instance(), SIGNAL(currentPlayingTrackChanged(TrackPointer)),
            this, SLOT(slotPlayingTrackChanged(TrackPointer)));
}

HistoryFeature::~HistoryFeature() {
    // If the history playlist we created doesn't have any tracks in it then
    // delete it so we don't end up with tons of empty playlists. This is mostly
    // for developers since they regularly open Mixxx without loading a track.
    if (m_playlistId != -1 &&
        m_playlistDao.tracksInPlaylist(m_playlistId) == 0) {
        m_playlistDao.deletePlaylist(m_playlistId);
    }
}

QVariant HistoryFeature::title() {
    return tr("History");
}

QString HistoryFeature::getIconPath() {
    return ":/images/library/ic_library_history.png";
}

QString HistoryFeature::getSettingsName() const {
    return "HistoryFeature";
}

void HistoryFeature::onRightClick(const QPoint&) {
    m_lastRightClickedIndex = QModelIndex();

    // Create the right-click menu
    // QMenu menu(NULL);
    // menu.addAction(m_pCreatePlaylistAction);
    // TODO(DASCHUER) add something like disable logging
    // menu.exec(globalPos);
}

void HistoryFeature::onRightClickChild(const QPoint& globalPos, const QModelIndex &index) {
    //Save the model index so we can get it in the action slots...
    m_lastChildClicked[m_featurePane] = m_lastRightClickedIndex = index;
    bool ok;
    int playlistId = index.data(AbstractRole::RoleDataPath).toInt(&ok);
    if (!ok || playlistId < 0) {
        return;
    }


    bool locked = m_playlistDao.isPlaylistLocked(playlistId);
    m_pDeletePlaylistAction->setEnabled(!locked);
    m_pRenamePlaylistAction->setEnabled(!locked);
    m_pJoinWithNextAction->setEnabled(!locked);

    m_pLockPlaylistAction->setText(locked ? tr("Unlock") : tr("Lock"));


    //Create the right-click menu
    QMenu menu(nullptr);
    //menu.addAction(m_pCreatePlaylistAction);
    //menu.addSeparator();
    menu.addAction(m_pAddToAutoDJAction);
    menu.addAction(m_pAddToAutoDJTopAction);
    menu.addAction(m_pRenamePlaylistAction);
    menu.addAction(m_pJoinWithNextAction);
    if (playlistId != m_playlistId) {
        // Todays playlist should not be locked or deleted
        menu.addAction(m_pDeletePlaylistAction);
        menu.addAction(m_pLockPlaylistAction);
    }
    if (playlistId == m_playlistId && m_playlistDao.tracksInPlaylist(m_playlistId) != 0) {
        // Todays playlists can change !
        menu.addAction(m_pGetNewPlaylist);
    }
    menu.addSeparator();
    menu.addAction(m_pExportPlaylistAction);
    menu.exec(globalPos);
}


void HistoryFeature::buildPlaylistList() {
    m_playlistList.clear();
    
    // Setup the sidebar playlist model    
    QSqlQuery query("SELECT id, name FROM Playlists WHERE hidden=2");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
    
    int iId = query.record().indexOf("id");
    int iName = query.record().indexOf("name");
    while (query.next()) {
        int id = query.value(iId).toInt();
        QString name = query.value(iName).toString();
        m_playlistList << PlaylistItem(id, name);
    }
}

void HistoryFeature::decorateChild(TreeItem* item, int playlist_id) {
    if (playlist_id == m_playlistId) {
        item->setIcon(QIcon(":/images/library/ic_library_history_current.png"));
    } else if (m_playlistDao.isPlaylistLocked(playlist_id)) {
        item->setIcon(QIcon(":/images/library/ic_library_locked.png"));
    } else {
        item->setIcon(QIcon());
    }
}

QModelIndex HistoryFeature::constructChildModel(int selected_id) {
    buildPlaylistList();
    QModelIndex index = m_pHistoryTreeModel->reloadListsTree(selected_id);
    if (!m_pSidebar.isNull()) {
        m_pSidebar->expandAll();
    }
    return index;
}

PlaylistTableModel* HistoryFeature::constructTableModel() {
    return new PlaylistTableModel(this, m_pTrackCollection, 
                                  "mixxx.db.model.setlog", true);
}

QSet<int> HistoryFeature::playlistIdsFromIndex(const QModelIndex& index) const {
    QList<QVariant> auxList = index.data(AbstractRole::RoleQuery).toList();
    QSet<int> playlistIds;
    for (QVariant& var : auxList) {
        bool ok;
        playlistIds.insert(var.toInt(&ok));
        if (!ok) {
            return QSet<int>();
        }
    }
    return playlistIds;
}

QModelIndex HistoryFeature::indexFromPlaylistId(int playlistId) const {
    return m_pHistoryTreeModel->indexFromPlaylistId(playlistId);
}

void HistoryFeature::slotGetNewPlaylist() {
    //qDebug() << "slotGetNewPlaylist() succesfully triggered !";

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
    m_playlistId = m_playlistDao.createPlaylist(set_log_name,
                                                PlaylistDAO::PLHT_SET_LOG);

    if (m_playlistId == -1) {
        qDebug() << "Setlog playlist Creation Failed";
        qDebug() << "An unknown error occurred while creating playlist: " << set_log_name;
    }

    slotPlaylistTableChanged(m_playlistId); // For moving selection
    showTrackModel(m_pPlaylistTableModel);
}

QWidget* HistoryFeature::createInnerSidebarWidget(KeyboardEventFilter* pKeyboard) {
    m_pSidebar = createLibrarySidebarWidget(pKeyboard);
    m_pSidebar->expandAll();
    return m_pSidebar;
}

void HistoryFeature::slotJoinWithNext() {
    //qDebug() << "slotJoinWithPrevious() row:" << m_lastRightClickedIndex.data();
    m_pPlaylistTableModel = getPlaylistTableModel(m_featurePane);
    if (m_lastRightClickedIndex.isValid()) {
        bool ok;
        int currentPlaylistId =
                m_lastRightClickedIndex.data(AbstractRole::RoleDataPath).toInt(&ok);
        
        if (!ok) {
            return;
        }

        bool locked = m_playlistDao.isPlaylistLocked(currentPlaylistId);
        
        if (locked) {
            qDebug() << "Skipping playlist deletion because playlist" << currentPlaylistId << "is locked.";
            return;
        }
        
        // Add every track from right klicked playlist to that with the next smaller ID
        // Although being the name "join with next" this is because it's ordered
        // in descendant order so actually the next playlist in the GUI is the
        // previous Id playlist.
        int previousPlaylistId = m_playlistDao.getPreviousPlaylist(currentPlaylistId, PlaylistDAO::PLHT_SET_LOG);
        if (previousPlaylistId < 0) {
            return;
        }
            
        m_pPlaylistTableModel->setTableModel(previousPlaylistId);
        
        if (currentPlaylistId == m_playlistId) {
            // mark all the Tracks in the previous Playlist as played
            
            m_pPlaylistTableModel->select();
            int rows = m_pPlaylistTableModel->rowCount();
            for (int i = 0; i < rows; ++i) {
                QModelIndex index = m_pPlaylistTableModel->index(i,0);
                if (index.isValid()) {
                    TrackPointer track = m_pPlaylistTableModel->getTrack(index);
                    // Do not update the play count, just set played status.
                    PlayCounter playCounter(track->getPlayCounter());
                    playCounter.setPlayed();
                    track->setPlayCounter(playCounter);
                }
            }
            
            // Change current setlog
            m_playlistId = previousPlaylistId;
        }
        qDebug() << "slotJoinWithPrevious() current:" << currentPlaylistId << " previous:" << previousPlaylistId;
        if (m_playlistDao.copyPlaylistTracks(currentPlaylistId, previousPlaylistId)) {
            m_playlistDao.deletePlaylist(currentPlaylistId);
            slotPlaylistTableChanged(previousPlaylistId); // For moving selection
            showTrackModel(m_pPlaylistTableModel);
        }
    }
}

void HistoryFeature::slotPlayingTrackChanged(TrackPointer currentPlayingTrack) {
    if (!currentPlayingTrack) {
        return;
    }

    TrackId currentPlayingTrackId(currentPlayingTrack->getId());
    bool track_played_recently = false;
    if (currentPlayingTrackId.isValid()) {
        // Remove the track from the recent tracks list if it's present and put
        // at the front of the list.
        track_played_recently = m_recentTracks.removeOne(currentPlayingTrackId);
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
    
    m_pPlaylistTableModel = getPlaylistTableModel(-1);

    if (m_pPlaylistTableModel->getPlaylist() == m_playlistId) {
        // View needs a refresh
        m_pPlaylistTableModel->appendTrack(currentPlayingTrackId);
    } else {
        // TODO(XXX): Care whether the append succeeded.
        m_playlistDao.appendTrackToPlaylist(currentPlayingTrackId,
                                            m_playlistId);
    }
    // Refresh sidebar tree
    constructChildModel(m_playlistId);
}

void HistoryFeature::slotPlaylistTableChanged(int playlistId) {
    if (!m_pPlaylistTableModel) {
        return;
    }

    //qDebug() << "slotPlaylistTableChanged() playlistId:" << playlistId;
    PlaylistDAO::HiddenType type = m_playlistDao.getHiddenType(playlistId);
    if (type == PlaylistDAO::PLHT_SET_LOG ||
        type == PlaylistDAO::PLHT_UNKNOWN) { // In case of a deleted Playlist
        m_lastRightClickedIndex = constructChildModel(playlistId);
        m_lastChildClicked[m_featurePane] = m_lastRightClickedIndex;
    }
}

void HistoryFeature::slotPlaylistContentChanged(int playlistId) {
    if (!m_pPlaylistTableModel) {
        return;
    }

    //qDebug() << "slotPlaylistContentChanged() playlistId:" << playlistId;
    enum PlaylistDAO::HiddenType type = m_playlistDao.getHiddenType(playlistId);
    if (type == PlaylistDAO::PLHT_SET_LOG ||
        type == PlaylistDAO::PLHT_UNKNOWN) { // In case of a deleted Playlist
        updateChildModel(playlistId);
    }
}

void HistoryFeature::slotPlaylistTableRenamed(int playlistId,
                                             QString /* a_strName */) {
    if (!m_pPlaylistTableModel) {
        return;
    }

    //qDebug() << "slotPlaylistTableChanged() playlistId:" << playlistId;
    enum PlaylistDAO::HiddenType type = m_playlistDao.getHiddenType(playlistId);
    if (type == PlaylistDAO::PLHT_SET_LOG ||
        type == PlaylistDAO::PLHT_UNKNOWN) { // In case of a deleted Playlist
        m_lastRightClickedIndex = constructChildModel(playlistId);
        m_lastChildClicked[m_featurePane] = m_lastRightClickedIndex;
        if (type != PlaylistDAO::PLHT_UNKNOWN) {
            activatePlaylist(playlistId);
        }
    }
}

QString HistoryFeature::getRootViewHtml() const {
    QString playlistsTitle = tr("History");
    QString playlistsSummary = tr("The history section automatically keeps a list of tracks you play in your DJ sets.");
    QString playlistsSummary2 = tr("This is handy for remembering what worked in your DJ sets, posting set-lists, or reporting your plays to licensing organizations.");
    QString playlistsSummary3 = tr("Every time you start Mixxx, a new history section is created. You can export it as a playlist in various formats or play it again with Auto DJ.");
    QString playlistsSummary4 = tr("You can join the current history session with a previous one by right-clicking and selecting \"Join with previous\".");

    QString html;
    html.append(QString("<h2>%1</h2>").arg(playlistsTitle));
    html.append("<table border=\"0\" cellpadding=\"5\"><tr><td>");
    html.append(QString("<p>%1</p>").arg(playlistsSummary));
    html.append(QString("<p>%1</p>").arg(playlistsSummary2));
    html.append(QString("<p>%1</p>").arg(playlistsSummary3));
    html.append(QString("<p>%1</p>").arg(playlistsSummary4));
    html.append("</td></tr></table>");
    return html;
}
