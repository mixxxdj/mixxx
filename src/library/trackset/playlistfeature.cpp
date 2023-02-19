#include "library/trackset/playlistfeature.h"

#include <QFile>
#include <QMenu>
#include <QtDebug>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/library.h"
#include "library/parser.h"
#include "library/playlisttablemodel.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/treeitem.h"
#include "moc_playlistfeature.cpp"
#include "sources/soundsourceproxy.h"
#include "util/dnd.h"
#include "util/duration.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextbrowser.h"

namespace {

QString formatLabel(const PlaylistSummary& playlistSummary) {
    return QStringLiteral("%1 (%2) %3")
            .arg(playlistSummary.getName(),
                    QString::number(playlistSummary.getTrackCount()),
                    playlistSummary.getTrackDurationText());
}

} // anonymous namespace

PlaylistFeature::PlaylistFeature(Library* pLibrary, UserSettingsPointer pConfig)
        : BasePlaylistFeature(pLibrary,
                  pConfig,
                  new PlaylistTableModel(nullptr,
                          pLibrary->trackCollectionManager(),
                          "mixxx.db.model.playlist"),
                  QStringLiteral("PLAYLISTHOME"),
                  QStringLiteral("playlist")),
          m_pTrackCollection(pLibrary->trackCollectionManager()->internalCollection()) {
    // construct child model
    std::unique_ptr<TreeItem> pRootItem = TreeItem::newRoot(this);
    m_pSidebarModel->setRootItem(std::move(pRootItem));
    constructChildModel(kInvalidPlaylistId);
}

QVariant PlaylistFeature::title() {
    return tr("Playlists");
}

void PlaylistFeature::onRightClick(const QPoint& globalPos) {
    m_lastRightClickedIndex = QModelIndex();
    QMenu menu(m_pSidebarWidget);
    menu.addAction(m_pCreatePlaylistAction);
    menu.addSeparator();
    menu.addAction(m_pCreateImportPlaylistAction);
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
    if (!trackIds.size()) {
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

QString PlaylistFeature::fetchPlaylistLabel(int playlistId) {
    PlaylistSummary playlistSummary;
    if (m_pTrackCollection->playlists().readPlaylistSummaryById(
                PlaylistId(playlistId), &playlistSummary)) {
        return formatLabel(playlistSummary);
    }

    return QString();
}

/// Purpose: When inserting or removing playlists,
/// we require the sidebar model not to reset.
/// This method queries the database and does dynamic insertion
/// @param selectedId entry which should be selected
QModelIndex PlaylistFeature::constructChildModel(int selectedId) {
    std::vector<std::unique_ptr<TreeItem>> childrenToAdd;
    childrenToAdd.reserve(m_pTrackCollection->playlists().countPlaylists(
            PlaylistDAO::HiddenType::PLHT_NOT_HIDDEN));

    int selectedRow = -1;
    PlaylistSummarySelectResult playlistSummaries(
            m_pTrackCollection->playlists().selectPlaylistSummaries(
                    PlaylistDAO::HiddenType::PLHT_NOT_HIDDEN));
    PlaylistSummary playlistSummary;
    while (playlistSummaries.populateNext(&playlistSummary)) {
        int playlistId = playlistSummary.getId().value();

        if (selectedId == playlistId) {
            // save index for selection
            selectedRow = childrenToAdd.size() - 1;
        }

        // Create the TreeItem whose parent is the invisible root item
        auto pItem = std::make_unique<TreeItem>(formatLabel(playlistSummary), playlistId);
        pItem->setBold(m_playlistIdsOfSelectedTrack.contains(playlistId));

        decorateChild(pItem.get(), playlistId);
        childrenToAdd.push_back(std::move(pItem));
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
    //qDebug() << "slotPlaylistTableChanged() playlistId:" << playlistId;
    enum PlaylistDAO::HiddenType type = m_playlistDao.getHiddenType(playlistId);
    if (type == PlaylistDAO::PLHT_NOT_HIDDEN ||
            type == PlaylistDAO::PLHT_UNKNOWN) { // In case of a deleted Playlist
        clearChildModel();
        m_lastRightClickedIndex = constructChildModel(playlistId);
    }
}

void PlaylistFeature::slotPlaylistContentChanged(QSet<int> playlistIds) {
    for (const auto playlistId : qAsConst(playlistIds)) {
        enum PlaylistDAO::HiddenType type =
                m_playlistDao.getHiddenType(playlistId);
        if (type == PlaylistDAO::PLHT_NOT_HIDDEN ||
                type == PlaylistDAO::PLHT_UNKNOWN) { // In case of a deleted Playlist
            updateChildModel(playlistId);
        }
    }
}

void PlaylistFeature::slotPlaylistTableRenamed(
        int playlistId, const QString& newName) {
    Q_UNUSED(newName);
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
