#include "library/browse/browsefeature.h"

#include <QAction>
#include <QDirIterator>
#include <QFileInfo>
#include <QMenu>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardPaths>
#include <QStringList>
#include <QtConcurrentRun>
#include <memory>

#include "library/browse/browselibrarytablemodel.h"
#include "library/browse/browsetablemodel.h"
#include "library/browse/foldertreemodel.h"
#include "library/library.h"
#include "library/library_prefs.h"
#include "library/proxytrackmodel.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/treeitem.h"
#include "moc_browsefeature.cpp"
#include "util/cmdlineargs.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextbrowser.h"

namespace {

const QString kViewName = QStringLiteral("BROWSEHOME");

const QString kQuickLinksSeparator = QStringLiteral("-+-");

const ConfigKey kQuickLinksCfgKey = ConfigKey("[Browse]", "QuickLinks");

#if defined(__LINUX__)
const QStringList removableDriveRootPaths() {
    QStringList paths;
    const QString user = QString::fromLocal8Bit(qgetenv("USER"));
    paths.append("/media");
    paths.append(QStringLiteral("/media/") + user);
    paths.append(QStringLiteral("/run/media/") + user);
    return paths;
}
#endif

} // anonymous namespace

BrowseFeature::BrowseFeature(Library* pLibrary,
        UserSettingsPointer pConfig,
        RecordingManager* pRecordingManager)
        : LibraryFeature(pLibrary, pConfig, QString("computer")),
          m_pTrackCollection(
                  pLibrary->trackCollectionManager()->internalCollection()),
          m_pBrowseModel(make_parented<BrowseTableModel>(
                  this, pLibrary->trackCollectionManager(), pRecordingManager)),
          m_pProxyModel(std::make_unique<ProxyTrackModel>(
                  m_pBrowseModel, true /* handle search */)),
          m_pLibraryTableModel(make_parented<BrowseLibraryTableModel>(this,
                  pLibrary->trackCollectionManager())),
          m_pCurrentTrackModel(nullptr),
          m_pSidebarModel(make_parented<FolderTreeModel>(this)) {
    connect(m_pBrowseModel,
            &BrowseTableModel::restoreModelState,
            this,
            &LibraryFeature::restoreModelState);
    connect(m_pSidebarModel,
            &QAbstractItemModel::rowsAboutToBeRemoved,
            this,
            &BrowseFeature::invalidateRightClickIndex);
    connect(m_pSidebarModel,
            &QAbstractItemModel::rowsAboutToBeInserted,
            this,
            &BrowseFeature::invalidateRightClickIndex);
    connect(m_pSidebarModel,
            &QAbstractItemModel::modelAboutToBeReset,
            this,
            &BrowseFeature::invalidateRightClickIndex);

    m_pAddQuickLinkAction = make_parented<QAction>(tr("Add to Quick Links"), this);
    connect(m_pAddQuickLinkAction,
            &QAction::triggered,
            this,
            &BrowseFeature::slotAddQuickLink);

    m_pRemoveQuickLinkAction = make_parented<QAction>(tr("Remove from Quick Links"), this);
    connect(m_pRemoveQuickLinkAction,
            &QAction::triggered,
            this,
            &BrowseFeature::slotRemoveQuickLink);

    m_pAddtoLibraryAction = make_parented<QAction>(tr("Add to Library"), this);
    connect(m_pAddtoLibraryAction,
            &QAction::triggered,
            this,
            &BrowseFeature::slotAddToLibrary);

    m_pRefreshDirTreeAction = make_parented<QAction>(tr("Refresh directory tree"), this);
    connect(m_pRefreshDirTreeAction,
            &QAction::triggered,
            this,
            &BrowseFeature::slotRefreshDirectoryTree);

    m_pProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_pProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    // BrowseThread sets the Qt::UserRole of every QStandardItem to the sort key
    // of the item.
    m_pProxyModel->setSortRole(Qt::UserRole);
    // Dynamically re-sort contents as we add items to the source model.
    m_pProxyModel->setDynamicSortFilter(true);

    // The invisible root item of the child model
    std::unique_ptr<TreeItem> pRootItem = TreeItem::newRoot(this);

    m_pQuickLinkItem = pRootItem->appendChild(tr("Quick Links"), QUICK_LINK_NODE);

    // Create the 'devices' shortcut
#if defined(__WINDOWS__)
    TreeItem* devices_link = pRootItem->appendChild(tr("Devices"), DEVICE_NODE);
    // show drive letters
    QFileInfoList drives = QDir::drives();
    // show drive letters
    for (const QFileInfo& drive : std::as_const(drives)) {
        // Using drive.filePath() to get path to display instead of drive.canonicalPath()
        // as it delay the startup too much if there is a network share mounted
        // (drive letter assigned) but unavailable
        // We avoid using canonicalPath() here because it makes an
        // unneeded system call to the underlying filesystem which
        // can be very long if the said filesystem is an unavailable
        // network share. drive.filePath() doesn't make any filesystem call
        // in this case because drive is an absolute path as it is taken from
        // QDir::drives(). See Qt's QDir code, especially qdir.cpp
        QString display_path = drive.filePath();
        if (display_path.endsWith("/")) {
            display_path.chop(1);
        }
        devices_link->appendChild(
                display_path, // Displays C:
                drive.filePath()); // Displays C:/
    }
#elif defined(__APPLE__)
    // Apple hides the base Linux file structure But all devices are mounted at
    // /Volumes
    pRootItem->appendChild(tr("Devices"), "/Volumes/");
#else  // LINUX
    // DEVICE_NODE contents will be rendered lazily in onLazyChildExpandation.
    pRootItem->appendChild(tr("Removable Devices"), DEVICE_NODE);

    // show root directory on Linux.
    pRootItem->appendChild(QDir::rootPath(), QDir::rootPath());
#endif

    // Just a word about how the TreeItem objects are used for the BrowseFeature:
    // The constructor has 4 arguments:
    // 1. argument represents the folder name shown in the sidebar later on
    // 2. argument represents the folder path which MUST end with '/'
    // 3. argument is the library feature itself
    // 4. the parent TreeItem object
    //
    // Except the invisible root item, you must always state all 4 arguments.
    //
    // Once the TreeItem objects are inserted to models, the models take care of their
    // deletion.

    loadQuickLinks();

    for (const QString& quickLinkPath : std::as_const(m_quickLinkList)) {
        QString name = extractNameFromPath(quickLinkPath);
        qDebug() << "Appending Quick Link: " << name << "---" << quickLinkPath;
        auto pItem = createPathTreeItem(name, quickLinkPath);
        m_pQuickLinkItem->insertChild(m_pQuickLinkItem->childRows(), std::move(pItem));
    }

    // initialize the model
    m_pSidebarModel->setRootItem(std::move(pRootItem));

    // Prepare everything for creating the symlink map of track directories.
    // For reason and details updateSymlinkList().
    // This is called by slotLibraryDirectoriesChanged() and then all items'
    // `isWatchedLibraryPath` flags are updated accordingly.
    connect(&m_future_watcher,
            &QFutureWatcher<QMap<QString, QString>>::finished,
            this,
            &BrowseFeature::onSymLinkMapUpdated);
    // We don't need to update now if we rescan on startup or scheduled a scan via
    // command line; in both cases it's triggered by CoreServices after the library
    // has been constructed, and we're listening to the libraryScanFinished() signal below.
    bool rescan = CmdlineArgs::Instance().getRescanLibrary() ||
            pConfig->getValue<bool>(mixxx::library::prefs::kRescanOnStartupConfigKey);
    if (!rescan) {
        slotLibraryDirectoriesChanged();
    }

    // Update symlink map after library scan and when library
    // directories have been changed in the preferences.
    connect(pLibrary,
            &Library::trackDirectoriesUpdated,
            this,
            &BrowseFeature::slotLibraryDirectoriesChanged);
    connect(pLibrary->trackCollectionManager(),
            &TrackCollectionManager::libraryScanFinished,
            this,
            &BrowseFeature::slotLibraryDirectoriesChanged);
}

