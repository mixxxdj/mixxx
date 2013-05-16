#include <QtDebug>
#include <QMenu>
#include <QInputDialog>
#include <QFileDialog>
#include <QDesktopServices>
#include <QDateTime>

#include "library/setlogfeature.h"

#include "controlobject.h"
#include "library/playlisttablemodel.h"
#include "library/trackcollection.h"
#include "library/treeitem.h"
#include "playerinfo.h"


SetlogFeature::SetlogFeature(QObject* parent,
                             ConfigObject<ConfigValue>* pConfig,
                             TrackCollection* pTrackCollection)
        : BasePlaylistFeature(parent, pConfig, pTrackCollection,
                              "SETLOGHOME") {
    m_pPlaylistTableModel = new PlaylistTableModel(this, pTrackCollection,
                                                   "mixxx.db.model.setlog",
                                                   true);//show all tracks
    m_pJoinWithPreviousAction = new QAction(tr("Join with previous"), this);
    connect(m_pJoinWithPreviousAction, SIGNAL(triggered()),
            this, SLOT(slotJoinWithPrevious()));

    //create a new playlist for today
    QString set_log_name_format;
    QString set_log_name;

    set_log_name = QDate::currentDate().toString(Qt::ISODate);
    set_log_name_format = set_log_name + " (%1)";
    int i = 1;

    // calculate name of the todays setlog
    while (m_playlistDao.getPlaylistIdFromName(set_log_name) != -1) {
        set_log_name = set_log_name_format.arg(++i);
    }

    qDebug() << "Creating session history playlist name:" << set_log_name;
    m_playlistId = m_playlistDao.createPlaylist(set_log_name,
                                                PlaylistDAO::PLHT_SET_LOG);

    if (m_playlistId == -1) {
        qDebug() << "Setlog playlist Creation Failed";
        qDebug() << "An unknown error occurred while creating playlist: " << set_log_name;
    }

    //construct child model
    TreeItem *rootItem = new TreeItem();
    m_childModel.setRootItem(rootItem);
    constructChildModel(-1);
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
    return QIcon(":/images/library/ic_library_history.png");
}

void SetlogFeature::bindWidget(WLibrary* libraryWidget,
                               MixxxKeyboard* keyboard) {
    BasePlaylistFeature::bindWidget(libraryWidget,
                                    keyboard);
    connect(&PlayerInfo::Instance(), SIGNAL(currentPlayingDeckChanged(int)),
            this, SLOT(slotPlayingDeckChanged(int)));
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

    m_pLockPlaylistAction->setText(locked ? tr("Unlock") : tr("Lock"));


    //Create the right-click menu
    QMenu menu(NULL);
    //menu.addAction(m_pCreatePlaylistAction);
    //menu.addSeparator();
    menu.addAction(m_pAddToAutoDJAction);
    menu.addAction(m_pAddToAutoDJTopAction);
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
    menu.addSeparator();
    menu.addAction(m_pExportPlaylistAction);
    menu.exec(globalPos);
}


void SetlogFeature::buildPlaylistList() {
    m_playlistList.clear();
    // Setup the sidebar playlist model
    QSqlTableModel playlistTableModel(this, m_pTrackCollection->getDatabase());
    playlistTableModel.setTable("Playlists");
    playlistTableModel.setFilter("hidden=2"); // PLHT_SET_LOG
    playlistTableModel.setSort(playlistTableModel.fieldIndex("id"),
                               Qt::AscendingOrder);
    playlistTableModel.select();
    while (playlistTableModel.canFetchMore()) {
        playlistTableModel.fetchMore();
    }
    int nameColumn = playlistTableModel.record().indexOf("name");
    int idColumn = playlistTableModel.record().indexOf("id");

    for (int row = 0; row < playlistTableModel.rowCount(); ++row) {
        int id = playlistTableModel.data(
            playlistTableModel.index(row, idColumn)).toInt();
        QString name = playlistTableModel.data(
            playlistTableModel.index(row, nameColumn)).toString();
        m_playlistList.append(qMakePair(id, name));
    }
}

void SetlogFeature::decorateChild(TreeItem* item, int playlist_id) {
    if (playlist_id == m_playlistId) {
        item->setIcon(QIcon(":/images/library/ic_library_history_current.png"));
    } else if (m_playlistDao.isPlaylistLocked(playlist_id)) {
        item->setIcon(QIcon(":/images/library/ic_library_locked.png"));
    } else {
        item->setIcon(QIcon());
    }
}

