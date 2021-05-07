#include "aoide/libraryfeature.h"

#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QJsonDocument>
#include <QMenu>
#include <QMessageBox>

#include "aoide/collectionlistmodel.h"
#include "aoide/settings.h"
#include "aoide/tracksearchoverlayfilterdlg.h"
#include "aoide/tracktablemodel.h"
#include "aoide/web/createcollectedplaylisttask.h"
#include "aoide/web/deleteplaylisttask.h"
#include "aoide/web/listcollectedplayliststask.h"
#include "aoide/web/playlistappendtrackentriesbyurltask.h"
#include "library/library.h"
#include "library/treeitem.h"
#include "sources/soundsourceproxy.h"
#include "util/assert.h"
#include "util/cmdlineargs.h"
#include "util/logger.h"
#include "widget/wlibrary.h"

namespace {

const mixxx::Logger kLogger("aoide LibraryFeature");

const QString kDefaultPlaylistKind = QStringLiteral("mixxx.org");
const QString kHistoryPlaylistKind = QStringLiteral("history.mixxx.org");
const QString kAutoDjPlaylistKind = QStringLiteral("autodj.mixxx.org");

const QString kInitialSearch = QStringLiteral("");

QString defaultPreparedQueriesFilePath(
        const UserSettingsPointer& settings) {
    auto filePath = aoide::Settings(settings).preparedQueriesFilePath();
    if (filePath.isEmpty()) {
        filePath = CmdlineArgs::Instance().getSettingsPath();
    }
    return filePath;
}

QJsonArray loadPreparedQueries(const QString& fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        kLogger.warning()
                << "Failed to open file:"
                << fileName;
        return QJsonArray();
    }
    QByteArray jsonData = file.readAll();
    file.close();
    if (jsonData.isEmpty()) {
        kLogger.warning()
                << "Empty file"
                << fileName;
        return QJsonArray();
    }
    QJsonParseError parseError;
    const auto jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
    // QJsonDocument::fromJson() returns a non-null document
    // if parsing succeeds and otherwise null on error. The
    // parse error must only be evaluated if the returned
    // document is null!
    if (jsonDoc.isNull() &&
            parseError.error != QJsonParseError::NoError) {
        kLogger.warning()
                << "Failed to parse JSON data:"
                << parseError.errorString()
                << "at offset"
                << parseError.offset;
        return QJsonArray();
    }
    if (!jsonDoc.isArray()) {
        kLogger.warning()
                << "Expected a JSON array with prepared queries and groups:"
                << jsonDoc;
        return QJsonArray();
    }
    return jsonDoc.array();
}

// Returns a null QString on success, otherwise an error message
QString savePreparedQueries(
        const QString& fileName,
        const QJsonArray& preparedQueries) {
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        QByteArray jsonData =
                QJsonDocument(preparedQueries).toJson(QJsonDocument::Compact);
        const auto bytesWritten = file.write(jsonData);
        file.close();
        DEBUG_ASSERT(bytesWritten <= jsonData.size());
        if (bytesWritten >= jsonData.size()) {
            return QString(); // success
        }
        kLogger.warning()
                << "Failed to save prepared queries into file:"
                << fileName
                << file.errorString();
    } else {
        kLogger.warning()
                << "Failed to open file for writing:"
                << fileName
                << file.errorString();
    }
    DEBUG_ASSERT(!file.errorString().isNull());
    return file.errorString();
}

} // anonymous namespace

