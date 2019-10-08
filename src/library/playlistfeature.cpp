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
#include "controllers/keyboard/keyboardeventfilter.h"
#include "sources/soundsourceproxy.h"
#include "util/db/dbconnection.h"
#include "util/dnd.h"
#include "util/duration.h"

namespace {

QString createPlaylistLabel(
        QString name,
        int count,
        int duration) {
    return QString("%1 (%2) %3").arg(name, QString::number(count),
            mixxx::Duration::formatTime(duration, mixxx::Duration::Precision::SECONDS));
}

} // anonymous namespace


PlaylistFeature::PlaylistFeature(QObject* parent,
                                 TrackCollection* pTrackCollection,
                                 UserSettingsPointer pConfig)
        : BasePlaylistFeature(parent, pConfig, pTrackCollection,
                              "PLAYLISTHOME"),
          m_icon(":/images/library/ic_library_playlist.svg") {
    m_pPlaylistTableModel = new PlaylistTableModel(this, pTrackCollection,
                                                   "mixxx.db.model.playlist");

    //construct child model
    auto pRootItem = std::make_unique<TreeItem>(this);
    m_childModel.setRootItem(std::move(pRootItem));
    constructChildModel(-1);
}

PlaylistFeature::~PlaylistFeature() {
}

QVariant PlaylistFeature::title() {
    return tr("Playlists");
}

QIcon PlaylistFeature::getIcon() {
    return m_icon;
}

void PlaylistFeature::onRightClick(const QPoint& globalPos) {
    m_lastRightClickedIndex = QModelIndex();

    //Create the right-click menu
    QMenu menu(NULL);
    menu.addAction(m_pCreatePlaylistAction);
    menu.addSeparator();
    menu.addAction(m_pCreateImportPlaylistAction);
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
    menu.addAction(m_pExportTrackFilesAction);
    menu.exec(globalPos);
}

bool PlaylistFeature::dropAcceptChild(const QModelIndex& index, QList<QUrl> urls,
                                      QObject* pSource) {
    int playlistId = playlistIdFromIndex(index);

    QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(urls, false, true);

    QList<TrackId> trackIds;
    if (pSource) {
        trackIds = m_pTrackCollection->getTrackDAO().getTrackIds(files);
        m_pTrackCollection->unhideTracks(trackIds);
    } else {
        // If a track is dropped onto a playlist's name, but the track isn't in the
        // library, then add the track to the library before adding it to the
        // playlist.
        // Adds track, does not insert duplicates, handles unremoving logic.
        trackIds = m_pTrackCollection->getTrackDAO().addMultipleTracks(files, true);
    }

    // remove tracks that could not be added
    for (int trackIdIndex = 0; trackIdIndex < trackIds.size(); ++trackIdIndex) {
        if (!trackIds.at(trackIdIndex).isValid()) {
            trackIds.removeAt(trackIdIndex--);
        }
    }

    // Return whether appendTracksToPlaylist succeeded.
    return m_playlistDao.appendTracksToPlaylist(trackIds, playlistId);
}

bool PlaylistFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    int playlistId = playlistIdFromIndex(index);
    bool locked = m_playlistDao.isPlaylistLocked(playlistId);

    bool formatSupported = SoundSourceProxy::isUrlSupported(url) ||
            Parser::isPlaylistFilenameSupported(url.toLocalFile());
    return !locked && formatSupported;
}