BrowseFeature::~BrowseFeature() {
}

QVariant BrowseFeature::title() {
    return QVariant(tr("Computer"));
}

void BrowseFeature::slotAddQuickLink() {
    const QString path = getLastRightClickedPath();
    if (path.isEmpty()) {
        return;
    }

    const QModelIndex parent = m_pSidebarModel->index(m_pQuickLinkItem->parentRow(), 0);
    std::vector<std::unique_ptr<TreeItem>> rows;
    // TODO() Use here std::span to get around the heap allocation of
    // std::vector for a single element.
    const QString name = extractNameFromPath(path);
    qDebug() << "Appending Quick Link: " << name << "---" << path;
    auto pItem = createPathTreeItem(name, path);
    rows.emplace_back(std::move(pItem));
    m_pSidebarModel->insertTreeItemRows(std::move(rows), m_pQuickLinkItem->childRows(), parent);

    m_quickLinkList.append(path);
    saveQuickLinks();
}

void BrowseFeature::slotAddToLibrary() {
    const QString path = getLastRightClickedPath();
    if (path.isEmpty()) {
        return;
    }

    if (!m_pLibrary->requestAddDir(path)) {
        // Return if adding failed due to missing/unreadable/SQL error,
        // or if the directory is already watched.
        return;
    }

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    // strings are dupes from DlgPrefLibrary
    msgBox.setWindowTitle(tr("Music Directory Added"));
    QString msgText = tr(
            "You added one or more music directories. The tracks in "
            "these directories won't be available until you rescan "
            "your library. Would you like to rescan now?");
    msgText.append(QStringLiteral("\n\n"));
    msgText.append(tr(
            "If yes, you also need to click the directory again after the"
            "scan in order to switch from the current file tag view to the "
            "library view for this directory."));
    msgBox.setText(msgText);
    QPushButton* scanButton = msgBox.addButton(
        tr("Scan"), QMessageBox::AcceptRole);
    msgBox.addButton(QMessageBox::Cancel);
    msgBox.setDefaultButton(scanButton);
    msgBox.exec();

    if (msgBox.clickedButton() == scanButton) {
        emit scanLibrary();
    }
}

