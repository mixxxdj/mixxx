#include "library/trackset/playlist/basegroupedplaylistsfeature.h"

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
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/playlist/groupedplayliststablemodel.h"
#include "library/treeitem.h"
#include "library/treeitemmodel.h"
#include "moc_basegroupedplaylistsfeature.cpp"
#include "track/track.h"
#include "track/trackid.h"
#include "util/assert.h"
#include "util/defs.h"
#include "util/file.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextbrowser.h"

namespace {
constexpr bool sDebug = false;
constexpr QChar kUnsafeFilenameReplacement = '-';
const ConfigKey kConfigKeyLastImportExportPlaylistDirectory(
        "[Library]", "LastImportExportPlaylistDirectory");

QString formatLabel(
        const QString& playlistname, int playlistCount, int playlistDurationSeconds) {
    return QStringLiteral("%1 (%2) %3")
            .arg(playlistname,
                    QString::number(playlistCount),
                    mixxx::Duration::formatTime(playlistDurationSeconds,
                            mixxx::Duration::Precision::SECONDS));
}

} // anonymous namespace

using namespace mixxx::library::prefs;

BaseGroupedPlaylistsFeature::BaseGroupedPlaylistsFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig,
        GroupedPlaylistsTableModel* pModel,
        const QString& rootViewName,
        const QString& iconName,
        const QString& countsDurationTableName,
        bool keepHiddenTracks)
        : BaseTrackSetFeature(pLibrary, pConfig, rootViewName, iconName),
          m_lockedPlaylistIcon(":/images/library/ic_library_locked_tracklist.svg"),
          m_playlistDao(pLibrary->trackCollectionManager()
                          ->internalCollection()
                          ->getPlaylistDAO()),
          m_pGroupedPlaylistsTableModel(pModel),
          m_countsDurationTableName(countsDurationTableName),
          m_pTrackCollection(pLibrary->trackCollectionManager()->internalCollection()),
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
            &BaseGroupedPlaylistsFeature::slotResetSelectedTrack);
}

void BaseGroupedPlaylistsFeature::initActions() {
    m_pCreateGroupedPlaylistsAction = new QAction(tr("Create New Playlist"), this);
    connect(m_pCreateGroupedPlaylistsAction,
            &QAction::triggered,
            this,
            &BaseGroupedPlaylistsFeature::slotCreatePlaylist);

    m_pRenameGroupedPlaylistsAction = new QAction(tr("Rename"), this);
    m_pRenameGroupedPlaylistsAction->setShortcut(kRenameSidebarItemShortcutKey);
    connect(m_pRenameGroupedPlaylistsAction,
            &QAction::triggered,
            this,
            &BaseGroupedPlaylistsFeature::slotRenamePlaylist);
    m_pDuplicateGroupedPlaylistsAction = new QAction(tr("Duplicate"), this);
    connect(m_pDuplicateGroupedPlaylistsAction,
            &QAction::triggered,
            this,
            &BaseGroupedPlaylistsFeature::slotDuplicatePlaylist);
    m_pDeleteGroupedPlaylistsAction = new QAction(tr("Remove"), this);
    const auto removeKeySequence =
            // TODO(XXX): Qt6 replace enum | with QKeyCombination
            QKeySequence(static_cast<int>(kHideRemoveShortcutModifier) |
                    kHideRemoveShortcutKey);
    m_pDeleteGroupedPlaylistsAction->setShortcut(removeKeySequence);
    connect(m_pDeleteGroupedPlaylistsAction,
            &QAction::triggered,
            this,
            &BaseGroupedPlaylistsFeature::slotDeletePlaylist);
    m_pLockGroupedPlaylistsAction = new QAction(tr("Lock"), this);
    connect(m_pLockGroupedPlaylistsAction,
            &QAction::triggered,
            this,
            &BaseGroupedPlaylistsFeature::slotTogglePlaylistLock);

    m_pAddToAutoDJAction = new QAction(tr("Add to Auto DJ Queue (bottom)"), this);
    connect(m_pAddToAutoDJAction,
            &QAction::triggered,
            this,
            &BaseGroupedPlaylistsFeature::slotAddToAutoDJ);
    m_pAddToAutoDJTopAction = new QAction(tr("Add to Auto DJ Queue (top)"), this);
    connect(m_pAddToAutoDJTopAction,
            &QAction::triggered,
            this,
            &BaseGroupedPlaylistsFeature::slotAddToAutoDJTop);
    m_pAddToAutoDJReplaceAction = new QAction(tr("Add to Auto DJ Queue (replace)"), this);
    connect(m_pAddToAutoDJReplaceAction,
            &QAction::triggered,
            this,
            &BaseGroupedPlaylistsFeature::slotAddToAutoDJReplace);

    m_pAnalyzeGroupedPlaylistsAction = new QAction(tr("Analyze entire Playlist"), this);
    connect(m_pAnalyzeGroupedPlaylistsAction,
            &QAction::triggered,
            this,
            &BaseGroupedPlaylistsFeature::slotAnalyzePlaylist);

    m_pImportGroupedPlaylistsAction = new QAction(tr("Import Playlist"), this);
    connect(m_pImportGroupedPlaylistsAction,
            &QAction::triggered,
            this,
            &BaseGroupedPlaylistsFeature::slotImportPlaylist);
    m_pCreateImportGroupedPlaylistsAction = new QAction(tr("Import Playlist"), this);
    connect(m_pCreateImportGroupedPlaylistsAction,
            &QAction::triggered,
            this,
            &BaseGroupedPlaylistsFeature::slotCreateImportPlaylist);
    m_pExportGroupedPlaylistsAction = new QAction(tr("Export Playlist"), this);
    connect(m_pExportGroupedPlaylistsAction,
            &QAction::triggered,
            this,
            &BaseGroupedPlaylistsFeature::slotExportPlaylist);
    m_pExportTrackFilesAction = new QAction(tr("Export Track Files"), this);
    connect(m_pExportTrackFilesAction,
            &QAction::triggered,
            this,
            &BaseGroupedPlaylistsFeature::slotExportTrackFiles);
}

