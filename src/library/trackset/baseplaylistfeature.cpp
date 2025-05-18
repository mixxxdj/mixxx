#include "library/trackset/baseplaylistfeature.h"

#include <QAction>
#include <QFileInfo>
#include <QInputDialog>
#include <QList>
#include <QSqlTableModel>
#include <QStandardPaths>

#include "library/export/trackexportwizard.h"
#include "library/library.h"
#include "library/library_prefs.h"
#include "library/parser.h"
#include "library/parsercsv.h"
#include "library/playlisttablemodel.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/treeitem.h"
#include "library/treeitemmodel.h"
#include "moc_baseplaylistfeature.cpp"
#include "track/track.h"
#include "track/trackid.h"
#include "util/assert.h"
#include "util/defs.h"
#include "util/file.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextbrowser.h"

namespace {
constexpr QChar kUnsafeFilenameReplacement = '-';
const ConfigKey kConfigKeyLastImportExportPlaylistDirectory(
        "[Library]", "LastImportExportPlaylistDirectory");

} // anonymous namespace

using namespace mixxx::library::prefs;

BasePlaylistFeature::BasePlaylistFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig,
        PlaylistTableModel* pModel,
        const QString& rootViewName,
        const QString& iconName,
        const QString& countsDurationTableName,
        bool keepHiddenTracks)
        : BaseTrackSetFeature(pLibrary, pConfig, rootViewName, iconName),
          m_playlistDao(pLibrary->trackCollectionManager()
                                ->internalCollection()
                                ->getPlaylistDAO()),
          m_pPlaylistTableModel(pModel),
          m_countsDurationTableName(countsDurationTableName),
          m_keepHiddenTracks(keepHiddenTracks) {
    pModel->setParent(this);

    initActions();
    connectPlaylistDAO();
    connect(m_pLibrary,
            &Library::trackSelected,
            this,
            [this](const TrackPointer& pTrack) {
                const auto trackId = pTrack ? pTrack->getId() : TrackId{};
                slotTrackSelected(trackId);
            });
    connect(m_pLibrary,
            &Library::switchToView,
            this,
            &BasePlaylistFeature::slotResetSelectedTrack);
}

void BasePlaylistFeature::initActions() {
    m_pCreatePlaylistAction = make_parented<QAction>(tr("Create New Playlist"), this);
    connect(m_pCreatePlaylistAction,
            &QAction::triggered,
            this,
            &BasePlaylistFeature::slotCreatePlaylist);

    m_pRenamePlaylistAction = make_parented<QAction>(tr("Rename"), this);
    connect(m_pRenamePlaylistAction,
            &QAction::triggered,
            this,
            &BasePlaylistFeature::slotRenamePlaylist);
    m_pDuplicatePlaylistAction = make_parented<QAction>(tr("Duplicate"), this);
    connect(m_pDuplicatePlaylistAction,
            &QAction::triggered,
            this,
            &BasePlaylistFeature::slotDuplicatePlaylist);
    m_pDeletePlaylistAction = make_parented<QAction>(tr("Remove"), this);
    const auto removeKeySequence =
            // TODO(XXX): Qt6 replace enum | with QKeyCombination
            QKeySequence(static_cast<int>(kHideRemoveShortcutModifier) |
                    kHideRemoveShortcutKey);
    m_pDeletePlaylistAction->setShortcut(removeKeySequence);
    connect(m_pDeletePlaylistAction,
            &QAction::triggered,
            this,
            &BasePlaylistFeature::slotDeletePlaylist);
    m_pLockPlaylistAction = make_parented<QAction>(tr("Lock"), this);
    connect(m_pLockPlaylistAction,
            &QAction::triggered,
            this,
            &BasePlaylistFeature::slotTogglePlaylistLock);

    m_pAddToAutoDJAction = make_parented<QAction>(tr("Add to Auto DJ Queue (bottom)"), this);
    connect(m_pAddToAutoDJAction,
            &QAction::triggered,
            this,
            &BasePlaylistFeature::slotAddToAutoDJ);
    m_pAddToAutoDJTopAction = make_parented<QAction>(tr("Add to Auto DJ Queue (top)"), this);
    connect(m_pAddToAutoDJTopAction,
            &QAction::triggered,
            this,
            &BasePlaylistFeature::slotAddToAutoDJTop);
    m_pAddToAutoDJReplaceAction =
            make_parented<QAction>(tr("Add to Auto DJ Queue (replace)"), this);
    connect(m_pAddToAutoDJReplaceAction,
            &QAction::triggered,
            this,
            &BasePlaylistFeature::slotAddToAutoDJReplace);

    m_pAnalyzePlaylistAction = make_parented<QAction>(tr("Analyze entire Playlist"), this);
    connect(m_pAnalyzePlaylistAction,
            &QAction::triggered,
            this,
            &BasePlaylistFeature::slotAnalyzePlaylist);

    m_pImportPlaylistAction = make_parented<QAction>(tr("Import Playlist"), this);
    connect(m_pImportPlaylistAction,
            &QAction::triggered,
            this,
            &BasePlaylistFeature::slotImportPlaylist);
    m_pCreateImportPlaylistAction = make_parented<QAction>(tr("Import Playlist"), this);
    connect(m_pCreateImportPlaylistAction,
            &QAction::triggered,
            this,
            &BasePlaylistFeature::slotCreateImportPlaylist);
    m_pExportPlaylistAction = make_parented<QAction>(tr("Export Playlist"), this);
    connect(m_pExportPlaylistAction,
            &QAction::triggered,
            this,
            &BasePlaylistFeature::slotExportPlaylist);
    m_pExportTrackFilesAction = make_parented<QAction>(tr("Export Track Files"), this);
    connect(m_pExportTrackFilesAction,
            &QAction::triggered,
            this,
            &BasePlaylistFeature::slotExportTrackFiles);
}