void BrowseFeature::slotLibraryScanStarted() {
    m_pAddtoLibraryAction->setEnabled(false);
}

void BrowseFeature::slotLibraryScanFinished() {
    m_pAddtoLibraryAction->setEnabled(true);
}

void BrowseFeature::slotRemoveQuickLink() {
    const QString path = getLastRightClickedPath();
    if (path.isEmpty()) {
        return;
    }

    int quickLinkIndex = m_quickLinkList.indexOf(path);
    if (quickLinkIndex == -1) {
        return;
    }

    // Quick Links' parent is QModelIndex(), so we can call this without parent
    // and still get the QAbstractItemModel::hasIndex() match.
    QModelIndex parent = m_pSidebarModel->index(m_pQuickLinkItem->parentRow(), 0);
    m_pSidebarModel->removeRow(quickLinkIndex, parent);

    m_quickLinkList.removeAt(quickLinkIndex);
    saveQuickLinks();
}

void BrowseFeature::slotRefreshDirectoryTree() {
    if (!m_lastRightClickedIndex.isValid()) {
        return;
    }

    const QString path = getLastRightClickedPath();
    m_pSidebarModel->removeChildDirsFromCache(QStringList{path});

    // Update child items
    onLazyChildExpandation(m_lastRightClickedIndex);
}

void BrowseFeature::slotLibraryDirectoriesChanged() {
    // Let a worker thread do the XML parsing
    // Cancel the current run.
    if (!m_future_watcher.isFinished()) {
        // After cancel() the watcher will also return finished()???
        // qWarning() << "# cancel()";
        // m_future_watcher.cancel();
        m_future_watcher.waitForFinished();
    }
    // Run
    const auto rootDirs =
            m_pTrackCollection->getDirectoryDAO().loadAllDirectories(true /* ignore missing */);
    m_future = QtConcurrent::run(&BrowseFeature::updateSymlinkList,
            this,
            rootDirs);
    m_future_watcher.setFuture(m_future);
}