void BaseGroupedPlaylistsFeature::connectPlaylistDAO() {
    connect(&m_playlistDao,
            &PlaylistDAO::added,
            this,
            &BaseGroupedPlaylistsFeature::slotPlaylistTableChangedAndScrollTo);
    connect(&m_playlistDao,
            &PlaylistDAO::lockChanged,
            this,
            &BaseGroupedPlaylistsFeature::slotPlaylistContentOrLockChanged);
    connect(&m_playlistDao,
            &PlaylistDAO::deleted,
            this,
            &BaseGroupedPlaylistsFeature::slotPlaylistTableChanged);
    connect(&m_playlistDao,
            &PlaylistDAO::playlistContentChanged,
            this,
            &BaseGroupedPlaylistsFeature::slotPlaylistContentOrLockChanged);
    connect(&m_playlistDao,
            &PlaylistDAO::renamed,
            this,
            // In "History" just the item is renamed, while in "Playlists" the
            // entire sidebar model is rebuilt to re-sort items by name
            &BaseGroupedPlaylistsFeature::slotPlaylistTableRenamed);
}

int BaseGroupedPlaylistsFeature::playlistIdFromIndex(const QModelIndex& index) const {
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

void BaseGroupedPlaylistsFeature::selectPlaylistInSidebar(int playlistId, bool select) {
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

void BaseGroupedPlaylistsFeature::activatePlaylist(int playlistId) {
    QModelIndex index = indexFromPlaylistId(playlistId);
    if (sDebug) {
        qDebug() << "[BaseGroupedPlaylistsFeature] -> activatePlaylist() "
                 << "playlistId: " << playlistId
                 << "index: " << index;
    }
    m_lastClickedIndex = index;
    m_lastRightClickedIndex = QModelIndex();
    emit saveModelState();
    m_pGroupedPlaylistsTableModel->selectPlaylist(playlistId);
    emit showTrackModel(m_pGroupedPlaylistsTableModel);
    emit enableCoverArtDisplay(true);
    // Update selection
    emit featureSelect(this, m_lastClickedIndex);
}

void BaseGroupedPlaylistsFeature::renameItem(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
    if (sDebug) {
        qDebug() << "[BaseGroupedPlaylistsFeature] toggle of renameItem "
                 << index;
        qDebug() << "[BaseGroupedPlaylistsFeature] renameItem : "
                    "m_lastRightClickedIndex "
                 << m_lastRightClickedIndex;
    }
    slotRenamePlaylist();
}

void BaseGroupedPlaylistsFeature::slotRenamePlaylist() {
    int playlistId(playlistIdFromIndex(m_lastRightClickedIndex));
    if (sDebug) {
        qDebug() << "[BaseGroupedPlaylistsFeature] renameItem : "
                    "m_lastRightClickedIndex "
                 << m_lastRightClickedIndex;
        qDebug() << "[BaseGroupedPlaylistsFeature] toggle of slotRenamePlaylist "
                    "playlistId "
                 << playlistId;
    }
    if (playlistId == kInvalidPlaylistId) {
        return;
    }
    QString oldName = m_playlistDao.getPlaylistName(playlistId);
    bool locked = m_playlistDao.isPlaylistLocked(playlistId);

    if (locked) {
        if (sDebug) {
            qDebug() << "Skipping playlist rename because playlist"
                     << playlistId
                     << oldName << "is locked.";
        }
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
    slotPlaylistTableChanged(playlistId);
    activatePlaylist(playlistId);
}

void BaseGroupedPlaylistsFeature::slotDuplicatePlaylist() {
    int oldPlaylistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (sDebug) {
        qDebug() << "[BaseGroupedPlaylistsFeature] toggle of slotDuplicatePlaylist "
                 << oldPlaylistId;
    }
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
        slotPlaylistTableChanged(newPlaylistId);
        activatePlaylist(newPlaylistId);
    }
}

void BaseGroupedPlaylistsFeature::slotTogglePlaylistLock() {
    int playlistId(playlistIdFromIndex(m_lastRightClickedIndex));
    if (sDebug) {
        qDebug() << "[BaseGroupedPlaylistsFeature] toggle of "
                    "slotTogglePlaylistLock m_lastRightClickedIndex "
                 << m_lastRightClickedIndex;
        qDebug() << "[BaseGroupedPlaylistsFeature] toggle of "
                    "slotTogglePlaylistLock "
                 << playlistId;
    }
    if (playlistId == kInvalidPlaylistId) {
        return;
    }
    bool locked = !m_playlistDao.isPlaylistLocked(playlistId);

    if (!m_playlistDao.setPlaylistLocked(playlistId, locked)) {
        if (sDebug) {
            qDebug() << "Failed to toggle lock of playlistId "
                     << playlistId;
        }
    }
    slotPlaylistTableChanged(playlistId);
    activatePlaylist(playlistId);
}

void BaseGroupedPlaylistsFeature::slotCreatePlaylist() {
    if (sDebug) {
        qDebug() << "[BaseGroupedPlaylistsFeature] toggle of slotCreatePlaylist ";
    }
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
    slotPlaylistTableChanged(playlistId);
    activatePlaylist(playlistId);
}

/// Returns a playlist that is a sibling inside the same parent
/// as the start index
int BaseGroupedPlaylistsFeature::getSiblingPlaylistIdOf(QModelIndex& start) {
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

void BaseGroupedPlaylistsFeature::deleteItem(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
    if (sDebug) {
        qDebug() << "[BaseGroupedPlaylistsFeature] toggle of deleteItem "
                 << index;
    }
    slotDeletePlaylist();
}

void BaseGroupedPlaylistsFeature::slotDeletePlaylist() {
    if (sDebug) {
        qDebug() << "[BaseGroupedPlaylistsFeature] -> slotDeletePlaylist() row:"
                 << m_lastRightClickedIndex.data();
    }
    if (!m_lastRightClickedIndex.isValid()) {
        return;
    }

    int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (sDebug) {
        qDebug() << "[BaseGroupedPlaylistsFeature] toggle of slotDeletePlaylist "
                 << playlistId;
    }
    if (playlistId == kInvalidPlaylistId) {
        return;
    }
    bool locked = m_playlistDao.isPlaylistLocked(playlistId);
    if (locked) {
        if (sDebug) {
            qDebug() << "Skipping playlist deletion because playlist"
                     << playlistId << "is locked.";
        }
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
    slotPlaylistTableChanged(kInvalidPlaylistId);
}

void BaseGroupedPlaylistsFeature::slotImportPlaylist() {
    // qDebug() << "slotImportPlaylist() row:" << m_lastRightClickedIndex.data();
    const QString playlistFile = getPlaylistFile();
    if (playlistFile.isEmpty()) {
        return;
    }
    int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
    qDebug() << "[BaseGroupedPlaylistsFeature] toggle of slotImportPlaylist " << playlistId;
    if (playlistId == kInvalidPlaylistId) {
        return;
    }

    // Update the import/export playlist directory
    QString fileDirectory(playlistFile);
    fileDirectory.truncate(playlistFile.lastIndexOf("/"));
    m_pConfig->set(kConfigKeyLastImportExportPlaylistDirectory,
            ConfigValue(fileDirectory));

    slotImportPlaylistFile(playlistFile, playlistId);
    slotPlaylistTableChanged(playlistId);
}

void BaseGroupedPlaylistsFeature::slotImportPlaylistFile(const QString& playlistFile,
        int playlistId) {
    if (sDebug) {
        qDebug() << "[BaseGroupedPlaylistsFeature] toggle of slotImportPlaylistFile "
                 << playlistId;
    }
    if (playlistFile.isEmpty()) {
        return;
    }
    // The user has picked a new directory via a file dialog. This means the
    // system sandboxer (if we are sandboxed) has granted us permission to this
    // folder. We don't need access to this file on a regular basis so we do not
    // register a security bookmark.

    // Create a temporary GroupedPlaylistsTableModel for the Playlist the
    // entries shall be imported to. This is used as a proxy object to write to
    // the database. We cannot use m_pGroupedPlaylistsTableModel since it might
    // have another playlist selected which is not the playlist that received
    // the right-click.
    std::unique_ptr<GroupedPlaylistsTableModel> pGroupedPlaylistsTableModel =
            std::make_unique<GroupedPlaylistsTableModel>(this,
                    m_pLibrary->trackCollectionManager(),
                    m_pConfig,
                    "mixxx.db.model.playlist_export");
    pGroupedPlaylistsTableModel->selectPlaylist(playlistId);
    pGroupedPlaylistsTableModel->setSort(
            pGroupedPlaylistsTableModel->fieldIndex(
                    ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION),
            Qt::AscendingOrder);
    pGroupedPlaylistsTableModel->select();

    QList<QString> locations = Parser::parse(playlistFile);
    // Iterate over the List that holds locations of playlist entries
    pGroupedPlaylistsTableModel->addTracks(QModelIndex(), locations);
}

void BaseGroupedPlaylistsFeature::slotCreateImportPlaylist() {
    if (sDebug) {
        qDebug() << "[BaseGroupedPlaylistsFeature] toggle of slotCreateImportPlaylist ";
    }
    // Get file to read
    const QStringList playlistFiles = LibraryFeature::getPlaylistFiles();
    if (playlistFiles.isEmpty()) {
        return;
    }

    // Set last import directory
    QString fileDirectory(playlistFiles.first());
    fileDirectory.truncate(playlistFiles.first().lastIndexOf("/"));
    m_pConfig->set(kConfigKeyLastImportExportPlaylistDirectory,
            ConfigValue(fileDirectory));

    int lastPlaylistId = kInvalidPlaylistId;

    // For each selected element create a different playlist.
    for (const QString& playlistFile : playlistFiles) {
        const QFileInfo fileInfo(playlistFile);
        // Get a valid name
        const QString baseName = fileInfo.baseName();
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
    slotPlaylistTableChanged(lastPlaylistId);
    activatePlaylist(lastPlaylistId);
}

void BaseGroupedPlaylistsFeature::slotExportPlaylist() {
    int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (sDebug) {
        qDebug() << "[BaseGroupedPlaylistsFeature] toggle of slotExportPlaylist "
                 << playlistId;
    }
    if (playlistId == kInvalidPlaylistId) {
        return;
    }
    QString playlistName = m_playlistDao.getPlaylistName(playlistId);
    // replace separator character with something generic
    playlistName = playlistName.replace(QDir::separator(), kUnsafeFilenameReplacement);
    if (sDebug) {
        qDebug() << "Export playlist" << playlistName;
    }

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
    QString fileDirectory(fileLocation);
    fileDirectory.truncate(fileLocation.lastIndexOf("/"));
    m_pConfig->set(kConfigKeyLastImportExportPlaylistDirectory,
            ConfigValue(fileDirectory));

    // The user has picked a new directory via a file dialog. This means the
    // system sandboxer (if we are sandboxed) has granted us permission to this
    // folder. We don't need access to this file on a regular basis so we do not
    // register a security bookmark.

    // Create a new table model since the main one might have an active search.
    // This will only export songs that we think exist on default
    std::unique_ptr<GroupedPlaylistsTableModel> pGroupedPlaylistsTableModel =
            std::make_unique<GroupedPlaylistsTableModel>(this,
                    m_pLibrary->trackCollectionManager(),
                    m_pConfig,
                    "mixxx.db.model.playlist_export",
                    m_keepHiddenTracks);

    emit saveModelState();
    pGroupedPlaylistsTableModel->selectPlaylist(playlistId);
    pGroupedPlaylistsTableModel->setSort(
            pGroupedPlaylistsTableModel->fieldIndex(
                    ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION),
            Qt::AscendingOrder);
    pGroupedPlaylistsTableModel->select();

    // check config if relative paths are desired
    bool useRelativePath = m_pConfig->getValue<bool>(
            kUseRelativePathOnExportConfigKey);

    if (fileLocation.endsWith(".csv", Qt::CaseInsensitive)) {
        ParserCsv::writeCSVFile(fileLocation, pGroupedPlaylistsTableModel.get(), useRelativePath);
    } else if (fileLocation.endsWith(".txt", Qt::CaseInsensitive)) {
        if (m_playlistDao.getHiddenType(pGroupedPlaylistsTableModel->getPlaylist()) ==
                PlaylistDAO::PLHT_SET_LOG) {
            ParserCsv::writeReadableTextFile(fileLocation, pGroupedPlaylistsTableModel.get(), true);
        } else {
            ParserCsv::writeReadableTextFile(
                    fileLocation, pGroupedPlaylistsTableModel.get(), false);
        }
    } else {
        // Create and populate a list of files of the playlist
        QList<QString> playlistItems;
        int rows = pGroupedPlaylistsTableModel->rowCount();
        for (int i = 0; i < rows; ++i) {
            QModelIndex index = pGroupedPlaylistsTableModel->index(i, 0);
            playlistItems << pGroupedPlaylistsTableModel->getTrackLocation(index);
        }
        exportPlaylistItemsIntoFile(
                fileLocation,
                playlistItems,
                useRelativePath);
    }
}

void BaseGroupedPlaylistsFeature::slotExportTrackFiles() {
    int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (sDebug) {
        qDebug() << "[BaseGroupedPlaylistsFeature] toggle of slotExportTrackFiles "
                 << playlistId;
    }
    if (playlistId == kInvalidPlaylistId) {
        return;
    }
    std::unique_ptr<GroupedPlaylistsTableModel> pGroupedPlaylistsTableModel =
            std::make_unique<GroupedPlaylistsTableModel>(this,
                    m_pLibrary->trackCollectionManager(),
                    m_pConfig,
                    "mixxx.db.model.playlist_export");

    emit saveModelState();
    pGroupedPlaylistsTableModel->selectPlaylist(playlistId);
    pGroupedPlaylistsTableModel->setSort(pGroupedPlaylistsTableModel->fieldIndex(
                                                 ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION),
            Qt::AscendingOrder);
    pGroupedPlaylistsTableModel->select();

    int rows = pGroupedPlaylistsTableModel->rowCount();
    TrackPointerList tracks;
    for (int i = 0; i < rows; ++i) {
        QModelIndex index = pGroupedPlaylistsTableModel->index(i, 0);
        auto pTrack = pGroupedPlaylistsTableModel->getTrack(index);
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

void BaseGroupedPlaylistsFeature::slotAddToAutoDJ() {
    // qDebug() << "slotAddToAutoDJ() row:" << m_lastRightClickedIndex.data();
    addToAutoDJ(PlaylistDAO::AutoDJSendLoc::BOTTOM);
}

void BaseGroupedPlaylistsFeature::slotAddToAutoDJTop() {
    // qDebug() << "slotAddToAutoDJTop() row:" << m_lastRightClickedIndex.data();
    addToAutoDJ(PlaylistDAO::AutoDJSendLoc::TOP);
}

void BaseGroupedPlaylistsFeature::slotAddToAutoDJReplace() {
    // qDebug() << "slotAddToAutoDJReplace() row:" << m_lastRightClickedIndex.data();
    addToAutoDJ(PlaylistDAO::AutoDJSendLoc::REPLACE);
}

void BaseGroupedPlaylistsFeature::addToAutoDJ(PlaylistDAO::AutoDJSendLoc loc) {
    // qDebug() << "slotAddToAutoDJ() row:" << m_lastRightClickedIndex.data();
    if (m_lastRightClickedIndex.isValid()) {
        int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
        if (playlistId >= 0) {
            // Insert this playlist
            m_playlistDao.addPlaylistToAutoDJQueue(playlistId, loc);
        }
    }
}

void BaseGroupedPlaylistsFeature::slotAnalyzePlaylist() {
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

TreeItemModel* BaseGroupedPlaylistsFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void BaseGroupedPlaylistsFeature::bindLibraryWidget(WLibrary* pLibraryWidget,
        KeyboardEventFilter* pKeyboard) {
    Q_UNUSED(pKeyboard);
    WLibraryTextBrowser* pEdit = new WLibraryTextBrowser(pLibraryWidget);
    pEdit->setHtml(getRootViewHtml());
    pEdit->setOpenLinks(false);
    connect(pEdit,
            &WLibraryTextBrowser::anchorClicked,
            this,
            &BaseGroupedPlaylistsFeature::htmlLinkClicked);
    m_pLibraryWidget = QPointer(pLibraryWidget);
    m_pLibraryWidget->registerView(m_rootViewName, pEdit);
}

void BaseGroupedPlaylistsFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    // store the sidebar widget pointer for later use in onRightClickChild
    DEBUG_ASSERT(!m_pSidebarWidget);
    m_pSidebarWidget = pSidebarWidget;
}

void BaseGroupedPlaylistsFeature::htmlLinkClicked(const QUrl& link) {
    if (QString(link.path()) == "create") {
        slotCreatePlaylist();
    } else {
        if (sDebug) {
            qDebug() << "[BaseGroupedPlaylistsFeature] Unknown playlist link clicked"
                     << link.path();
        }
    }
}

QString BaseGroupedPlaylistsFeature::fetchPlaylistLabel(int playlistId) {
    // This queries the temporary id/count/duration table that was has been created
    // by the features' createPlaylistLabels() (updated each time playlists are added/removed)
    QSqlDatabase database =
            m_pLibrary->trackCollectionManager()->internalCollection()->database();
    VERIFY_OR_DEBUG_ASSERT(database.tables(QSql::Views).contains(m_countsDurationTableName)) {
        qWarning() << "[BaseGroupedPlaylistsFeature] view"
                   << m_countsDurationTableName
                   << "does not exist! Can't fetch label for playlist"
                   << playlistId;
        return QString();
    }
    QSqlTableModel groupedPlaylistsTableModel(this, database);
    groupedPlaylistsTableModel.setTable(m_countsDurationTableName);
    const QString filter = "id=" + QString::number(playlistId);
    groupedPlaylistsTableModel.setFilter(filter);
    groupedPlaylistsTableModel.select();
    while (groupedPlaylistsTableModel.canFetchMore()) {
        groupedPlaylistsTableModel.fetchMore();
    }
    QSqlRecord record = groupedPlaylistsTableModel.record();
    int nameColumn = record.indexOf("name");
    int countColumn = record.indexOf("count");
    int durationColumn = record.indexOf("durationSeconds");

    DEBUG_ASSERT(groupedPlaylistsTableModel.rowCount() <= 1);
    if (groupedPlaylistsTableModel.rowCount() > 0) {
        QString name =
                groupedPlaylistsTableModel.data(groupedPlaylistsTableModel.index(0, nameColumn))
                        .toString();
        int count = groupedPlaylistsTableModel
                            .data(groupedPlaylistsTableModel.index(0, countColumn))
                            .toInt();
        int duration =
                groupedPlaylistsTableModel
                        .data(groupedPlaylistsTableModel.index(0, durationColumn))
                        .toInt();
        return createPlaylistLabel(name, count, duration);
    }
    return QString();
}

/// Clears the child model dynamically, but the invisible root item remains
void BaseGroupedPlaylistsFeature::clearChildModel() {
    m_lastClickedIndex = QModelIndex();
    m_lastRightClickedIndex = QModelIndex();
    m_pSidebarModel->removeRows(0, m_pSidebarModel->rowCount());
}

QModelIndex BaseGroupedPlaylistsFeature::indexFromPlaylistId(int playlistId) {
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

bool BaseGroupedPlaylistsFeature::isChildIndexSelectedInSidebar(const QModelIndex& index) {
    return m_pSidebarWidget && m_pSidebarWidget->isChildIndexSelected(index);
};

void BaseGroupedPlaylistsFeature::slotTrackSelected(TrackId trackId) {
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

void BaseGroupedPlaylistsFeature::markTreeItem(TreeItem* pTreeItem) {
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

QString BaseGroupedPlaylistsFeature::createPlaylistLabel(const QString& name,
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

void BaseGroupedPlaylistsFeature::slotResetSelectedTrack() {
    slotTrackSelected(TrackId{});
}

///////////////////////////////////////////////////////////
////
//// appended grouped-logic
////
///////////////////////////////////////////////////////////

void BaseGroupedPlaylistsFeature::storePrevSiblingPlaylistId(int playlistId) {
    QModelIndex actIndex = indexFromPlaylistId(playlistId);
    m_prevSiblingPlaylist = playlistId;
    for (int i = (actIndex.row() + 1); i >= (actIndex.row() - 1); i -= 2) {
        QModelIndex newIndex = actIndex.sibling(i, actIndex.column());
        if (newIndex.isValid()) {
            TreeItem* pTreeItem = m_pSidebarModel->getItem(newIndex);
            DEBUG_ASSERT(pTreeItem != nullptr);
            if (!pTreeItem->hasChildren()) {
                m_prevSiblingPlaylist = playlistIdFromIndex(newIndex);
            }
        }
    }
}

QModelIndex BaseGroupedPlaylistsFeature::rebuildChildModel(int selectedPlaylistId) {
    if (sDebug) {
        qDebug() << "[GROUPEDPLAYLISTSFEATURE] -> rebuildChildModel()" << selectedPlaylistId;
    }

    QModelIndex previouslySelectedIndex = m_lastRightClickedIndex;

    // remember open/close state of group
    QMap<QString, bool> groupExpandedStates;
    for (int row = 0; row < m_pSidebarModel->rowCount(); ++row) {
        QModelIndex groupIndex = m_pSidebarModel->index(row, 0);
        if (groupIndex.isValid()) {
            TreeItem* pGroupItem = m_pSidebarModel->getItem(groupIndex);
            if (pGroupItem) {
                const QString& groupName = pGroupItem->getLabel();
                groupExpandedStates[groupName] = m_pSidebarWidget->isExpanded(groupIndex);
                if (sDebug) {
                    qDebug() << "[BaseGroupedPlaylistsFeature] Saved open/close state "
                                "for group:"
                             << groupName << "->"
                             << groupExpandedStates[groupName];
                }
            }
        }
    }

    m_lastRightClickedIndex = QModelIndex();
    TreeItem* pRootItem = m_pSidebarModel->getRootItem();
    VERIFY_OR_DEBUG_ASSERT(pRootItem != nullptr) {
        return QModelIndex();
    }
    m_pSidebarModel->removeRows(0, pRootItem->childRows());

    QList<QVariantMap> groupedPlaylists = m_pGroupedPlaylistsTableModel->getGroupedPlaylists();
    const QString& delimiter = m_pConfig->getValue(
            ConfigKey("[Library]", "GroupedPlaylistsVarLengthMask"));

    if (m_pConfig->getValue<int>(ConfigKey("[Library]", "GroupedPlaylistsLength")) == 0) {
        // Fixed prefix length
        QMap<QString, int> groupCounts;
        for (int i = 0; i < groupedPlaylists.size(); ++i) {
            const auto& playlistData = groupedPlaylists[i];
            const QString& groupName = playlistData["group_name"].toString();
            groupCounts[groupName]++;
        }

        QMap<QString, TreeItem*> groupItems;
        std::vector<std::unique_ptr<TreeItem>> modelRows;

        for (int i = 0; i < groupedPlaylists.size(); ++i) {
            const auto& playlistData = groupedPlaylists[i];
            const QString& groupName = playlistData["group_name"].toString();
            const QString& playlistName = playlistData["playlist_name"].toString();
            int playlistId(playlistData["playlist_id"].toInt());
            bool playlistLocked = (playlistData["playlist_locked"] = 1).toBool();
            int playlistCount = playlistData["playlist_count"].toInt();
            int playlistDurationSeconds = playlistData["playlist_durationSeconds"].toInt();

            const QString& displayPlaylistName =
                    formatLabel(playlistName,
                            playlistCount,
                            playlistDurationSeconds);

            if (groupCounts[groupName] > 1) {
                TreeItem* pGroupItem = groupItems.value(groupName, nullptr);
                if (!pGroupItem) {
                    auto newGroup = std::make_unique<TreeItem>(groupName, kInvalidPlaylistId);
                    pGroupItem = newGroup.get();
                    groupItems.insert(groupName, pGroupItem);
                    modelRows.push_back(std::move(newGroup));
                }

                const QString& displayPlaylistName =
                        formatLabel(playlistName.mid(groupName.length()),
                                playlistCount,
                                playlistDurationSeconds);

                if (sDebug) {
                    qDebug() << "[GROUPEDPLAYLISTSFEATURE] -> playlistSummaryName - "
                                "displayPlaylistName = "
                             << displayPlaylistName;
                    qDebug() << "[GROUPEDPLAYLISTSFEATURE] LOCKED = " << playlistLocked;
                }

                TreeItem* pChildItem = pGroupItem->appendChild(
                        displayPlaylistName, playlistId);
                pChildItem->setFullPath(groupName + delimiter + displayPlaylistName);
                if (sDebug) {
                    qDebug() << "[GROUPEDPLAYLISTSFEATURE] Added PlaylistId to group:"
                             << playlistId << "Group:" << groupName;
                }
            } else {
                auto newPlaylist = std::make_unique<TreeItem>(
                        displayPlaylistName, playlistId);
                newPlaylist->setFullPath(displayPlaylistName);

                modelRows.push_back(std::move(newPlaylist));
            }
        }

        m_pSidebarModel->insertTreeItemRows(std::move(modelRows), 0);
        slotTrackSelected(m_selectedTrackId);

    } else {
        // variable group prefix length with mask
        QMap<QString, QList<QVariantMap>> topLevelGroups;
        // Sort groupedPlaylists by lower(group_name)
        std::sort(groupedPlaylists.begin(),
                groupedPlaylists.end(),
                [](const QVariantMap& a, const QVariantMap& b) {
                    QString groupNameA = a["group_name"].toString().toLower();
                    QString groupNameB = b["group_name"].toString().toLower();

                    if (groupNameA == groupNameB) {
                        // If group_name is the same, sort by playlist_name
                        QString playlistNameA =
                                a["playlist_name"].toString().toLower();
                        QString playlistNameB =
                                b["playlist_name"].toString().toLower();
                        return playlistNameA < playlistNameB;
                    }

                    // Otherwise, sort by group_name
                    return groupNameA < groupNameB;
                });
        for (int i = 0; i < groupedPlaylists.size(); ++i) {
            const auto& playlistData = groupedPlaylists[i];
            const QString& groupName = playlistData["group_name"].toString();
            const QString& topGroup = groupName.section(delimiter, 0, 0);
            topLevelGroups[topGroup].append(playlistData);
        }

        if (sDebug) {
            qDebug() << "[GROUPEDPLAYLISTSFEATURE] Top-level groups:";
            for (auto it = topLevelGroups.constBegin(); it != topLevelGroups.constEnd(); ++it) {
                qDebug() << "Group:" << it.key() << "-> Playlists:" << it.value().size();
            }
        }
        // lambda function to build tree
        std::function<void(
                const QString&, const QList<QVariantMap>&, TreeItem*)>
                buildTreeStructure;
        buildTreeStructure = [&](const QString& currentPath,
                                     const QList<QVariantMap>& playlists,
                                     TreeItem* pParentItem) {
            QMap<QString, QList<QVariantMap>> subgroupedPlaylists;

            for (const QVariantMap& playlistData : playlists) {
                const QString& groupName = playlistData["group_name"].toString();

                if (sDebug) {
                    qDebug() << "[GROUPEDPLAYLISTSFEATURE] Processing playlist with "
                                "groupName:"
                             << groupName << "currentPath:" << currentPath;
                }

                if (!groupName.startsWith(currentPath)) {
                    if (sDebug) {
                        qDebug() << "[GROUPEDPLAYLISTSFEATURE] Skipping playlist. "
                                    "Group name does not match path:"
                                 << groupName << "Current path:" << currentPath;
                    }
                    continue;
                }

                const QString& remainingPath = groupName.mid(currentPath.length());
                int delimiterPos = remainingPath.indexOf(delimiter);

                if (delimiterPos >= 0) {
                    const QString& subgroupName = remainingPath.left(delimiterPos);
                    subgroupedPlaylists[subgroupName].append(playlistData);
                    if (sDebug) {
                        qDebug() << "[GROUPEDPLAYLISTSFEATURE] Added playlist to "
                                    "subgroup:"
                                 << subgroupName
                                 << "Remaining path:" << remainingPath;
                    }
                } else {
                    int playlistId = playlistData["playlist_id"].toInt();

                    // adapted for playlistlogic without PlaylistId
                    // count & durations are calculated in the getgroups and put in the playlistData
                    // formatlabel needs name, count & duration
                    const QString& groupName = playlistData["group_name"].toString();
                    bool playlistLocked = (playlistData["playlist_locked"] = 1).toBool();
                    int playlistCount = playlistData["playlist_count"].toInt();
                    int playlistDurationSeconds = playlistData["playlist_durationSeconds"].toInt();

                    const QString& displayPlaylistName =
                            formatLabel(groupName.mid(currentPath.length()),
                                    playlistCount,
                                    playlistDurationSeconds);

                    TreeItem* pChildItem = pParentItem->appendChild(
                            displayPlaylistName.trimmed(), playlistId);
                    decorateChild(pChildItem, playlistId);
                    pChildItem->setFullPath(currentPath + delimiter + displayPlaylistName);

                    if (sDebug) {
                        qDebug() << "[GROUPEDPLAYLISTSFEATURE] Added playlist to "
                                    "parent:"
                                 << displayPlaylistName
                                 << "Parent:" << pParentItem->getLabel();
                        qDebug() << "[GROUPEDPLAYLISTSFEATURE] LOCKED = " << playlistLocked;
                    }
                }
            }

            for (auto it = subgroupedPlaylists.constBegin();
                    it != subgroupedPlaylists.constEnd();
                    ++it) {
                const QString& subgroupName = it.key();
                const QList<QVariantMap>& subgroupPlaylists = it.value();
                if (!subgroupPlaylists.isEmpty()) {
                    if (subgroupPlaylists.size() > 1) {
                        // subgroup has > 1 playlist -> create subgroup
                        auto pNewSubgroup = std::make_unique<TreeItem>(
                                subgroupName, kInvalidPlaylistId);
                        TreeItem* pSubgroupItem = pNewSubgroup.get();
                        pParentItem->insertChild(pParentItem->childCount(),
                                std::move(pNewSubgroup));
                        if (sDebug) {
                            qDebug() << "[GROUPEDPLAYLISTSFEATURE] Created "
                                        "subgroup:"
                                     << subgroupName
                                     << "Parent:" << pParentItem->getLabel();
                        }

                        // loop into the subgroup
                        buildTreeStructure(
                                currentPath + subgroupName + delimiter,
                                subgroupPlaylists,
                                pSubgroupItem);
                    } else {
                        // only one playlist -> directly under the parent, NO subgroup
                        const QVariantMap& playlistData = subgroupPlaylists.first();
                        int playlistId(playlistData["playlist_id"].toInt());
                        // extra
                        // adapted for playlistlogic without PlaylistId
                        // count & durations are calculated in the getgroups and
                        // put in the playlistData formatlabel needs name, count
                        // & duration
                        const QString& groupName = playlistData["group_name"].toString();
                        bool playlistLocked = (playlistData["playlist_locked"].toInt() == 1);
                        int playlistCount = playlistData["playlist_count"].toInt();
                        int playlistDurationSeconds =
                                playlistData["playlist_durationSeconds"]
                                        .toInt();

                        const QString& displayPlaylistName =
                                formatLabel(groupName.mid(currentPath.length()),
                                        playlistCount,
                                        playlistDurationSeconds);

                        TreeItem* pChildItem = pParentItem->appendChild(
                                displayPlaylistName.trimmed(), playlistId);
                        decorateChild(pChildItem, playlistId);
                        pChildItem->setFullPath(currentPath + delimiter + displayPlaylistName);
                        if (sDebug) {
                            qDebug() << "[GROUPEDPLAYLISTSFEATURE] Added single playlist to parent:"
                                     << displayPlaylistName
                                     << "Parent:" << pParentItem->getLabel();
                            qDebug() << "[GROUPEDPLAYLISTSFEATURE] LOCKED = " << playlistLocked;
                        }
                    }
                }
            }
        };
        // building rootlevel groups
        for (auto it = topLevelGroups.constBegin(); it != topLevelGroups.constEnd(); ++it) {
            const QString& groupName = it.key();
            const QList<QVariantMap>& playlists = it.value();

            if (playlists.size() > 1) {
                auto pNewGroup = std::make_unique<TreeItem>(groupName, kInvalidPlaylistId);
                TreeItem* pGroupItem = pNewGroup.get();
                pRootItem->insertChild(pRootItem->childCount(), std::move(pNewGroup));
                if (sDebug) {
                    qDebug() << "[GROUPEDPLAYLISTSFEATURE] Created top-level group:" << groupName;
                }

                buildTreeStructure(groupName + delimiter, playlists, pGroupItem);
            } else {
                const QVariantMap& playlistData = playlists.first();
                int playlistId(playlistData["playlist_id"].toInt());
                // extra
                // adapted for playlistlogic without PlaylistId
                // count & durations are calculated in the getgroups and put in the playlistData
                // formatlabel needs name, count & duration
                const QString& groupName = playlistData["group_name"].toString();
                bool playlistLocked = (playlistData["playlist_locked"].toInt() == 1);
                int playlistCount = playlistData["playlist_count"].toInt();
                int playlistDurationSeconds = playlistData["playlist_durationSeconds"].toInt();

                const QString& displayPlaylistName = formatLabel(
                        groupName, playlistCount, playlistDurationSeconds);

                TreeItem* pChildItem = pRootItem->appendChild(
                        displayPlaylistName.trimmed(), playlistId);
                decorateChild(pChildItem, playlistId);

                if (sDebug) {
                    qDebug() << "[GROUPEDPLAYLISTSFEATURE] Added playlist to "
                                "root:"
                             << displayPlaylistName;
                    qDebug() << "[GROUPEDPLAYLISTSFEATURE] LOCKED = " << playlistLocked;
                }
            }
        }
    }
    // store open/close state of groups
    for (int row = 0; row < m_pSidebarModel->rowCount(); ++row) {
        QModelIndex groupIndex = m_pSidebarModel->index(row, 0);
        if (groupIndex.isValid()) {
            TreeItem* pGroupItem = m_pSidebarModel->getItem(groupIndex);
            if (pGroupItem) {
                const QString& groupName = pGroupItem->getLabel();
                if (groupExpandedStates.contains(groupName)) {
                    m_pSidebarWidget->setExpanded(groupIndex, groupExpandedStates[groupName]);
                    if (sDebug) {
                        qDebug() << "[GROUPEDPLAYLISTSFEATURE] Restored expanded "
                                    "state for group:"
                                 << groupName << "->"
                                 << groupExpandedStates[groupName];
                    }
                }
            }
        }
    }

    if (previouslySelectedIndex.isValid()) {
        return previouslySelectedIndex;
    }

    return QModelIndex();
}

void BaseGroupedPlaylistsFeature::updateChildModel(const QSet<int>& updatedPlaylistIds) {
    if (sDebug) {
        qDebug() << "[GROUPEDPLAYLISTSFEATURE] -> updateChildModel() -> Updating "
                    "playlists"
                 << updatedPlaylistIds;
    }

    QModelIndex previouslySelectedIndex = m_lastRightClickedIndex;

    // Fetch grouped playlists using GroupedPlaylistsTableModel
    QList<QVariantMap> groupedPlaylists = m_pGroupedPlaylistsTableModel->getGroupedPlaylists();

    QMap<QString, QList<QVariantMap>> groupedPlaylistsMap;
    for (int i = 0; i < groupedPlaylists.size(); ++i) {
        const auto& playlistData = groupedPlaylists[i];
        groupedPlaylistsMap[playlistData["group_name"].toString()].append(playlistData);
        if (sDebug) {
            qDebug() << "[GROUPEDPLAYLISTSFEATURE] -> updateChildModel() -> Updating "
                        "playlists: playlistData -> groupedPlaylistsMap: group_name "
                     << groupedPlaylistsMap[playlistData["group_name"].toString()];
        }
    }

    if (m_pConfig->getValue<int>(ConfigKey("[Library]", "GroupedPlaylistsLength")) == 1) {
        // Update full paths recursively for all items starting from the root
        updateFullPathRecursive(m_pSidebarModel->getRootItem(), QString());
    }

    // Update or rebuild items
    for (const int& playlistId : updatedPlaylistIds) {
        // Find the updated playlist in groupedPlaylists
        auto updatedGroup = std::find_if(
                groupedPlaylists.begin(),
                groupedPlaylists.end(),
                [&playlistId](const QVariantMap& playlistData) {
                    return playlistData["playlist_id"].toInt() == playlistId;
                });

        if (updatedGroup != groupedPlaylists.end()) {
            QModelIndex index = indexFromPlaylistId(playlistId);
            if (index.isValid()) {
                // Update the existing item
                TreeItem* pItem = m_pSidebarModel->getItem(index);
                VERIFY_OR_DEBUG_ASSERT(pItem != nullptr) {
                    continue;
                }
                pItem->setData((*updatedGroup)["playlist_name"].toString());

                decorateChild(pItem, playlistId);

                if (m_pConfig->getValue<int>(ConfigKey(
                            "[Library]", "GroupedPlaylistsLength")) == 1) {
                    // Update fullPath for the entire tree under this item
                    updateFullPathRecursive(m_pSidebarModel->getRootItem(), QString());
                }

                m_pSidebarModel->triggerRepaint(index);
            } else {
                // Rebuild the group if the playlist is missing
                rebuildChildModel(playlistId);
            }
        }
    }
    if (previouslySelectedIndex.isValid()) {
        m_lastRightClickedIndex = previouslySelectedIndex;
    }
}

void BaseGroupedPlaylistsFeature::activateChild(const QModelIndex& index) {
    if (sDebug) {
        qDebug() << "[GROUPEDPLAYLISTSFEATURE] -> activateChild() -> index" << index;
    }
    m_lastClickedIndex = index;
    m_lastRightClickedIndex = index;
    int playlistId(playlistIdFromIndex(index));

    if (playlistId == -1) {
        // Group activated
        if (sDebug) {
            qDebug() << "[BaseGroupedPlaylistsFeature] -> activateChild() -> Group activated";
        }

        m_prevSiblingPlaylist = playlistId;
        emit saveModelState();
        emit disableSearch();
        emit enableCoverArtDisplay(false);

        if (m_pConfig->getValue<int>(ConfigKey("[Library]", "GroupedPlaylistsLength")) == 0) {
            // Fixed prefix length
            if (sDebug) {
                qDebug() << "[BaseGroupedPlaylistsFeature] -> activateChild() -> Group "
                            "activated -> groupNameFromIndex(index):"
                         << groupNameFromIndex(index);
            }
            m_pGroupedPlaylistsTableModel->selectPlaylistGroup(groupNameFromIndex(index));

        } else if (m_pConfig->getValue<int>(ConfigKey(
                           "[Library]", "GroupedPlaylistsLength")) == 1) {
            // variable group prefix length with mask
            const QString& fullPath = fullPathFromIndex(index);
            if (fullPath.isEmpty()) {
                qWarning() << "[GROUPEDPLAYLISTSFEATURE] -> activateChild() -> Group "
                              "activated: No valid full path for index: "
                           << index;
                return;
            }
            if (sDebug) {
                qDebug() << "[BaseGroupedPlaylistsFeature] -> activateChild() -> Group "
                            "activated -> fullPath:"
                         << fullPath;
            }
            m_pGroupedPlaylistsTableModel->selectPlaylistGroup(fullPath);
        }

        emit featureSelect(this, m_lastClickedIndex);
        emit showTrackModel(m_pGroupedPlaylistsTableModel);
    } else {
        // Playlist activated
        if (sDebug) {
            qDebug() << "[GROUPEDPLAYLISTSFEATURE] -> activateChild() -> Child "
                        "playlist activated -> playlistId: "
                     << playlistId;
            qDebug() << "[GROUPEDPLAYLISTSFEATURE] -> m_lastRightClickedIndex: "
                     << m_lastRightClickedIndex;
        }
        m_prevSiblingPlaylist = playlistId;
        emit saveModelState();
        m_pGroupedPlaylistsTableModel->selectPlaylist(playlistId);
        emit showTrackModel(m_pGroupedPlaylistsTableModel);
        emit enableCoverArtDisplay(true);
    }
}

QString BaseGroupedPlaylistsFeature::fullPathFromIndex(const QModelIndex& index) const {
    const QString& delimiter = m_pConfig->getValue(
            ConfigKey("[Library]", "GroupedPlaylistsVarLengthMask"));
    if (!index.isValid()) {
        return QString();
    }

    TreeItem* pItem = m_pSidebarModel->getItem(index);
    if (!pItem) {
        return QString();
    }

    QString fullPath;
    TreeItem* currentItem = pItem;
    while (currentItem) {
        if (!fullPath.isEmpty()) {
            // Prepend delimiter
            fullPath.prepend(delimiter);
        }
        // Prepend current item's label
        fullPath.prepend(currentItem->getLabel());
        currentItem = currentItem->parent();
    }

    // remove the last prepended delimiter (we don't know the depth of the tree)
    // if another root level playlist exists that is not in the group
    // (beginning of the name equal to root level groop name),
    // the member tracks would be added to the group,
    // with added delimiter only tracks in group member playlists are added
    fullPath = fullPath.mid(delimiter.length()).append(delimiter);
    return fullPath;
}

QString BaseGroupedPlaylistsFeature::groupNameFromIndex(const QModelIndex& index) const {
    if (!index.isValid()) {
        // If index Invalid -> return an empty string
        return QString();
    }

    TreeItem* pItem = m_pSidebarModel->getItem(index);
    if (!pItem) {
        // ig no item found for this index -> return an empty string
        return QString();
    }
    // if index & label found -> return label
    return pItem->getLabel();
}

void BaseGroupedPlaylistsFeature::updateFullPathRecursive(
        TreeItem* pItem, const QString& parentPath) {
    const QString& delimiter = m_pConfig->getValue(
            ConfigKey("[Library]", "GroupedPlaylistsVarLengthMask"));
    if (!pItem) {
        return;
    }

    QString currentFullPath = parentPath.isEmpty()
            ? pItem->getLabel()
            : parentPath + delimiter + pItem->getLabel();
    pItem->setFullPath(currentFullPath);

    if (sDebug) {
        qDebug() << "[GROUPEDPLAYLISTSFEATURE] -> Updated full path for item: " << pItem->getLabel()
                 << " FullPath: " << currentFullPath;
    }

    for (TreeItem* pChild : pItem->children()) {
        updateFullPathRecursive(pChild, currentFullPath);
    }
}