void BasePlaylistFeature::connectPlaylistDAO() {
    connect(&m_playlistDao,
            &PlaylistDAO::added,
            this,
            &BasePlaylistFeature::slotPlaylistTableChangedAndScrollTo);
    connect(&m_playlistDao,
            &PlaylistDAO::lockChanged,
            this,
            &BasePlaylistFeature::slotPlaylistContentOrLockChanged);
    connect(&m_playlistDao,
            &PlaylistDAO::deleted,
            this,
            &BasePlaylistFeature::slotPlaylistTableChanged);
    connect(&m_playlistDao,
            &PlaylistDAO::playlistContentChanged,
            this,
            &BasePlaylistFeature::slotPlaylistContentOrLockChanged);
    connect(&m_playlistDao,
            &PlaylistDAO::renamed,
            this,
            // In "History" just the item is renamed, while in "Playlists" the
            // entire sidebar model is rebuilt to re-sort items by name
            &BasePlaylistFeature::slotPlaylistTableRenamed);
}

int BasePlaylistFeature::playlistIdFromIndex(const QModelIndex& index) const {
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item == nullptr) {
        return kInvalidPlaylistId;
    }

    bool ok = false;
    int playlistId = item->getData().toInt(&ok);
    if (ok) {
        return playlistId;
    } else {
        return kInvalidPlaylistId;
    }
}

void BasePlaylistFeature::selectPlaylistInSidebar(int playlistId, bool select) {
    if (!m_pSidebarWidget) {
        return;
    }
    if (playlistId == kInvalidPlaylistId) {
        return;
    }
    QModelIndex index = indexFromPlaylistId(playlistId);
    if (index.isValid()) {
        m_pSidebarWidget->selectChildIndex(index, select);
    }
}

void BasePlaylistFeature::activateChild(const QModelIndex& index) {
    //qDebug() << "BasePlaylistFeature::activateChild()" << index;
    int playlistId = playlistIdFromIndex(index);
    if (playlistId == kInvalidPlaylistId) {
        // may happen during initialization
        return;
    }
    m_lastClickedIndex = index;
    m_lastRightClickedIndex = QModelIndex();
    emit saveModelState();
    m_pPlaylistTableModel->selectPlaylist(playlistId);
    emit showTrackModel(m_pPlaylistTableModel);
    emit enableCoverArtDisplay(true);
}