void SetlogFeature::slotJoinWithPrevious() {
    //qDebug() << "slotJoinWithPrevious() row:" << m_lastRightClickedIndex.data();

    if (m_lastRightClickedIndex.isValid()) {
        int currentPlaylistId = m_playlistDao.getPlaylistIdFromName(
            m_lastRightClickedIndex.data().toString());

        if (currentPlaylistId >= 0) {

            bool locked = m_playlistDao.isPlaylistLocked(currentPlaylistId);

            if (locked) {
                qDebug() << "Skipping playlist deletion because playlist" << currentPlaylistId << "is locked.";
                return;
            }

            // Add every track from right klicked playlist to that with the next smaller ID
            int previousPlaylistId = m_playlistDao.getPreviousPlaylist(currentPlaylistId, PlaylistDAO::PLHT_SET_LOG);
            if (previousPlaylistId >= 0) {

                m_pPlaylistTableModel->setPlaylist(previousPlaylistId);

                if (currentPlaylistId == m_playlistId) {
                    // mark all the Tracks in the previous Playlist as played

                    m_pPlaylistTableModel->select();
                    int rows = m_pPlaylistTableModel->rowCount();
                    for(int i = 0; i < rows; ++i){
                        QModelIndex index = m_pPlaylistTableModel->index(i,0);
                        if (index.isValid()) {
                            TrackPointer track = m_pPlaylistTableModel->getTrack(index);
                            // Do not update the playcount, just set played
                            // status.
                            track->setPlayed(true);
                        }
                    }

                    // Change current setlog
                    m_playlistId = previousPlaylistId;
                }
                qDebug() << "slotJoinWithPrevious() current:" << currentPlaylistId << " previous:" << previousPlaylistId;
                if (m_playlistDao.copyPlaylistTracks(currentPlaylistId, previousPlaylistId)) {
                    m_playlistDao.deletePlaylist(currentPlaylistId);
                    slotPlaylistTableChanged(previousPlaylistId); // For moving selection
                    emit(showTrackModel(m_pPlaylistTableModel));
                }
            }
        }
    }
}

void SetlogFeature::slotPlayingDeckChanged(int deck) {
    if (deck > 0) {
        QString chan = QString("[Channel%1]").arg(deck);
        TrackPointer currentPlayingTrack =
                PlayerInfo::Instance().getTrackInfo(chan);
        if (!currentPlayingTrack) {
            return;
        }

        int currentPlayingTrackId = currentPlayingTrack->getId();
        bool track_played_recently = false;
        if (currentPlayingTrackId >= 0) {
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
        currentPlayingTrack->setPlayedAndUpdatePlaycount(true);

        // We can only add tracks that are Mixxx library tracks, not external
        // sources.
        if (currentPlayingTrackId < 0) {
            return;
        }

        if (m_pPlaylistTableModel->getPlaylist() == m_playlistId) {
            // View needs a refresh
            m_pPlaylistTableModel->appendTrack(currentPlayingTrackId);
        } else {
            // TODO(XXX): Care whether the append succeeded.
            m_playlistDao.appendTrackToPlaylist(currentPlayingTrackId,
                                                m_playlistId);
        }
    }
}

void SetlogFeature::slotPlaylistTableChanged(int playlistId) {
    if (!m_pPlaylistTableModel) {
        return;
    }

    //qDebug() << "slotPlaylistTableChanged() playlistId:" << playlistId;
    PlaylistDAO::HiddenType type = m_playlistDao.getHiddenType(playlistId);
    if (type == PlaylistDAO::PLHT_SET_LOG ||
        type == PlaylistDAO::PLHT_UNKNOWN) { // In case of a deleted Playlist
        clearChildModel();
        m_lastRightClickedIndex = constructChildModel(playlistId);

        if (type != PlaylistDAO::PLHT_UNKNOWN) {
            // Switch the view to the playlist.
            m_pPlaylistTableModel->setPlaylist(playlistId);
            // Update selection
            emit(featureSelect(this, m_lastRightClickedIndex));
        }
    }
}


QString SetlogFeature::getRootViewHtml() const {
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