namespace aoide {

LibraryFeature::LibraryFeature(
        Library* library,
        UserSettingsPointer settings,
        Subsystem* subsystem)
        : ::LibraryFeature(library, settings, QStringLiteral("aoide")),
          m_title(QStringLiteral("aoide")),
          m_preparedQueriesIcon(QStringLiteral(
                  ":/images/library/ic_library_tag-search-filter.svg")),
          m_playlistsIcon(
                  QStringLiteral(":/images/library/ic_library_playlist.svg")),
          m_trackSearchOverlayFilterAction(
                  new QAction(tr("Track search overlay filter..."), this)),
          m_loadPreparedQueriesAction(
                  new QAction(tr("Load prepared queries..."), this)),
          m_savePreparedQueriesAction(
                  new QAction(tr("Save prepared queries..."), this)),
          m_refreshQueryResultsAction(
                  new QAction(tr("Refresh query results"), this)),
          m_reloadPlaylistsAction(new QAction(tr("Reload playlists"), this)),
          m_createPlaylistAction(
                  new QAction(tr("Create new playlist..."), this)),
          m_deletePlaylistAction(new QAction(tr("Delete playlist..."), this)),
          m_refreshPlaylistEntriesAction(
                  new QAction(tr("Refresh playlist entries"), this)),
          m_subsystem(subsystem),
          m_collectionListModel(
                  make_parented<CollectionListModel>(subsystem, this)),
          m_trackTableModel(
                  make_parented<TrackTableModel>(
                          library->trackCollectionManager(), subsystem, this)),
          m_sidebarModel(make_parented<TreeItemModel>(this)),
          m_previousSearch(kInitialSearch) {
    m_sidebarModel->setRootItem(TreeItem::newRoot(this));

    // Actions
    connect(m_trackSearchOverlayFilterAction,
            &QAction::triggered,
            this,
            &LibraryFeature::slotTrackSearchOverlayFilter);
    connect(m_loadPreparedQueriesAction,
            &QAction::triggered,
            this,
            &LibraryFeature::slotLoadPreparedQueries);
    connect(m_savePreparedQueriesAction,
            &QAction::triggered,
            this,
            &LibraryFeature::slotSavePreparedQueries);
    connect(m_refreshQueryResultsAction,
            &QAction::triggered,
            this,
            &LibraryFeature::slotRefreshQueryResults);
    connect(m_reloadPlaylistsAction,
            &QAction::triggered,
            this,
            &LibraryFeature::slotReloadPlaylists);
    connect(m_createPlaylistAction,
            &QAction::triggered,
            this,
            &LibraryFeature::slotCreatePlaylist);
    connect(m_deletePlaylistAction,
            &QAction::triggered,
            this,
            &LibraryFeature::slotDeletePlaylist);
    connect(m_refreshPlaylistEntriesAction,
            &QAction::triggered,
            this,
            &LibraryFeature::slotRefreshPlaylistEntries);

    // Subsystem
    connect(m_subsystem,
            &Subsystem::connected,
            this,
            [this]() {
                reloadCollectedPlaylists();
            });
    connect(m_subsystem,
            &Subsystem::disconnected,
            this,
            [this]() {
                reloadCollectedPlaylists();
            });
    connect(m_subsystem,
            &Subsystem::collectionsChanged,
            this,
            [this](int flags) {
                if (flags & Subsystem::CollectionsChangedFlags::ACTIVE_COLLECTION) {
                    reloadCollectedPlaylists();
                }
            });

    QString preparedQueriesFilePath =
            Settings(m_pConfig).preparedQueriesFilePath();
    if (!preparedQueriesFilePath.isEmpty()) {
        reloadPreparedQueries(preparedQueriesFilePath);
    }

    kLogger.debug() << "Created instance" << this;
}

LibraryFeature::~LibraryFeature() {
    kLogger.debug() << "Destroying instance" << this;
}

QVariant LibraryFeature::title() {
    return m_title;
}

void LibraryFeature::bindLibraryWidget(
        WLibrary* /*libraryWidget*/, KeyboardEventFilter* /*keyboard*/) {
}

void LibraryFeature::bindSidebarWidget(WLibrarySidebar* /*sidebarWidget*/) {
}

TreeItemModel* LibraryFeature::sidebarModel() const {
    return m_sidebarModel;
}

void LibraryFeature::activate() {
    emit showTrackModel(m_trackTableModel);
    emit enableCoverArtDisplay(true);
}

void LibraryFeature::activateChild(const QModelIndex& index) {
    const auto currentSearch = m_trackTableModel->searchText();
    if (!currentSearch.isNull()) {
        m_previousSearch = currentSearch;
    }
    auto preparedQuery = preparedQueryAt(index);
    if (preparedQuery.isEmpty()) {
        const auto playlistEntity = playlistAt(index);
        if (playlistEntity.isEmpty()) {
            // Nothing
            if (m_activeChildIndex != index) {
                // Initial activation
                m_activeChildIndex = index;
                m_trackTableModel->reset();
            }
        } else {
            // Activate playlist
            if (m_activeChildIndex != index) {
                // Initial activation
                m_activeChildIndex = index;
                // TODO: Populate table model with playlist entries
                m_trackTableModel->reset();
            }
        }
    } else {
        // Activate prepared query
        if ((m_activeChildIndex != index) ||
                m_trackTableModel->searchText().isNull()) {
            // Initial activation
            m_activeChildIndex = index;
            DEBUG_ASSERT(!m_previousSearch.isNull());
            m_trackTableModel->searchTracks(
                    preparedQuery,
                    m_trackSearchOverlayFilter,
                    m_previousSearch);
        }
        emit restoreSearch(m_trackTableModel->searchText());
    }
    activate();
    emit switchToView(m_title);
}

void LibraryFeature::reactivateChild() {
    auto activeIndex = m_activeChildIndex;
    m_activeChildIndex = QModelIndex();
    activateChild(activeIndex);
}

QJsonObject LibraryFeature::preparedQueryAt(const QModelIndex& index) const {
    if (!index.isValid()) {
        return QJsonObject();
    }
    auto parentIndex = index;
    while (parentIndex.parent().parent().isValid()) {
        parentIndex = parentIndex.parent();
    }
    if (parentIndex.row() != 0) {
        return QJsonObject();
    }
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (!item) {
        return QJsonObject();
    }
    return item->getData().toJsonObject();
}

json::PlaylistWithEntriesSummaryEntity LibraryFeature::playlistAt(
        const QModelIndex& index) const {
    if (!index.isValid()) {
        return json::PlaylistWithEntriesSummaryEntity();
    }
    auto parentIndex = index;
    while (parentIndex.parent().parent().isValid()) {
        parentIndex = parentIndex.parent();
    }
    if (parentIndex.row() != 1) {
        return json::PlaylistWithEntriesSummaryEntity();
    }
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (!item) {
        return json::PlaylistWithEntriesSummaryEntity();
    }
    int row = item->getData().toInt();
    if (row < 0 || row >= m_playlistEntities.size()) {
        return json::PlaylistWithEntriesSummaryEntity();
    }
    return m_playlistEntities.at(row);
}

namespace {

std::vector<std::unique_ptr<TreeItem>> buildPreparedQuerySubtreeModel(
        const QJsonArray& jsonItems) {
    std::vector<std::unique_ptr<TreeItem>> treeItems;
    treeItems.reserve(jsonItems.size());
    for (auto i = 0; i < jsonItems.size(); ++i) {
        if (!jsonItems[i].isObject()) {
            kLogger.warning() << "invalid JSON item" << jsonItems[i];
            continue;
        }
        const auto& jsonItem = jsonItems[i].toObject();
        auto treeItem =
                std::make_unique<TreeItem>(jsonItem.value("label").toString());
        treeItem->setToolTip(jsonItem.value("notes").toString());
        auto jsonType = jsonItem.value("@type").toString();
        if (jsonType == "query") {
            treeItem->setData(jsonItem);
        } else if (jsonType == "group") {
            // TODO: Check JsonValue
            auto jsonValue = jsonItem.value("items");
            if (jsonValue.isArray()) {
                auto childItems =
                        buildPreparedQuerySubtreeModel(jsonValue.toArray());
                for (auto&& childItem : childItems) {
                    treeItem->appendChild(std::move(childItem));
                }
            } else {
                kLogger.warning() << "Group" << treeItem->getLabel()
                                  << "contains invalid items" << jsonValue;
            }
        } else {
            kLogger.warning() << "Unknown item type" << jsonType;
        }
        treeItems.push_back(std::move(treeItem));
    }
    return treeItems;
}

std::vector<std::unique_ptr<TreeItem>> buildPlaylistSubtreeModel(
        const QVector<json::PlaylistWithEntriesSummaryEntity>& playlistEntites) {
    std::vector<std::unique_ptr<TreeItem>> treeItems;
    treeItems.reserve(playlistEntites.size());
    for (int i = 0; i < playlistEntites.size(); ++i) {
        const auto playlist = playlistEntites[i].body();
        auto label = QString("%1 (%2)")
                             .arg(playlist.title())
                             .arg(playlist.entries().totalTracksCount());
        auto treeItem = std::make_unique<TreeItem>(std::move(label), i);
        treeItem->setToolTip(playlist.notes());
        treeItems.push_back(std::move(treeItem));
    }
    return treeItems;
}

} // anonymous namespace

void LibraryFeature::rebuildChildModel() {
    TreeItem* rootItem = m_sidebarModel->getRootItem();
    VERIFY_OR_DEBUG_ASSERT(rootItem) {
        return;
    }
    m_sidebarModel->removeRows(0, rootItem->childRows());
    auto preparedQueriesRoot =
            std::make_unique<TreeItem>(tr("Prepared Queries"));
    preparedQueriesRoot->setIcon(m_preparedQueriesIcon);
    {
        auto childItems = buildPreparedQuerySubtreeModel(m_preparedQueries);
        for (auto&& childItem : childItems) {
            preparedQueriesRoot->appendChild(std::move(childItem));
        }
    }
    auto playlistsRoot =
            std::make_unique<TreeItem>(tr("Playlists"));
    playlistsRoot->setIcon(m_playlistsIcon);
    {
        auto childItems = buildPlaylistSubtreeModel(m_playlistEntities);
        for (auto&& childItem : childItems) {
            playlistsRoot->appendChild(std::move(childItem));
        }
    }
    QList<TreeItem*> rootRows;
    rootRows.reserve(2);
    rootRows.append(preparedQueriesRoot.release());
    rootRows.append(playlistsRoot.release());
    m_sidebarModel->insertTreeItemRows(rootRows, 0);
}

void LibraryFeature::onRightClick(const QPoint& globalPos) {
    // TODO
    Q_UNUSED(globalPos);
}

void LibraryFeature::onRightClickChild(
        const QPoint& globalPos, const QModelIndex& index) {
    kLogger.debug() << "onRightClickChild" << index;
    VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
        return;
    }
    const auto parentIndex = index.parent();
    DEBUG_ASSERT(parentIndex.isValid());
    if (!parentIndex.parent().isValid()) {
        // 1st level
        DEBUG_ASSERT(index.column() == 0);
        switch (index.row()) {
        case 0: {
            // Prepared queries
            QMenu menu;
            menu.addAction(m_trackSearchOverlayFilterAction);
            menu.addSeparator();
            menu.addAction(m_loadPreparedQueriesAction);
            menu.addAction(m_savePreparedQueriesAction);
            menu.exec(globalPos);
            return;
        }
        case 1: {
            // Playlists
            QMenu menu;
            menu.addAction(m_reloadPlaylistsAction);
            menu.addSeparator();
            m_createPlaylistAction->setEnabled(
                    m_subsystem && m_subsystem->isConnected());
            menu.addAction(m_createPlaylistAction);
            m_deletePlaylistAction->setEnabled(false);
            menu.addAction(m_deletePlaylistAction);
            menu.exec(globalPos);
            return;
        }
        default:
            DEBUG_ASSERT(!"unreachable");
        }
    }
    DEBUG_ASSERT(parentIndex.parent().isValid());
    if (!parentIndex.parent().parent().isValid()) {
        // 2nd level
        if (parentIndex.row() == 1) {
            // Playlist item
            DEBUG_ASSERT(index.column() == 0); // no nesting (yet)
            if (m_activeChildIndex != index) {
                activateChild(index);
            }
            QMenu menu;
            menu.addAction(m_refreshPlaylistEntriesAction);
            menu.addSeparator();
            m_createPlaylistAction->setEnabled(
                    m_subsystem && m_subsystem->isConnected());
            menu.addAction(m_createPlaylistAction);
            m_deletePlaylistAction->setEnabled(
                    m_subsystem && m_subsystem->isConnected());
            menu.addAction(m_deletePlaylistAction);
            menu.exec(globalPos);
            return;
        }
    }
    // Prepared query item
    if (m_activeChildIndex != index) {
        activateChild(index);
    }
    auto query = preparedQueryAt(index);
    if (query.isEmpty()) {
        return;
    }
    QMenu menu;
    menu.addAction(m_refreshQueryResultsAction);
    menu.exec(globalPos);
}