void BasePlaylistFeature::activatePlaylist(int playlistId) {
    // qDebug() << "BasePlaylistFeature::activatePlaylist()" << playlistId << index;
    VERIFY_OR_DEBUG_ASSERT(playlistId != kInvalidPlaylistId) {
        return;
    }
    QModelIndex index = indexFromPlaylistId(playlistId);
    VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
        return;
    }
    m_lastClickedIndex = index;
    m_lastRightClickedIndex = QModelIndex();
    emit saveModelState();
    m_pPlaylistTableModel->selectPlaylist(playlistId);
    emit showTrackModel(m_pPlaylistTableModel);
    emit enableCoverArtDisplay(true);
    // Update selection
    emit featureSelect(this, m_lastClickedIndex);
}

void BasePlaylistFeature::renameItem(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
    slotRenamePlaylist();
}

void BasePlaylistFeature::slotRenamePlaylist() {
    int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (playlistId == kInvalidPlaylistId) {
        return;
    }
    QString oldName = m_playlistDao.getPlaylistName(playlistId);
    bool locked = m_playlistDao.isPlaylistLocked(playlistId);

    if (locked) {
        qDebug() << "Skipping playlist rename because playlist" << playlistId
                 << oldName << "is locked.";
        return;
    }
    QString newName;
    bool validNameGiven = false;

    while (!validNameGiven) {
        bool ok = false;
        newName = QInputDialog::getText(nullptr,
                tr("Rename Playlist"),
                tr("Enter new name for playlist:"),
                QLineEdit::Normal,
                oldName,
                &ok)
                          .trimmed();
        if (!ok || oldName == newName) {
            return;
        }

        int existingId = m_playlistDao.getPlaylistIdFromName(newName);

        if (existingId != kInvalidPlaylistId) {
            QMessageBox::warning(nullptr,
                    tr("Renaming Playlist Failed"),
                    tr("A playlist by that name already exists."));
        } else if (newName.isEmpty()) {
            QMessageBox::warning(nullptr,
                    tr("Renaming Playlist Failed"),
                    tr("A playlist cannot have a blank name."));
        } else {
            validNameGiven = true;
        }
    }

    m_playlistDao.renamePlaylist(playlistId, newName);
}

void BasePlaylistFeature::slotDuplicatePlaylist() {
    int oldPlaylistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (oldPlaylistId == kInvalidPlaylistId) {
        return;
    }

    QString oldName = m_playlistDao.getPlaylistName(oldPlaylistId);

    QString name;
    bool validNameGiven = false;

    while (!validNameGiven) {
        bool ok = false;
        name = QInputDialog::getText(nullptr,
                tr("Duplicate Playlist"),
                tr("Enter name for new playlist:"),
                QLineEdit::Normal,
                //: Appendix to default name when duplicating a playlist
                oldName + tr("_copy", "//:"),
                &ok)
                       .trimmed();
        if (!ok || oldName == name) {
            return;
        }

        int existingId = m_playlistDao.getPlaylistIdFromName(name);

        if (existingId != kInvalidPlaylistId) {
            QMessageBox::warning(nullptr,
                    tr("Playlist Creation Failed"),
                    tr("A playlist by that name already exists."));
        } else if (name.isEmpty()) {
            QMessageBox::warning(nullptr,
                    tr("Playlist Creation Failed"),
                    tr("A playlist cannot have a blank name."));
        } else {
            validNameGiven = true;
        }
    }

    int newPlaylistId = m_playlistDao.createPlaylist(name);

    if (newPlaylistId != kInvalidPlaylistId) {
        m_playlistDao.copyPlaylistTracks(oldPlaylistId, newPlaylistId);
    }
}

void BasePlaylistFeature::slotTogglePlaylistLock() {
    int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (playlistId == kInvalidPlaylistId) {
        return;
    }
    bool locked = !m_playlistDao.isPlaylistLocked(playlistId);

    if (!m_playlistDao.setPlaylistLocked(playlistId, locked)) {
        qDebug() << "Failed to toggle lock of playlistId " << playlistId;
    }
}

