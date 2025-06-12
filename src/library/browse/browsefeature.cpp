#include "library/browse/browsefeature.h"

#include <QAction>
#include <QFileInfo>
#include <QMenu>
#include <QPushButton>
#include <QStandardPaths>
#include <QStringList>
#include <memory>

#include "library/browse/foldertreemodel.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/treeitem.h"
#include "moc_browsefeature.cpp"
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

BrowseFeature::BrowseFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig,
        RecordingManager* pRecordingManager)
        : LibraryFeature(pLibrary, pConfig, QString("computer")),
          m_pTrackCollection(pLibrary->trackCollectionManager()->internalCollection()),
          m_browseModel(this, pLibrary->trackCollectionManager(), pRecordingManager),
          m_proxyModel(&m_browseModel, true),
          m_pSidebarModel(new FolderTreeModel(this)) {
    connect(&m_browseModel,
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

    m_pAddQuickLinkAction = new QAction(tr("Add to Quick Links"),this);
    connect(m_pAddQuickLinkAction,
            &QAction::triggered,
            this,
            &BrowseFeature::slotAddQuickLink);

    m_pRemoveQuickLinkAction = new QAction(tr("Remove from Quick Links"),this);
    connect(m_pRemoveQuickLinkAction,
            &QAction::triggered,
            this,
            &BrowseFeature::slotRemoveQuickLink);

    m_pAddtoLibraryAction = new QAction(tr("Add to Library"),this);
    connect(m_pAddtoLibraryAction,
            &QAction::triggered,
            this,
            &BrowseFeature::slotAddToLibrary);

    m_pRefreshDirTreeAction = new QAction(tr("Refresh directory tree"), this);
    connect(m_pRefreshDirTreeAction,
            &QAction::triggered,
            this,
            &BrowseFeature::slotRefreshDirectoryTree);

    m_proxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel.setSortCaseSensitivity(Qt::CaseInsensitive);
    // BrowseThread sets the Qt::UserRole of every QStandardItem to the sort key
    // of the item.
    m_proxyModel.setSortRole(Qt::UserRole);
    // Dynamically re-sort contents as we add items to the source model.
    m_proxyModel.setDynamicSortFilter(true);

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
        m_pQuickLinkItem->appendChild(name, quickLinkPath);
    }

    // initialize the model
    m_pSidebarModel->setRootItem(std::move(pRootItem));
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

    const QString name = extractNameFromPath(path);

    const QModelIndex parent = m_pSidebarModel->index(m_pQuickLinkItem->parentRow(), 0);
    std::vector<std::unique_ptr<TreeItem>> rows;
    // TODO() Use here std::span to get around the heap allocation of
    // std::vector for a single element.
    rows.push_back(std::make_unique<TreeItem>(name, path));
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
        return;
    }

    // TODO Check if this really added a new directory. Ignore if it's a child
    // of an already watched directory. Notify if it failed for another reason.
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    // strings are dupes from DlgPrefLibrary
    msgBox.setWindowTitle(tr("Music Directory Added"));
    msgBox.setText(tr("You added one or more music directories. The tracks in "
                      "these directories won't be available until you rescan "
                      "your library. Would you like to rescan now?"));
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
}

// Note: This is executed whenever you single click on an child item
// Single clicks will not populate sub folders
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
    if (path == QUICK_LINK_NODE || path == DEVICE_NODE) {
        emit saveModelState();
        // Clear the tracks view
        m_browseModel.setPath({});
    } else {
        // Open a security token for this path and if we do not have access, ask
        // for it.
        auto dirInfo = mixxx::FileInfo(path);
        auto dirAccess = mixxx::FileAccess(dirInfo);
        if (!dirAccess.isReadable()) {
            if (Sandbox::askForAccess(&dirInfo)) {
                // Re-create to get a new token.
                dirAccess = mixxx::FileAccess(dirInfo);
            } else {
                // TODO(rryan): Activate an info page about sandboxing?
                return;
            }
        }
        emit saveModelState();
        m_browseModel.setPath(std::move(dirAccess));
    }
    emit showTrackModel(&m_proxyModel);
    // Search is restored in Library::slotShowTrackModel, disable it where it's useless
    if (path == QUICK_LINK_NODE || path == DEVICE_NODE) {
        emit disableSearch();
    }
    emit enableCoverArtDisplay(false);
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

    // TODO Check if we already watch this path or a parent and don't show or
    // disable this action.
    menu.addAction(m_pAddtoLibraryAction);
    menu.addAction(m_pRefreshDirTreeAction);
    menu.exec(globalPos);
}

namespace {
// Get the list of devices (under "Removable Devices" section).
std::vector<std::unique_ptr<TreeItem>> createRemovableDevices() {
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
        ret.push_back(std::make_unique<TreeItem>(
                device.fileName(),
                QVariant(device.filePath() + QStringLiteral("/"))));
    }
#endif
    return ret;
}
} // namespace

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
        folders = getChildDirectoryItems(path);
    }

    if (!folders.empty()) {
        m_pSidebarModel->insertTreeItemRows(std::move(folders), 0, index);
    }
}

std::vector<std::unique_ptr<TreeItem>> BrowseFeature::getChildDirectoryItems(
        const QString& path) const {
    std::vector<std::unique_ptr<TreeItem>> items;

    if (path.isEmpty()) {
        return items;
    }
    // we assume that the path refers to a folder in the file system
    // populate children
    const auto dirAccess = mixxx::FileAccess(mixxx::FileInfo(path));

    QFileInfoList all = dirAccess.info().toQDir().entryInfoList(
            QDir::Dirs | QDir::NoDotAndDotDot);

    // loop through all the item and construct the children
    foreach (QFileInfo one, all) {
        // Skip folders that end with .app on OS X
#if defined(__APPLE__)
        if (one.isDir() && one.fileName().endsWith(".app"))
            continue;
#endif
        // We here create new items for the sidebar models
        // Once the items are added to the TreeItemModel,
        // the models takes ownership of them and ensures their deletion
        items.push_back(std::make_unique<TreeItem>(
                one.fileName(),
                QVariant(one.absoluteFilePath() + QStringLiteral("/"))));
    }

    return items;
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

void BrowseFeature::releaseBrowseThread() {
    m_browseModel.releaseBrowseThread();
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