void LibraryFeature::slotTrackSearchOverlayFilter() {
    auto dlg = TrackSearchOverlayFilterDlg{m_trackSearchOverlayFilter};
    if (dlg.exec() == QDialog::Accepted) {
        m_trackSearchOverlayFilter = dlg.overlayFilter();
        // Refresh the search results
        m_trackTableModel->searchTracks(
                m_trackSearchOverlayFilter,
                m_previousSearch);
    }
}

void LibraryFeature::slotLoadPreparedQueries() {
    const auto msgBoxTitle = tr("aoide: Load Prepared Queries from File");
    const auto filePath = QFileDialog::getOpenFileName(nullptr,
            msgBoxTitle,
            defaultPreparedQueriesFilePath(m_pConfig),
            "*.json");
    if (filePath.isEmpty()) {
        kLogger.info() << "No file with prepared queries selected";
        return;
    }
    if (!reloadPreparedQueries(filePath)) {
        // TODO: Display more detailed error message
        QMessageBox(QMessageBox::Warning,
                msgBoxTitle,
                tr("Failed to load prepared queries.") +
                        QStringLiteral("\n\n") + filePath,
                QMessageBox::Close)
                .exec();
    }
}

void LibraryFeature::slotSavePreparedQueries() {
    const auto msgBoxTitle = tr("aoide: Save Prepared Queries into File");
    const auto filePath = QFileDialog::getSaveFileName(nullptr,
            msgBoxTitle,
            defaultPreparedQueriesFilePath(m_pConfig),
            "*.json");
    if (filePath.isEmpty()) {
        kLogger.info() << "No file for saving prepared queries selected";
        return;
    }
    const auto errorMessage = savePreparedQueries(filePath, m_preparedQueries);
    if (errorMessage.isNull()) {
        Settings(m_pConfig).setPreparedQueriesFilePath(filePath);
        QMessageBox(QMessageBox::Information,
                msgBoxTitle,
                tr("Saved prepared queries.") + QStringLiteral("\n\n") +
                        filePath,
                QMessageBox::Ok)
                .exec();
    } else {
        QMessageBox(QMessageBox::Warning,
                msgBoxTitle,
                tr("Failed to save prepared queries:") + QChar(' ') +
                        errorMessage + QStringLiteral("\n\n") + filePath,
                QMessageBox::Close)
                .exec();
    }
}