QList<BasePlaylistFeature::IdAndLabel> PlaylistFeature::createPlaylistLabels() {
    QList<BasePlaylistFeature::IdAndLabel> playlistLabels;
    QString queryString = QString(
        "CREATE TEMPORARY VIEW IF NOT EXISTS PlaylistsCountsDurations "
        "AS SELECT "
        "  Playlists.id AS id, "
        "  Playlists.name AS name, "
        "  LOWER(Playlists.name) AS sort_name, "
        "  COUNT(case library.mixxx_deleted when 0 then 1 else null end) AS count, "
        "  SUM(case library.mixxx_deleted when 0 then library.duration else 0 end) AS durationSeconds "
        "FROM Playlists "
        "LEFT JOIN PlaylistTracks ON PlaylistTracks.playlist_id = Playlists.id "
        "LEFT JOIN library ON PlaylistTracks.track_id = library.id "
        "WHERE Playlists.hidden = 0 "
        "GROUP BY Playlists.id");
    queryString.append(mixxx::DbConnection::collateLexicographically(
            " ORDER BY sort_name"));
    QSqlQuery query(m_pTrackCollection->database());
    if (!query.exec(queryString)) {
        LOG_FAILED_QUERY(query);
    }

    // Setup the sidebar playlist model
    QSqlTableModel playlistTableModel(this, m_pTrackCollection->database());
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
        int id = playlistTableModel.data(
                playlistTableModel.index(row, idColumn)).toInt();
        QString name = playlistTableModel.data(
                playlistTableModel.index(row, nameColumn)).toString();
        int count = playlistTableModel.data(
                playlistTableModel.index(row, countColumn)).toInt();
        int duration = playlistTableModel.data(
                playlistTableModel.index(row, durationColumn)).toInt();
        BasePlaylistFeature::IdAndLabel idAndLabel;
        idAndLabel.id = id;
        idAndLabel.label = createPlaylistLabel(name, count, duration);
        playlistLabels.append(idAndLabel);
    }
    return playlistLabels;
}

QString PlaylistFeature::fetchPlaylistLabel(int playlistId) {
    // Setup the sidebar playlist model
    QSqlTableModel playlistTableModel(this, m_pTrackCollection->database());
    playlistTableModel.setTable("PlaylistsCountsDurations");
    QString filter = "id=" + QString::number(playlistId);
    playlistTableModel.setFilter(filter);
    playlistTableModel.select();
    while (playlistTableModel.canFetchMore()) {
        playlistTableModel.fetchMore();
    }
    QSqlRecord record = playlistTableModel.record();
    int nameColumn = record.indexOf("name");
    int countColumn = record.indexOf("count");
    int durationColumn = record.indexOf("durationSeconds");

    DEBUG_ASSERT(playlistTableModel.rowCount() <= 1);
    if (playlistTableModel.rowCount() > 0) {
        QString name = playlistTableModel.data(
                playlistTableModel.index(0, nameColumn)).toString();
        int count = playlistTableModel.data(
                playlistTableModel.index(0, countColumn)).toInt();
        int duration = playlistTableModel.data(
                playlistTableModel.index(0, durationColumn)).toInt();
        return createPlaylistLabel(name, count, duration);
    }
    return QString();
}

void PlaylistFeature::decorateChild(TreeItem* item, int playlistId) {
    if (m_playlistDao.isPlaylistLocked(playlistId)) {
        item->setIcon(QIcon(":/images/library/ic_library_locked_tracklist.svg"));
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
    }
}

void PlaylistFeature::slotPlaylistContentChanged(int playlistId) {
    if (!m_pPlaylistTableModel) {
        return;
    }

    //qDebug() << "slotPlaylistContentChanged() playlistId:" << playlistId;
    enum PlaylistDAO::HiddenType type = m_playlistDao.getHiddenType(playlistId);
    if (type == PlaylistDAO::PLHT_NOT_HIDDEN ||
        type == PlaylistDAO::PLHT_UNKNOWN) { // In case of a deleted Playlist
        updateChildModel(playlistId);
    }
}



void PlaylistFeature::slotPlaylistTableRenamed(int playlistId,
                                               QString /* a_strName */) {
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
            activatePlaylist(playlistId);
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
    html.append(QString("<p>%1</p>").arg(playlistsSummary));
    html.append(QString("<p>%1</p>").arg(playlistsSummary2));
    html.append(QString("<p>%1<br>%2</p>").arg(playlistsSummary3,
                                            playlistsSummary4));
    html.append(QString("<a style=\"color:#0496FF;\" href=\"create\">%1</a>")
                .arg(createPlaylistLink));
    return html;
}