QMap<QString, QString> BrowseFeature::updateSymlinkList(const QList<mixxx::FileInfo>& rootDirs) {
    // Create the map of symlink'ed track (sub)directories.
    // It's reverse, ie. <target, symlink>
    //
    // The purpose:
    // Ability to unresolve clicked symlink targets in order to show the content
    // of the respective library directory with the full track view.
    //
    // Let's say we have a library directory /home/user/Music
    // which is a symlink to /mnt/SSD/music
    // In the tree this directory can occur in multiple places, for example
    // QuickLinks -> Music
    // /mnt/SSD/music
    // When selecting either of those, we want the correct directory filter for
    // the library view model for showing all contained tracks.
    //
    // However, Library directories and their tracks' locations are stored by
    // location (absolute path), which -in case of a symlink- is the symlink location,
    // NOT its target's location.
    // So, when selecting /mnt/SSD/music, the directory filter would not yield any
    // results.
    //
    // This reverse target/symlink map allows us to get the correct library
    // (sub)directory for a given symlink target.
    //
    // NOTE: we could also use this map when creating setting new tree items' paths
    // so we work with the lib dir path right away and save us the unresole step
    // on selection. BUT in case library directories are removed/relocated we'd
    // not only have to update the items' `isPathWatched` flag but also their path :|
    //
    // Also check symlik'ed directories, inside or outside library directories,
    // that are effectively inside library dirs. Example
    // /home/ is a library dir. Cases covered:
    // /home/Abc links to /mnt/Abc
    // /mnt/Cba links to /home/Abc
    // /mnt/Bca links to /mnt/Abc

    VERIFY_OR_DEBUG_ASSERT_THIS_QOBJECT_THREAD_ANTI_AFFINITY() {
        // Must only be run in separate thread via QFutureWatcher and
        // QtConcurrent::run()
        return {};
    }

    // There's no need for high. Also it may also be run startup and
    // we don't want to push back other threads for this.
    QThread* thisThread = QThread::currentThread();
    thisThread->setPriority(QThread::LowPriority);

    PerformanceTimer timer;
    timer.start();

    QMap<QString, QString> symLinksMap;

    for (auto& rootDir : rootDirs) {
        symLinksMap.insert(rootDir.location(), rootDir.location());
        if (rootDir.location() != rootDir.canonicalLocation()) {
            // some path segment is a symlink
            symLinksMap.insert(rootDir.canonicalLocation(), rootDir.location());
        }
        // Scan all subdirectories
        QDirIterator it(rootDir.location(),
                QDir::Dirs | QDir::NoDotAndDotDot,
                QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
        while (it.hasNext()) {
            it.next();
            mixxx::FileInfo dirInfo(it.fileInfo());
            if (dirInfo.canonicalLocation().isEmpty()) {
                continue;
            }
            symLinksMap.insert(dirInfo.location(), dirInfo.location());
            symLinksMap.insert(dirInfo.canonicalLocation(), dirInfo.location());
        }
    }
    return symLinksMap;
}

void BrowseFeature::onSymLinkMapUpdated() {
    m_mutex.lock();
    // Swap the symlink map
    m_trackDirSymlinksMap = m_future_watcher.result();
    m_mutex.unlock();
    slotUpdateAllTreeItemsIsWatchedPath();
}

void BrowseFeature::slotUpdateAllTreeItemsIsWatchedPath() {
    // Update ALL items
    for (int row = 0; row < m_pSidebarModel->rowCount(); ++row) {
        const QModelIndex index = m_pSidebarModel->index(row, 0);
        TreeItem* pTreeItem = m_pSidebarModel->getItem(index);
        VERIFY_OR_DEBUG_ASSERT(pTreeItem) {
            continue;
        }
        updateItemIsWatchedPathRecursively(pTreeItem);
    }
    // Make sure we have a sidebar widget
    if (m_pSidebarWidget) {
        m_pSidebarWidget->update();
        // If `isWatched` of the currently selected path doesn't match
        // the used model anymore, update it.
        const auto& selectedIndex = m_pSidebarWidget->selectedIndex();
        auto* pSelectedItem = m_pSidebarModel->getItem(selectedIndex);
        if (!pSelectedItem) {
            return;
        }
        bool watched = pSelectedItem->isWatchedLibraryPath();
        bool usingLibraryView =
                static_cast<BrowseLibraryTableModel*>(m_pCurrentTrackModel) != nullptr;
        if (watched != usingLibraryView) {
            activateChild(selectedIndex);
        }
    }
}

void BrowseFeature::updateItemIsWatchedPathRecursively(TreeItem* pItem) {
    VERIFY_OR_DEBUG_ASSERT(pItem) {
        return;
    }
    const auto itemData = pItem->getData();
    VERIFY_OR_DEBUG_ASSERT(itemData.isValid() && itemData.canConvert<QString>()) {
        return;
    }
    const QString path = itemData.toString();
    if (path.isEmpty()) {
        return;
    }

    if (isPathWatched(path)) {
        pItem->updateIsWatchedLibraryPathRecursively(true);
    } else {
        pItem->setIsWatchedLibraryPath(false);
        for (auto* pChild : std::as_const(pItem->children())) {
            updateItemIsWatchedPathRecursively(pChild);
        }
    }
}

TreeItemModel* BrowseFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void BrowseFeature::bindLibraryWidget(WLibrary* libraryWidget,
                               KeyboardEventFilter* keyboard) {
    Q_UNUSED(keyboard);
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    edit->setHtml(getRootViewHtml());
    libraryWidget->registerView(kViewName, edit);
}

void BrowseFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    // store the sidebar widget pointer for later use in onRightClickChild
    m_pSidebarWidget = pSidebarWidget;
}