bool LibraryFeature::reloadPreparedQueries(const QString& filePath) {
    auto preparedQueries = loadPreparedQueries(filePath);
    if (preparedQueries.isEmpty()) {
        kLogger.warning() << "Failed to load prepared queries from file:"
                          << filePath;
        return false;
    }
    m_preparedQueries = preparedQueries;
    Settings(m_pConfig).setPreparedQueriesFilePath(filePath);
    // TODO: Only rebuild the subtree underneath the prepared queries
    // node instead of the whole child model
    rebuildChildModel();
    return true;
}

bool LibraryFeature::reloadCollectedPlaylists() {
    if (!m_subsystem ||
            !m_subsystem->isConnected() ||
            !m_subsystem->activeCollection()) {
        m_playlistEntities.clear();
        // TODO: Only rebuild the subtree underneath the prepared queries
        // node instead of the whole child model
        rebuildChildModel();
        return false;
    }
    auto* listCollectedPlaylistsTask = m_pendingListCollectedPlaylistsTask.data();
    if (listCollectedPlaylistsTask) {
        kLogger.info() << "Discarding pending request"
                       << "for loading playlists";
        listCollectedPlaylistsTask->disconnect(this);
        listCollectedPlaylistsTask->invokeAbort();
        listCollectedPlaylistsTask->deleteLater();
        m_pendingListCollectedPlaylistsTask.clear();
    }
    listCollectedPlaylistsTask = m_subsystem->listCollectedPlaylists(kDefaultPlaylistKind);
    DEBUG_ASSERT(listCollectedPlaylistsTask);
    connect(listCollectedPlaylistsTask,
            &ListCollectedPlaylistsTask::succeeded,
            this,
            &LibraryFeature::slotListCollectedPlaylistsTaskSucceeded,
            Qt::UniqueConnection);
    listCollectedPlaylistsTask->invokeStart();
    m_pendingListCollectedPlaylistsTask = listCollectedPlaylistsTask;
    return true;
}