void BasePlaylistFeature::slotCreatePlaylist() {
    QString name;
    bool validNameGiven = false;

    while (!validNameGiven) {
        bool ok = false;
        name = QInputDialog::getText(nullptr,
                tr("Create New Playlist"),
                tr("Enter name for new playlist:"),
                QLineEdit::Normal,
                tr("New Playlist"),
                &ok)
                       .trimmed();
        if (!ok) {
            return;
        }

        int existingId = m_playlistDao.getPlaylistIdFromName(name);

        if (existingId != kInvalidPlaylistId) {
            QMessageBox::warning(nullptr,
                    tr("Playlist Creation Failed"),
                    tr("A playlist by that name already exists."));
        } else if (name.isEmpty()) {
            QMessageBox::warning(nullptr,
                    tr("Playlist Creation Failed"),
                    tr("A playlist cannot have a blank name."));
        } else {
            validNameGiven = true;
        }
    }

    int playlistId = m_playlistDao.createPlaylist(name);

    if (playlistId == kInvalidPlaylistId) {
        QMessageBox::warning(nullptr,
                tr("Playlist Creation Failed"),
                tr("An unknown error occurred while creating playlist: ") + name);
    }
}

/// Returns a playlist that is a sibling inside the same parent
/// as the start index
int BasePlaylistFeature::getSiblingPlaylistIdOf(QModelIndex& start) {
    QModelIndex nextIndex = start.sibling(start.row() + 1, start.column());
    if (!nextIndex.isValid() && start.row() > 0) {
        // No playlist below, looking above.
        nextIndex = start.sibling(start.row() - 1, start.column());
    }
    if (nextIndex.isValid()) {
        TreeItem* pTreeItem = m_pSidebarModel->getItem(nextIndex);
        DEBUG_ASSERT(pTreeItem != nullptr);
        if (!pTreeItem->hasChildren()) {
            return playlistIdFromIndex(nextIndex);
        }
    }
    return kInvalidPlaylistId;
}

void BasePlaylistFeature::deleteItem(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
    slotDeletePlaylist();
}