void BrowseFeature::activate() {
    emit switchToView(kViewName);
    emit disableSearch();
    emit enableCoverArtDisplay(false);
    // TODO rm. useful for debugging
    // slotLibraryDirectoriesChanged();
}

/// This is executed whenever you single click on an child item. The track view
/// is then populated with tracks from that directory.
/// Two different views (models) are used, depending on whether the directory
/// is a (child of a) library directory or not:
/// 1. not a library dir: file view
///    This view shows the audio tags of the track files each time the directory item
///    is activated, regardless if individual (or all) tracks have been added to the
///    library. Population is usually slow and doesn't have all library columns.
/// 2. a library dir: track (library) view
///    Displays metadata from the library (database) like in other features,
///    has all columns and capabilities of Tracks.
///    Note: this also applies if a library (sub)directory is a symlink and we
///    clicked the symlink target (which may be outside the library dir).
///
/// Note: single clicks will not expand or populate sub directories.
void BrowseFeature::activateChild(const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }
    TreeItem* pItem = static_cast<TreeItem*>(index.internalPointer());
    if (!(pItem && pItem->getData().isValid())) {
        return;
    }
    qDebug() << "BrowseFeature::activateChild " << pItem->getLabel() << " "
             << pItem->getData().toString();

    QString path = pItem->getData().toString();
    if (path.isEmpty()) {
        return;
    }

    mixxx::FileAccess dirAccess;
    bool pathIsQuickLinkOrDevice = path == QUICK_LINK_NODE || path == DEVICE_NODE;
    bool useLibraryTrackView = pItem->isWatchedLibraryPath();
    if (!pathIsQuickLinkOrDevice && !useLibraryTrackView) {
        // Unwatched and potentially unknown directory.
        // Open a security token for this path and if we do not have access, ask
        // for it.
        auto dirInfo = mixxx::FileInfo(path);
        dirAccess = mixxx::FileAccess(dirInfo);
        if (!dirAccess.isReadable()) {
            if (Sandbox::askForAccess(&dirInfo)) {
                // Re-create to get a new token.
                dirAccess = mixxx::FileAccess(dirInfo);
            } else {
                // TODO(rryan): Activate an info page about sandboxing?
                return;
            }
        }
    }

    emit saveModelState();

    const QString currSearch = getCurrentSearch();
    // TODO sync sort column, if possible
    // * get sort column from current model
    // * if we switch the model, set sort column
    if (useLibraryTrackView) {
        // use LibraryTableModel
        // filter tracks by directory (not recursive)
        // Resolve path, ie. figure if it's symlink'ed in some library directory.
        // If yes, use the un-resolved path to set the directory filter
        const QString unresolvedPath = maybeUnResolveSymlink(std::move(path));
        m_pLibraryTableModel->setPath(std::move(unresolvedPath));
        m_pLibraryTableModel->search(currSearch, "");
        m_pCurrentTrackModel = m_pLibraryTableModel;
        emit showTrackModel(m_pLibraryTableModel);
    } else {
        // use BrowseTableModel
        // dirAccess is empty in case of QUICK_LINK_NODE or DEVICE_NODE
        m_pBrowseModel->setPath(std::move(dirAccess));
        m_pProxyModel->search(currSearch);
        m_pCurrentTrackModel = m_pProxyModel.get();
        emit showTrackModel(m_pProxyModel.get());
    }
    // Search is restored in Library::slotShowTrackModel, disable it where it's useless
    if (pathIsQuickLinkOrDevice) {
        emit disableSearch();
    }
    emit enableCoverArtDisplay(useLibraryTrackView);
}