void LibraryFeature::slotListCollectedPlaylistsTaskSucceeded(
        const QVector<json::PlaylistWithEntriesSummaryEntity>& result) {
    auto* finishedListCollectedPlaylistsTask =
            qobject_cast<ListCollectedPlaylistsTask*>(sender());
    VERIFY_OR_DEBUG_ASSERT(finishedListCollectedPlaylistsTask) {
        return;
    }
    const auto finishedListCollectedPlaylistsTaskDeleter =
            mixxx::ScopedDeleteLater(finishedListCollectedPlaylistsTask);

    VERIFY_OR_DEBUG_ASSERT(finishedListCollectedPlaylistsTask ==
            m_pendingListCollectedPlaylistsTask.data()) {
        return;
    }
    m_pendingListCollectedPlaylistsTask.clear();

    m_playlistEntities = std::move(result);
    // TODO: Only rebuild the subtree underneath the playlists
    // node instead of the whole child model
    rebuildChildModel();
}

void LibraryFeature::slotRefreshQueryResults() {
    reactivateChild();
}

void LibraryFeature::slotReloadPlaylists() {
    reloadCollectedPlaylists();
}

void LibraryFeature::slotRefreshPlaylistEntries() {
    reactivateChild();
}

void LibraryFeature::slotCreatePlaylist() {
    if (!m_subsystem ||
            !m_subsystem->isConnected() ||
            !m_subsystem->activeCollection()) {
        return;
    }
    bool ok = false;
    auto name = QInputDialog::getText(nullptr,
            tr("aoide: Create New Playlist"),
            tr("Enter name for new playlist:"),
            QLineEdit::Normal,
            tr("New Playlist"),
            &ok)
                        .trimmed();
    if (!ok || name.isEmpty()) {
        return;
    }
    json::Playlist playlist;
    playlist.setCollectedAt(QDateTime::currentDateTime());
    playlist.setTitle(std::move(name));
    playlist.setKind(kDefaultPlaylistKind);
    playlist.setNotes(QStringLiteral("Created by Mixxx"));
    kLogger.info() << "Creating playlist" << playlist;
    auto* task = m_subsystem->createPlaylist(std::move(playlist));
    connect(task,
            &CreateCollectedPlaylistTask::succeeded,
            this,
            &LibraryFeature::slotPlaylistCreated);
    task->invokeStart();
}