void BasePlaylistFeature::slotDeletePlaylist() {
    //qDebug() << "slotDeletePlaylist() row:" << m_lastRightClickedIndex.data();
    if (!m_lastRightClickedIndex.isValid()) {
        return;
    }

    int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (playlistId == kInvalidPlaylistId) {
        return;
    }

    bool locked = m_playlistDao.isPlaylistLocked(playlistId);
    if (locked) {
        qDebug() << "Skipping playlist deletion because playlist" << playlistId << "is locked.";
        return;
    }

    QMessageBox::StandardButton btn = QMessageBox::question(nullptr,
            tr("Confirm Deletion"),
            tr("Do you really want to delete playlist <b>%1</b>?")
                    .arg(m_playlistDao.getPlaylistName(playlistId)),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
    if (btn == QMessageBox::No) {
        return;
    }

    m_playlistDao.deletePlaylist(playlistId);
}

void BasePlaylistFeature::slotImportPlaylist() {
    //qDebug() << "slotImportPlaylist() row:" << m_lastRightClickedIndex.data();
    const QString playlistFile = getPlaylistFile();
    if (playlistFile.isEmpty()) {
        return;
    }
    int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (playlistId == kInvalidPlaylistId) {
        return;
    }

    // Update the import/export playlist directory
    QFileInfo fileDirectory(playlistFile);
    m_pConfig->set(kConfigKeyLastImportExportPlaylistDirectory,
            ConfigValue(fileDirectory.absoluteDir().canonicalPath()));

    slotImportPlaylistFile(playlistFile, playlistId);
}

void BasePlaylistFeature::slotImportPlaylistFile(const QString& playlistFile,
        int playlistId) {
    if (playlistFile.isEmpty()) {
        return;
    }
    // The user has picked a new directory via a file dialog. This means the
    // system sandboxer (if we are sandboxed) has granted us permission to this
    // folder. We don't need access to this file on a regular basis so we do not
    // register a security bookmark.

    // Create a temporary PlaylistTableModel for the Playlist the entries shall be imported to.
    // This is used as a proxy object to write to the database.
    // We cannot use m_pPlaylistTableModel since it might have another playlist selected which
    // is not the playlist that received the right-click.
    std::unique_ptr<PlaylistTableModel> pPlaylistTableModel =
            std::make_unique<PlaylistTableModel>(this,
                    m_pLibrary->trackCollectionManager(),
                    "mixxx.db.model.playlist_export");
    pPlaylistTableModel->selectPlaylist(playlistId);
    pPlaylistTableModel->setSort(
            pPlaylistTableModel->fieldIndex(
                    ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION),
            Qt::AscendingOrder);
    pPlaylistTableModel->select();

    QList<QString> locations = Parser::parse(playlistFile);
    // Iterate over the List that holds locations of playlist entries
    pPlaylistTableModel->addTracks(QModelIndex(), locations);
}

void BasePlaylistFeature::slotCreateImportPlaylist() {
    // Get file to read
    const QStringList playlistFiles = LibraryFeature::getPlaylistFiles();
    if (playlistFiles.isEmpty()) {
        return;
    }

    // Set last import directory
    QFileInfo fileDirectory(playlistFiles.first());
    m_pConfig->set(kConfigKeyLastImportExportPlaylistDirectory,
            ConfigValue(fileDirectory.absoluteDir().canonicalPath()));

    int lastPlaylistId = kInvalidPlaylistId;

    // For each selected element create a different playlist.
    for (const QString& playlistFile : playlistFiles) {
        const QFileInfo fileInfo(playlistFile);
        // Get a valid name
        const QString baseName = fileInfo.completeBaseName();
        QString name = baseName;
        // Check if there already is a playlist by that name. If yes, add
        // increasing suffix (1++) until we find an unused name.
        int i = 1;
        while (m_playlistDao.getPlaylistIdFromName(name) != kInvalidPlaylistId) {
            name = baseName + QChar(' ') + QString::number(i);
            ++i;
        }

        lastPlaylistId = m_playlistDao.createPlaylist(name);
        if (lastPlaylistId == kInvalidPlaylistId) {
            QMessageBox::warning(nullptr,
                    tr("Playlist Creation Failed"),
                    tr("An unknown error occurred while creating playlist: ") + name);
            return;
        }

        slotImportPlaylistFile(playlistFile, lastPlaylistId);
    }
    activatePlaylist(lastPlaylistId);
}

void BasePlaylistFeature::slotExportPlaylist() {
    int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (playlistId == kInvalidPlaylistId) {
        return;
    }
    QString playlistName = m_playlistDao.getPlaylistName(playlistId);
    // replace separator character with something generic
    playlistName = playlistName.replace(QDir::separator(), kUnsafeFilenameReplacement);
    qDebug() << "Export playlist" << playlistName;

    QString lastPlaylistDirectory = m_pConfig->getValue(
            kConfigKeyLastImportExportPlaylistDirectory,
            QStandardPaths::writableLocation(QStandardPaths::MusicLocation));

    // Open a dialog to let the user choose the file location for playlist export.
    // The location is set to the last used directory for import/export and the file
    // name to the playlist name.
    const QString fileLocation = getFilePathWithVerifiedExtensionFromFileDialog(
            tr("Export Playlist"),
            lastPlaylistDirectory.append("/").append(playlistName).append(".m3u"),
            tr("M3U Playlist (*.m3u);;M3U8 Playlist (*.m3u8);;"
               "PLS Playlist (*.pls);;Text CSV (*.csv);;Readable Text (*.txt)"),
            tr("M3U Playlist (*.m3u)"));
    // Exit method if the file name is empty because the user cancelled the save dialog.
    if (fileLocation.isEmpty()) {
        return;
    }

    // Update the import/export playlist directory
    QFileInfo fileDirectory(fileLocation);
    m_pConfig->set(kConfigKeyLastImportExportPlaylistDirectory,
            ConfigValue(fileDirectory.absoluteDir().canonicalPath()));

    // The user has picked a new directory via a file dialog. This means the
    // system sandboxer (if we are sandboxed) has granted us permission to this
    // folder. We don't need access to this file on a regular basis so we do not
    // register a security bookmark.

    // Create a new table model since the main one might have an active search.
    // This will only export songs that we think exist on default
    std::unique_ptr<PlaylistTableModel> pPlaylistTableModel =
            std::make_unique<PlaylistTableModel>(this,
                    m_pLibrary->trackCollectionManager(),
                    "mixxx.db.model.playlist_export",
                    m_keepHiddenTracks);

    emit saveModelState();
    pPlaylistTableModel->selectPlaylist(playlistId);
    pPlaylistTableModel->setSort(
            pPlaylistTableModel->fieldIndex(
                    ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION),
            Qt::AscendingOrder);
    pPlaylistTableModel->select();

    // check config if relative paths are desired
    bool useRelativePath = m_pConfig->getValue<bool>(
            kUseRelativePathOnExportConfigKey);

    if (fileLocation.endsWith(".csv", Qt::CaseInsensitive)) {
        ParserCsv::writeCSVFile(fileLocation, pPlaylistTableModel.get(), useRelativePath);
    } else if (fileLocation.endsWith(".txt", Qt::CaseInsensitive)) {
        if (m_playlistDao.getHiddenType(pPlaylistTableModel->getPlaylist()) ==
                PlaylistDAO::PLHT_SET_LOG) {
            ParserCsv::writeReadableTextFile(fileLocation, pPlaylistTableModel.get(), true);
        } else {
            ParserCsv::writeReadableTextFile(fileLocation, pPlaylistTableModel.get(), false);
        }
    } else {
        // Create and populate a list of files of the playlist
        QList<QString> playlistItems;
        int rows = pPlaylistTableModel->rowCount();
        for (int i = 0; i < rows; ++i) {
            QModelIndex index = pPlaylistTableModel->index(i, 0);
            playlistItems << pPlaylistTableModel->getTrackLocation(index);
        }
        exportPlaylistItemsIntoFile(
                fileLocation,
                playlistItems,
                useRelativePath);
    }
}

void BasePlaylistFeature::slotExportTrackFiles() {
    int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (playlistId == kInvalidPlaylistId) {
        return;
    }
    std::unique_ptr<PlaylistTableModel> pPlaylistTableModel =
            std::make_unique<PlaylistTableModel>(this,
                    m_pLibrary->trackCollectionManager(),
                    "mixxx.db.model.playlist_export");

    emit saveModelState();
    pPlaylistTableModel->selectPlaylist(playlistId);
    pPlaylistTableModel->setSort(pPlaylistTableModel->fieldIndex(
                                         ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION),
            Qt::AscendingOrder);
    pPlaylistTableModel->select();

    int rows = pPlaylistTableModel->rowCount();
    TrackPointerList tracks;
    for (int i = 0; i < rows; ++i) {
        QModelIndex index = pPlaylistTableModel->index(i, 0);
        auto pTrack = pPlaylistTableModel->getTrack(index);
        VERIFY_OR_DEBUG_ASSERT(pTrack != nullptr) {
            continue;
        }
        tracks.push_back(pTrack);
    }

    if (tracks.isEmpty()) {
        return;
    }

    TrackExportWizard track_export(nullptr, m_pConfig, tracks);
    track_export.exportTracks();
}

void BasePlaylistFeature::slotAddToAutoDJ() {
    //qDebug() << "slotAddToAutoDJ() row:" << m_lastRightClickedIndex.data();
    addToAutoDJ(PlaylistDAO::AutoDJSendLoc::BOTTOM);
}

void BasePlaylistFeature::slotAddToAutoDJTop() {
    //qDebug() << "slotAddToAutoDJTop() row:" << m_lastRightClickedIndex.data();
    addToAutoDJ(PlaylistDAO::AutoDJSendLoc::TOP);
}

void BasePlaylistFeature::slotAddToAutoDJReplace() {
    //qDebug() << "slotAddToAutoDJReplace() row:" << m_lastRightClickedIndex.data();
    addToAutoDJ(PlaylistDAO::AutoDJSendLoc::REPLACE);
}

void BasePlaylistFeature::addToAutoDJ(PlaylistDAO::AutoDJSendLoc loc) {
    //qDebug() << "slotAddToAutoDJ() row:" << m_lastRightClickedIndex.data();
    if (m_lastRightClickedIndex.isValid()) {
        int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
        if (playlistId >= 0) {
            // Insert this playlist
            m_playlistDao.addPlaylistToAutoDJQueue(playlistId, loc);
        }
    }
}

void BasePlaylistFeature::slotAnalyzePlaylist() {
    if (m_lastRightClickedIndex.isValid()) {
        int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
        if (playlistId >= 0) {
            const QList<TrackId> ids = m_playlistDao.getTrackIds(playlistId);
            QList<AnalyzerScheduledTrack> tracks;
            for (auto id : ids) {
                tracks.append(id);
            }
            emit analyzeTracks(tracks);
        }
    }
}

TreeItemModel* BasePlaylistFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void BasePlaylistFeature::bindLibraryWidget(WLibrary* pLibraryWidget,
        KeyboardEventFilter* pKeyboard) {
    Q_UNUSED(pKeyboard);
    WLibraryTextBrowser* pEdit = new WLibraryTextBrowser(pLibraryWidget);
    pEdit->setHtml(getRootViewHtml());
    pEdit->setOpenLinks(false);
    connect(pEdit,
            &WLibraryTextBrowser::anchorClicked,
            this,
            &BasePlaylistFeature::htmlLinkClicked);
    m_pLibraryWidget = QPointer(pLibraryWidget);
    m_pLibraryWidget->registerView(m_rootViewName, pEdit);
}

void BasePlaylistFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    // store the sidebar widget pointer for later use in onRightClickChild
    DEBUG_ASSERT(!m_pSidebarWidget);
    m_pSidebarWidget = pSidebarWidget;
}

