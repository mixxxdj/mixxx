#include <QtDebug>
#include <QMenu>
#include <QFile>
#include <QFileInfo>

#include "library/playlistfeature.h"

#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextbrowser.h"
#include "library/trackcollection.h"
#include "library/playlisttablemodel.h"
#include "library/treeitem.h"
#include "library/queryutil.h"
#include "library/parser.h"
#include "mixxxkeyboard.h"
#include "soundsourceproxy.h"
#include "util/dnd.h"
#include "util/time.h"

PlaylistFeature::PlaylistFeature(QObject* parent,
                                 TrackCollection* pTrackCollection,
                                 ConfigObject<ConfigValue>* pConfig)
        : BasePlaylistFeature(parent, pConfig, pTrackCollection,
                              "PLAYLISTHOME") {
    m_pPlaylistTableModel = new PlaylistTableModel(this, pTrackCollection,
                                                   "mixxx.db.model.playlist");

    //construct child model
    TreeItem *rootItem = new TreeItem();
    m_childModel.setRootItem(rootItem);
    constructChildModel(-1);
}

PlaylistFeature::~PlaylistFeature() {
}

QVariant PlaylistFeature::title() {
    return tr("Playlists");
}

QIcon PlaylistFeature::getIcon() {
    return QIcon(":/images/library/ic_library_playlist.png");
}

void PlaylistFeature::onRightClick(const QPoint& globalPos) {
    m_lastRightClickedIndex = QModelIndex();

    //Create the right-click menu
    QMenu menu(NULL);
    menu.addAction(m_pCreatePlaylistAction);
    menu.exec(globalPos);
}

void PlaylistFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
    //Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;
    int playlistId = playlistIdFromIndex(index);

    bool locked = m_playlistDao.isPlaylistLocked(playlistId);
    m_pDeletePlaylistAction->setEnabled(!locked);
    m_pRenamePlaylistAction->setEnabled(!locked);

    m_pLockPlaylistAction->setText(locked ? tr("Unlock") : tr("Lock"));

    //Create the right-click menu
    QMenu menu(NULL);
    menu.addAction(m_pCreatePlaylistAction);
    menu.addSeparator();
    menu.addAction(m_pAddToAutoDJAction);
    menu.addAction(m_pAddToAutoDJTopAction);
    menu.addSeparator();
    menu.addAction(m_pRenamePlaylistAction);
    menu.addAction(m_pDuplicatePlaylistAction);
    menu.addAction(m_pDeletePlaylistAction);
    menu.addAction(m_pLockPlaylistAction);
    menu.addSeparator();
    menu.addAction(m_pAnalyzePlaylistAction);
    menu.addSeparator();
    menu.addAction(m_pImportPlaylistAction);
    menu.addAction(m_pExportPlaylistAction);
    menu.exec(globalPos);
}

bool PlaylistFeature::dropAcceptChild(const QModelIndex& index, QList<QUrl> urls,
                                      QObject* pSource) {
    int playlistId = playlistIdFromIndex(index);
    //m_playlistDao.appendTrackToPlaylist(url.toLocalFile(), playlistId);

    QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(urls, false, true);

    QList<int> trackIds;
    if (pSource) {
        trackIds = m_pTrackCollection->getTrackDAO().getTrackIds(files);
        m_pTrackCollection->getTrackDAO().unhideTracks(trackIds);
    } else {
        // If a track is dropped onto a playlist's name, but the track isn't in the
        // library, then add the track to the library before adding it to the
        // playlist.
        // Adds track, does not insert duplicates, handles unremoving logic.
        trackIds = m_pTrackCollection->getTrackDAO().addTracks(files, true);
    }

    // remove tracks that could not be added
    for (int trackId = 0; trackId < trackIds.size(); ++trackId) {
        if (trackIds.at(trackId) < 0) {
            trackIds.removeAt(trackId--);
        }
    }

    // Return whether appendTracksToPlaylist succeeded.
    return m_playlistDao.appendTracksToPlaylist(trackIds, playlistId);
}

bool PlaylistFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    int playlistId = playlistIdFromIndex(index);
    bool locked = m_playlistDao.isPlaylistLocked(playlistId);

    QFileInfo file(url.toLocalFile());
    bool formatSupported = SoundSourceProxy::isFilenameSupported(file.fileName()) ||
            Parser::isPlaylistFilenameSupported(file.fileName());
    return !locked && formatSupported;
}