void LibraryFeature::slotPlaylistCreated(
        const json::PlaylistEntity& playlistEntity) {
    Q_UNUSED(playlistEntity);
    auto* task = qobject_cast<CreateCollectedPlaylistTask*>(sender());
    VERIFY_OR_DEBUG_ASSERT(task) {
        return;
    }
    task->deleteLater();
    reloadCollectedPlaylists();
}

void LibraryFeature::slotDeletePlaylist() {
    if (!m_subsystem || !m_subsystem->isConnected()) {
        return;
    }
    const auto playlistEntity = playlistAt(m_activeChildIndex);
    if (playlistEntity.isEmpty()) {
        return;
    }
    const auto playlistName = playlistEntity.body().title();
    auto msgBox = QMessageBox(QMessageBox::Question,
            tr("aoide: Delete Playlist"),
            tr("Do your really want to delete this playlist?") +
                    QStringLiteral("\n\n") + playlistName,
            QMessageBox::Ok | QMessageBox::Cancel);
    if (QMessageBox::Ok != msgBox.exec()) {
        return;
    }
    auto playlistUid = playlistEntity.header().uid();
    kLogger.info() << "Deleting playlist" << playlistUid << playlistName;
    auto* task = m_subsystem->deletePlaylist(std::move(playlistUid));
    connect(task,
            &DeletePlaylistTask::succeeded,
            this,
            &LibraryFeature::slotPlaylistDeleted);
    task->invokeStart();
}