void BasePlaylistFeature::htmlLinkClicked(const QUrl& link) {
    if (QString(link.path()) == "create") {
        slotCreatePlaylist();
    } else {
        qDebug() << "Unknown playlist link clicked" << link.path();
    }
}

QString BasePlaylistFeature::fetchPlaylistLabel(int playlistId) {
    // This queries the temporary id/count/duration table that was has been created
    // by the features' createPlaylistLabels() (updated each time playlists are added/removed)
    QSqlDatabase database =
            m_pLibrary->trackCollectionManager()->internalCollection()->database();
    VERIFY_OR_DEBUG_ASSERT(database.tables(QSql::Views).contains(m_countsDurationTableName)) {
        qWarning() << "BasePlaylistFeature: view" << m_countsDurationTableName
                   << "does not exist! Can't fetch label for playlist" << playlistId;
        return QString();
    }
    QSqlTableModel playlistTableModel(this, database);
    playlistTableModel.setTable(m_countsDurationTableName);
    const QString filter = "id=" + QString::number(playlistId);
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
        QString name =
                playlistTableModel.data(playlistTableModel.index(0, nameColumn))
                        .toString();
        int count = playlistTableModel
                            .data(playlistTableModel.index(0, countColumn))
                            .toInt();
        int duration =
                playlistTableModel
                        .data(playlistTableModel.index(0, durationColumn))
                        .toInt();
        return createPlaylistLabel(name, count, duration);
    }
    return QString();
}