void BrowseFeature::onRightClickChild(const QPoint& globalPos, const QModelIndex& index) {
    TreeItem* pItem = static_cast<TreeItem*>(index.internalPointer());
    if (!pItem) {
        return;
    }

    QString path = pItem->getData().toString();

    if (path == QUICK_LINK_NODE || path == DEVICE_NODE) {
        return;
    }

    // Make sure that this is reset whenever the tree changes
    // and it may have become a dangling pointer
    m_lastRightClickedIndex = index;

    QMenu menu(m_pSidebarWidget);

    if (pItem->parent()->getData().toString() == QUICK_LINK_NODE ||
            m_quickLinkList.contains(path)) {
        // This is a QuickLink or path is in the Quick Link list
        menu.addAction(m_pRemoveQuickLinkAction);
    } else {
        menu.addAction(m_pAddQuickLinkAction);
    }

    if (!pItem->isWatchedLibraryPath()) {
        menu.addAction(m_pAddtoLibraryAction);
    }
    menu.addAction(m_pRefreshDirTreeAction);
    menu.exec(globalPos);
}

// Get the list of devices (under "Removable Devices" section).
std::vector<std::unique_ptr<TreeItem>> BrowseFeature::createRemovableDevices() const {
    std::vector<std::unique_ptr<TreeItem>> ret;
#if defined(__WINDOWS__)
    // Repopulate drive list
    const QFileInfoList drives = QDir::drives();
    // show drive letters
    for (const QFileInfo& drive : std::as_const(drives)) {
        // Using drive.filePath() instead of drive.canonicalPath() as it
        // freezes interface too much if there is a network share mounted
        // (drive letter assigned) but unavailable
        //
        // drive.canonicalPath() make a system call to the underlying filesystem
        // introducing delay if it is unreadable.
        // drive.filePath() doesn't make any access to the filesystem and consequently
        // shorten the delay
        QString display_path = drive.filePath();
        if (display_path.endsWith("/")) {
            display_path.chop(1);
        }
        ret.push_back(std::make_unique<TreeItem>(
                display_path,       // Displays C:
                drive.filePath())); // Displays C:/
    }
#elif defined(__LINUX__)
    QFileInfoList devices;
    for (const QString& path : removableDriveRootPaths()) {
        devices += QDir(path).entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
    }

    // Convert devices into a QList<TreeItem*> for display.
    for (const QFileInfo& device : std::as_const(devices)) {
        // On Linux, devices can be mounted in /media and /media/user and /run/media/[user]
        // but there's no benefit of displaying the [user] dir in Devices.
        // Show its children but skip the dir itself.
        if (removableDriveRootPaths().contains(device.absoluteFilePath())) {
            continue;
        }
        const QString path = device.filePath() + QStringLiteral("/");
        auto pNewItem = createPathTreeItem(device.fileName(), path, false);
        ret.emplace_back(std::move(pNewItem));
    }
#endif
    return ret;
}

// This is called whenever you double click or use the triangle symbol to expand
// the subtree. The method will read the subfolders.
void BrowseFeature::onLazyChildExpandation(const QModelIndex& index) {
    // Caution: Make sure the passed index still exists in the model.
    // In case it has been removed or replaced, it is still "valid", but
    // holds dangling internalPointer() that causes a crash.
    // These sanity checks will pass in such case.
    if (!index.isValid()) {
        return;
    }
    TreeItem* pItem = static_cast<TreeItem*>(index.internalPointer());
    if (!(pItem && pItem->getData().isValid())) {
        return;
    }

    qDebug() << "BrowseFeature::onLazyChildExpandation " << pItem->getLabel()
             << " " << pItem->getData();

    QString path = pItem->getData().toString();

    // If the item is a built-in node, e.g., 'QuickLink' return
    if (path.isEmpty() || path == QUICK_LINK_NODE) {
        return;
    }

    // Before we populate the subtree, we need to delete old subtrees
    m_pSidebarModel->removeRows(0, pItem->childRows(), index);

    // List of subfolders or drive letters
    std::vector<std::unique_ptr<TreeItem>> folders;

    // If we are on the special device node
    if (path == DEVICE_NODE) {
#if defined(__LINUX__)
        // Tell the model to remove the cached 'hasChildren' states of all sub-
        // directories when we expand the Device node.
        // This ensures we show the real dir tree. This is relevant when devices
        // were unmounted, changed and mounted again.
        m_pSidebarModel->removeChildDirsFromCache(removableDriveRootPaths());
#endif
        folders = createRemovableDevices();
    } else {
        bool isWatched = pItem->isWatchedLibraryPath();
        folders = getChildDirectoryItems(path, isWatched);
    }

    if (!folders.empty()) {
        m_pSidebarModel->insertTreeItemRows(std::move(folders), 0, index);
    }
}