void LibraryFeature::slotPlaylistDeleted(const QString& playlistUid) {
    Q_UNUSED(playlistUid);
    auto* task = qobject_cast<DeletePlaylistTask*>(sender());
    VERIFY_OR_DEBUG_ASSERT(task) {
        return;
    }
    task->deleteLater();
    reloadCollectedPlaylists();
}

void LibraryFeature::slotPlaylistAppendTrackEntriesByUrlSucceeded(
        const json::PlaylistWithEntriesSummaryEntity& playlistEntity,
        const QList<QUrl>& unresolvedTrackUrls) {
    auto* task = qobject_cast<PlaylistAppendTrackEntriesByUrlTask*>(sender());
    VERIFY_OR_DEBUG_ASSERT(task) {
        return;
    }
    task->deleteLater();
    if (!unresolvedTrackUrls.isEmpty()) {
        QStringList missingTracks;
        missingTracks.reserve(unresolvedTrackUrls.size());
        for (const auto& url : qAsConst(unresolvedTrackUrls)) {
            missingTracks.append(url.toString());
        }
        auto msgBox = QMessageBox(QMessageBox::Question,
                tr("aoide: Add Playlist Tracks"),
                playlistEntity.body().title() + QStringLiteral(" <") +
                        playlistEntity.header().uid().value() +
                        QStringLiteral(">\n\n") +
                        tr("Failed resolve some track URLs:") +
                        QStringLiteral("\n") +
                        missingTracks.join(QStringLiteral("\n")) +
                        QStringLiteral("\n\n") +
                        tr("Please export their metadata before adding those "
                           "tracks to a playlist!"),
                QMessageBox::Close);
        msgBox.exec();
    }
    // TODO: Only reload the affected playlist instead
    // of all playlists
    reloadCollectedPlaylists();
}

bool LibraryFeature::dragMoveAcceptChild(
        const QModelIndex& index,
        const QUrl& url) {
    VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
        return false;
    }
    const auto playlist = playlistAt(index);
    if (playlist.isEmpty()) {
        // Dropping is only supported on playlists
        return false;
    }
    return SoundSourceProxy::isUrlSupported(url);
}

bool LibraryFeature::dropAcceptChild(
        const QModelIndex& index,
        const QList<QUrl>& urls,
        QObject* pSource) {
    VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(!urls.isEmpty()) {
        return false;
    }
    DEBUG_ASSERT(pSource);
    Q_UNUSED(pSource);
    const auto playlist = playlistAt(index);
    if (playlist.isEmpty()) {
        // Dropping is only supported on playlists
        return false;
    }
    kLogger.debug()
            << "Adding tracks to playlist:"
            << playlist
            << urls;
    auto* task = m_subsystem->playlistAppendTrackEntriesByUrl(
            playlist.header(),
            std::move(urls));
    DEBUG_ASSERT(task);
    connect(task,
            &PlaylistAppendTrackEntriesByUrlTask::succeeded,
            this,
            &LibraryFeature::slotPlaylistAppendTrackEntriesByUrlSucceeded,
            Qt::UniqueConnection);
    task->invokeStart();
    return true;
}

} // namespace aoide