void BasePlaylistFeature::updateChildModel(const QSet<int>& playlistIds) {
    // qDebug() << "BasePlaylistFeature::updateChildModel() for"
    //          << playlistIds.count() << "playlist(s)";
    if (playlistIds.isEmpty()) {
        return;
    }

    int id = kInvalidPlaylistId;
    QString label;
    bool ok = false;

    for (int row = 0; row < m_pSidebarModel->rowCount(); ++row) {
        QModelIndex index = m_pSidebarModel->index(row, 0);
        TreeItem* pTreeItem = m_pSidebarModel->getItem(index);
        DEBUG_ASSERT(pTreeItem != nullptr);
        if (pTreeItem->hasChildren()) {
            for (TreeItem* pChild : pTreeItem->children()) {
                id = pChild->getData().toInt(&ok);
                if (ok && id != kInvalidPlaylistId && playlistIds.contains(id)) {
                    label = fetchPlaylistLabel(id);
                    pChild->setLabel(label);
                    decorateChild(pChild, id);
                    markTreeItem(pChild);
                }
            }
        } else {
            id = pTreeItem->getData().toInt(&ok);
            if (ok && id != kInvalidPlaylistId && playlistIds.contains(id)) {
                label = fetchPlaylistLabel(id);
                pTreeItem->setLabel(label);
                decorateChild(pTreeItem, id);
                markTreeItem(pTreeItem);
            }
        }
    }
    m_pSidebarModel->triggerRepaint();
}