std::vector<std::unique_ptr<TreeItem>> BrowseFeature::getChildDirectoryItems(
        const QString& path,
        bool isWatched) const {
    std::vector<std::unique_ptr<TreeItem>> items;

    if (path.isEmpty()) {
        return items;
    }
    // we assume that the path refers to a folder in the file system
    // populate children
    const QDir dir(path);
    const QFileInfoList all = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    // loop through all the item and construct the children
    for (const auto& one : all) {
        // Skip folders that end with .app on OS X
#if defined(__APPLE__)
        if (one.isDir() && one.fileName().endsWith(".app"))
            continue;
#endif
        // We here create new items for the sidebar models.
        // Once the items are added to the TreeItemModel, the model takes
        // ownership of them and ensures their deletion.
        // Note: use absolutePath(), not canonicalPath().
        const QString chPath = one.absoluteFilePath() + QStringLiteral("/");
        auto pNewItem = createPathTreeItem(one.fileName(), chPath, isWatched);
        items.emplace_back(std::move(pNewItem));
    }

    return items;
}

std::unique_ptr<TreeItem> BrowseFeature::createPathTreeItem(
        const QString& name,
        const QString& path,
        bool parentIsWatched) const {
    auto pItem = std::make_unique<TreeItem>(name, path);
    pItem->setIsWatchedLibraryPath(parentIsWatched
                    ? true
                    : isPathWatched(path));
    return pItem;
}

bool BrowseFeature::isPathWatched(const QString& path) const {
    // Here we check if a path is a (child of a) library root directory by looking
    // it up in the root dir / symlinks map.
    if (path.isEmpty() || path == QUICK_LINK_NODE || path == DEVICE_NODE) {
        return false;
    }

    const auto dir = mixxx::FileInfo(path);
    if (!dir.exists() || !dir.isDir()) {
        qWarning() << "Failed to check" << dir.location();
        qWarning() << "Directory does not exist, is inaccessible or is not a directory";
        return false;
    }
    // NOTE Don't assert dir.isreadable(), this may be the linux root directory
    if (!dir.isReadable()) {
        qWarning() << "Aborting to check" << dir.location();
        qWarning() << "Directory can not be read";
        return false;
    }

    const auto dirCanLoc = dir.canonicalLocation();
    VERIFY_OR_DEBUG_ASSERT(!dirCanLoc.isEmpty()) {
        return false;
    }

    // Check if we have the path in the symlinks map.
    // Returns true if this is a symlink or target
    // Includes library root dirs
    const QMap<QString, QString>::const_iterator itcan =
            m_trackDirSymlinksMap.constFind(dir.canonicalLocation());
    if (itcan != m_trackDirSymlinksMap.constEnd()) {
        qWarning() << "     ~ isPathWatched YES" << path;
        if (dir.location() != itcan.value()) {
            // qWarning() << "    resolved to" << itcan.value();
        }
        return true;
    }
    return false;
}

QString BrowseFeature::maybeUnResolveSymlink(const QString& path) const {
    // Check symlinks. See updateSymlinkList() for details.
    mixxx::FileInfo dir(path);
    const QMap<QString, QString>::const_iterator it =
            m_trackDirSymlinksMap.constFind(dir.canonicalLocation());
    if (it == m_trackDirSymlinksMap.constEnd()) {
        return path;
    }

    // location() mmisses the trailing '/' but we need it only for the file view
    return it.value();
}

QString BrowseFeature::getRootViewHtml() const {
    const QString browseTitle = tr("Computer");
    const QString browseSummary = tr(
            "\"Computer\" lets you navigate, view, and load tracks"
            " from folders on your hard disk and external devices.");
    const QString browseDetails = tr(
                                          "It shows the data from the file tags, not track data"
                                          " from your Mixxx library like other track views.") +
            "<br><br>" +
            tr("If you load a track file from here, it will be added to your library.");

    QString html;
    html.append(QString("<h2>%1</h2>").arg(browseTitle));
    html.append(QString("<p>%1</p>").arg(browseSummary));
    html.append(QString("<p>%1</p>").arg(browseDetails));
    return html;
}

