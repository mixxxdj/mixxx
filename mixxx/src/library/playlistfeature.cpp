#include <QtDebug>
#include <QMenu>
#include <QInputDialog>
#include <QFileDialog>
#include <QDesktopServices>

#include "library/playlistfeature.h"
#include "library/parser.h"
#include "library/parserm3u.h"
#include "library/parserpls.h"
#include "library/parsercsv.h"

#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextbrowser.h"
#include "library/trackcollection.h"
#include "library/playlisttablemodel.h"
#include "library/treeitem.h"
#include "mixxxkeyboard.h"
#include "soundsourceproxy.h"

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
    QString playlistName = index.data().toString();
    int playlistId = m_playlistDao.getPlaylistIdFromName(playlistName);


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
    menu.addAction(m_pRenamePlaylistAction);
    menu.addAction(m_pDeletePlaylistAction);
    menu.addAction(m_pLockPlaylistAction);
    menu.addSeparator();
    menu.addAction(m_pImportPlaylistAction);
    menu.addAction(m_pExportPlaylistAction);
    menu.exec(globalPos);
}

bool PlaylistFeature::dropAcceptChild(const QModelIndex& index, QList<QUrl> urls){
    //TODO: Filter by supported formats regex and reject anything that doesn't match.
    QString playlistName = index.data().toString();
    int playlistId = m_playlistDao.getPlaylistIdFromName(playlistName);
    //m_playlistDao.appendTrackToPlaylist(url.toLocalFile(), playlistId);
    QList<QFileInfo> files;
    foreach (QUrl url, urls) {
        // XXX: Possible WTF alert - Previously we thought we needed toString() here
        // but what you actually want in any case when converting a QUrl to a file
        // system path is QUrl::toLocalFile(). This is the second time we have
        // flip-flopped on this, but I think toLocalFile() should work in any
        // case. toString() absolutely does not work when you pass the result to a
        files.append(url.toLocalFile());
    }

    // If a track is dropped onto a playlist's name, but the track isn't in the
    // library, then add the track to the library before adding it to the
    // playlist.
    // Adds track, does not insert duplicates, handles unremoving logic.
    QList<int> trackIds = m_trackDao.addTracks(files, true);

    // remove tracks that could not be added
    for (int trackId =0; trackId<trackIds.size() ; trackId++) {
        if (trackIds.at(trackId) < 0) {
            trackIds.removeAt(trackId--);
        }
    }

    // appendTracksToPlaylist doesn't return whether it succeeded, so assume it
    // did.
    m_playlistDao.appendTracksToPlaylist(trackIds, playlistId);
    return true;
}

bool PlaylistFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    //TODO: Filter by supported formats regex and reject anything that doesn't match.

    QString playlistName = index.data().toString();
    int playlistId = m_playlistDao.getPlaylistIdFromName(playlistName);
    bool locked = m_playlistDao.isPlaylistLocked(playlistId);

    QFileInfo file(url.toLocalFile());
    bool formatSupported = SoundSourceProxy::isFilenameSupported(file.fileName());
    return !locked && formatSupported;
}

void PlaylistFeature::buildPlaylistList() {
    m_playlistList.clear();
    // Setup the sidebar playlist model
    QSqlTableModel playlistTableModel(this, m_pTrackCollection->getDatabase());
    playlistTableModel.setTable("Playlists");
    playlistTableModel.setFilter("hidden=0");
    playlistTableModel.setSort(playlistTableModel.fieldIndex("name"),
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
            m_pPlaylistTableModel->setPlaylist(playlistId);
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
    QString createPlaylistLink = tr("Create new playlist");

    QString html;
    html.append(QString("<h2>%1</h2>").arg(playlistsTitle));
    html.append("<table border=\"0\" cellpadding=\"5\"><tr><td>");
    html.append(QString("<p>%1</p>").arg(playlistsSummary));
    html.append(QString("<p>%1</p>").arg(playlistsSummary2));
    html.append(QString("<p>%1 %2</p>").arg(playlistsSummary3,
                                            playlistsSummary4));
    html.append("</td></tr>");
    html.append(
        QString("<tr><td><a href=\"create\">%1</a>")
        .arg(createPlaylistLink)
    );
    html.append("</td></tr></table>");
    return html;
}