/// Clears the child model dynamically, but the invisible root item remains
void BasePlaylistFeature::clearChildModel() {
    m_lastClickedIndex = QModelIndex();
    m_lastRightClickedIndex = QModelIndex();
    m_pSidebarModel->removeRows(0, m_pSidebarModel->rowCount());
}

QModelIndex BasePlaylistFeature::indexFromPlaylistId(int playlistId) {
    QVariant variantId = QVariant(playlistId);
    QModelIndexList results = m_pSidebarModel->match(
            m_pSidebarModel->getRootIndex(),
            TreeItemModel::kDataRole,
            variantId,
            1,
            Qt::MatchWrap | Qt::MatchExactly | Qt::MatchRecursive);
    if (!results.isEmpty()) {
        return results.front();
    }
    return QModelIndex();
}

bool BasePlaylistFeature::isChildIndexSelectedInSidebar(const QModelIndex& index) {
    return m_pSidebarWidget && m_pSidebarWidget->isChildIndexSelected(index);
};

void BasePlaylistFeature::slotTrackSelected(TrackId trackId) {
    m_selectedTrackId = trackId;
    m_playlistDao.getPlaylistsTrackIsIn(m_selectedTrackId, &m_playlistIdsOfSelectedTrack);

    for (int row = 0; row < m_pSidebarModel->rowCount(); ++row) {
        QModelIndex index = m_pSidebarModel->index(row, 0);
        TreeItem* pTreeItem = m_pSidebarModel->getItem(index);
        DEBUG_ASSERT(pTreeItem != nullptr);
        markTreeItem(pTreeItem);
    }

    m_pSidebarModel->triggerRepaint();
}

void BasePlaylistFeature::markTreeItem(TreeItem* pTreeItem) {
    bool ok;
    int playlistId = pTreeItem->getData().toInt(&ok);
    if (ok) {
        bool shouldBold = m_playlistIdsOfSelectedTrack.contains(playlistId);
        pTreeItem->setBold(shouldBold);
        if (shouldBold && pTreeItem->hasParent()) {
            TreeItem* item = pTreeItem;
            // extra parents, because -Werror=parentheses
            while ((item = item->parent())) {
                item->setBold(true);
            }
        }
    }
    if (pTreeItem->hasChildren()) {
        QList<TreeItem*> children = pTreeItem->children();

        for (int i = 0; i < children.size(); i++) {
            markTreeItem(children.at(i));
        }
    }
}

QString BasePlaylistFeature::createPlaylistLabel(const QString& name,
        int count,
        int duration) const {
    // Show duration only if playlist has tracks
    if (count > 0) {
        return QStringLiteral("%1 (%2) %3")
                .arg(name,
                        QString::number(count),
                        mixxx::Duration::formatTime(
                                duration, mixxx::Duration::Precision::SECONDS));
    } else {
        return QStringLiteral("%1 (%2)").arg(name,
                QString::number(count));
    }
}

void BasePlaylistFeature::slotResetSelectedTrack() {
    slotTrackSelected(TrackId{});
}