void PlaylistFeature::buildPlaylistList() {
    m_playlistList.clear();

    QString queryString = QString(
        "CREATE TEMPORARY VIEW IF NOT EXISTS PlaylistsCountsDurations "
        "AS SELECT "
        "  Playlists.id as id, "
        "  Playlists.name as name, "
        "  COUNT(library.id) as count, "
        "  SUM(library.duration) as durationSeconds "
        "FROM Playlists "
        "LEFT JOIN PlaylistTracks ON PlaylistTracks.playlist_id = Playlists.id "
        "LEFT JOIN library ON PlaylistTracks.track_id = library.id "
        "WHERE Playlists.hidden = 0 "
        "GROUP BY Playlists.id;");
    QSqlQuery query(m_pTrackCollection->getDatabase());
    if (!query.exec(queryString)) {
        LOG_FAILED_QUERY(query);
    }

    // Setup the sidebar playlist model
    QSqlTableModel playlistTableModel(this, m_pTrackCollection->getDatabase());
    playlistTableModel.setTable("PlaylistsCountsDurations");
    playlistTableModel.setSort(playlistTableModel.fieldIndex("name"),
                               Qt::AscendingOrder);
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
        int id = playlistTableModel.data(
            playlistTableModel.index(row, idColumn)).toInt();
        QString name = playlistTableModel.data(
            playlistTableModel.index(row, nameColumn)).toString();
        int count = playlistTableModel.data(
            playlistTableModel.index(row, countColumn)).toInt();
        int duration = playlistTableModel.data(
            playlistTableModel.index(row, durationColumn)).toInt();
        m_playlistList.append(qMakePair(id, QString("%1 (%2) %3")
                                        .arg(name, QString::number(count),
                                             Time::formatSeconds(duration, false))));
    }
}

void PlaylistFeature::decorateChild(TreeItem* item, int playlist_id) {
    if (m_playlistDao.isPlaylistLocked(playlist_id)) {
        item->setIcon(QIcon(":/images/library/ic_library_locked.png"));
    } else {
        item->setIcon(QIcon());
    }
}

void PlaylistFeature::slotPlaylistTableChanged(int playlistId) {
    if (!m_pPlaylistTableModel) {
        return;
    }

    //qDebug() << "slotPlaylistTableChanged() playlistId:" << playlistId;
    enum PlaylistDAO::HiddenType type = m_playlistDao.getHiddenType(playlistId);
    if (type == PlaylistDAO::PLHT_NOT_HIDDEN ||
        type == PlaylistDAO::PLHT_UNKNOWN) { // In case of a deleted Playlist
        clearChildModel();
        m_lastRightClickedIndex = constructChildModel(playlistId);

        if (type != PlaylistDAO::PLHT_UNKNOWN) {
            // Switch the view to the playlist.
            m_pPlaylistTableModel->setTableModel(playlistId);
            // Update selection
            emit(featureSelect(this, m_lastRightClickedIndex));
        }
    }
}

QString PlaylistFeature::getRootViewHtml() const {
    QString playlistsTitle = tr("Playlists");
    QString playlistsSummary = tr("Playlists are ordered lists of songs that allow you to plan your DJ sets.");
    QString playlistsSummary2 = tr("Some DJs construct playlists before they perform live, but others prefer to build them on-the-fly.");
    QString playlistsSummary3 = tr("When using a playlist during a live DJ set, remember to always pay close attention to how your audience reacts to the music you've chosen to play.");
    QString playlistsSummary4 = tr("It may be necessary to skip some songs in your prepared playlist or add some different songs in order to maintain the energy of your audience.");
    QString createPlaylistLink = tr("Create New Playlist");

    QString html;
    html.append(QString("<h2>%1</h2>").arg(playlistsTitle));
    html.append("<table border=\"0\" cellpadding=\"5\"><tr><td>");
    html.append(QString("<p>%1</p>").arg(playlistsSummary));
    html.append(QString("<p>%1</p>").arg(playlistsSummary2));
    html.append(QString("<p>%1 %2</p>").arg(playlistsSummary3,
                                            playlistsSummary4));
    html.append("</td></tr>");
    html.append(QString("<tr><td><a href=\"create\">%1</a>")
                .arg(createPlaylistLink));
    html.append("</td></tr></table>");
    return html;
}