void BrowseFeature::saveQuickLinks() {
    m_pConfig->setValue<QString>(kQuickLinksCfgKey,
            m_quickLinkList.join(kQuickLinksSeparator));
}

void BrowseFeature::loadQuickLinks() {
    if (!m_pConfig->exists(kQuickLinksCfgKey)) {
        // New profile, create default Quick Links
        m_quickLinkList = getDefaultQuickLinks();
    } else {
        const QString quickLinks = m_pConfig->getValueString(
                kQuickLinksCfgKey);
        if (!quickLinks.isEmpty()) {
            m_quickLinkList = quickLinks.split(kQuickLinksSeparator);
        }
    }
}

QString BrowseFeature::extractNameFromPath(const QString& spath) {
    return QDir(spath).dirName();
}

QStringList BrowseFeature::getDefaultQuickLinks() const {
    // Default configuration
    QDir osMusicDir(QStandardPaths::writableLocation(
            QStandardPaths::MusicLocation));
    QDir osDocumentsDir(QStandardPaths::writableLocation(
            QStandardPaths::DocumentsLocation));
    QDir osHomeDir(QStandardPaths::writableLocation(
            QStandardPaths::HomeLocation));
    QDir osDesktopDir(QStandardPaths::writableLocation(
            QStandardPaths::DesktopLocation));
    QDir osDownloadsDir(osHomeDir);
    // TODO(XXX) i18n -- no good way to get the download path. We could tr() it
    // but the translator may not realize we want the usual name of the
    // downloads folder.
    bool downloadsExists = osDownloadsDir.cd("Downloads");

    QStringList result;
    bool osMusicDirIncluded = false;
    bool osDownloadsDirIncluded = false;
    bool osDesktopDirIncluded = false;
    bool osDocumentsDirIncluded = false;
    const auto rootDirs = m_pLibrary->trackCollectionManager()
                                  ->internalCollection()
                                  ->loadRootDirs();
    for (mixxx::FileInfo fileInfo : rootDirs) {
        // Skip directories we don't have permission to.
        if (!Sandbox::canAccess(&fileInfo)) {
            continue;
        }
        const auto dir = fileInfo.toQDir();
        if (dir == osMusicDir) {
            osMusicDirIncluded = true;
        }
        if (dir == osDownloadsDir) {
            osDownloadsDirIncluded = true;
        }
        if (dir == osDesktopDir) {
            osDesktopDirIncluded = true;
        }
        if (dir == osDocumentsDir) {
            osDocumentsDirIncluded = true;
        }
        result << dir.canonicalPath() + "/";
    }

    if (!osMusicDirIncluded && Sandbox::canAccessDir(osMusicDir)) {
        result << osMusicDir.canonicalPath() + "/";
    }

    if (downloadsExists && !osDownloadsDirIncluded &&
            Sandbox::canAccessDir(osDownloadsDir)) {
        result << osDownloadsDir.canonicalPath() + "/";
    }

    if (!osDesktopDirIncluded && Sandbox::canAccessDir(osDesktopDir)) {
        result << osDesktopDir.canonicalPath() + "/";
    }

    if (!osDocumentsDirIncluded && Sandbox::canAccessDir(osDocumentsDir)) {
        result << osDocumentsDir.canonicalPath() + "/";
    }

    qDebug() << "Default quick links:" << result;
    return result;
}

QString BrowseFeature::getCurrentSearch() const {
    return m_pCurrentTrackModel != nullptr ? m_pCurrentTrackModel->currentSearch() : QString();
}

void BrowseFeature::releaseBrowseThread() {
    m_pBrowseModel->releaseBrowseThread();
}

QString BrowseFeature::getLastRightClickedPath() const {
    if (!m_lastRightClickedIndex.isValid()) {
        return {};
    }
    TreeItem* pItem = static_cast<TreeItem*>(m_lastRightClickedIndex.internalPointer());
    VERIFY_OR_DEBUG_ASSERT(pItem && pItem->getData().isValid()) {
        return {};
    }
    return pItem->getData().toString();
}

void BrowseFeature::invalidateRightClickIndex() {
    m_lastRightClickedIndex = QModelIndex();
}
