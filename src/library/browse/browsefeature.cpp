#include "library/browse/browsefeature.h"

#include <QAction>
#include <QDirModel>
#include <QFileInfo>
#include <QMenu>
#include <QPushButton>
#include <QStandardPaths>
#include <QStringList>
#include <QTreeView>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/treeitem.h"
#include "moc_browsefeature.cpp"
#include "track/track.h"
#include "util/memory.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextbrowser.h"

namespace {

const QString kQuickLinksSeparator = QStringLiteral("-+-");

} // anonymous namespace

BrowseFeature::BrowseFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig,
        RecordingManager* pRecordingManager)
        : LibraryFeature(pLibrary, pConfig),
          m_pTrackCollection(pLibrary->trackCollections()->internalCollection()),
          m_browseModel(this, pLibrary->trackCollections(), pRecordingManager),
          m_proxyModel(&m_browseModel),
          m_pLastRightClickedItem(nullptr),
          m_icon(":/images/library/ic_library_computer.svg") {
    connect(this,
            &BrowseFeature::requestAddDir,
            pLibrary,
            &Library::slotRequestAddDir);

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
    foreach (QFileInfo drive, drives) {
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
        TreeItem* driveLetter =
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

    foreach (QString quickLinkPath, m_quickLinkList) {
        QString name = extractNameFromPath(quickLinkPath);
        qDebug() << "Appending Quick Link: " << name << "---" << quickLinkPath;
        m_pQuickLinkItem->appendChild(name, quickLinkPath);
    }

    // initialize the model
    m_childModel.setRootItem(std::move(pRootItem));
}

BrowseFeature::~BrowseFeature() {
}

QVariant BrowseFeature::title() {
    return QVariant(tr("Computer"));
}

void BrowseFeature::slotAddQuickLink() {
    if (!m_pLastRightClickedItem) {
        return;
    }

    QVariant vpath = m_pLastRightClickedItem->getData();
    QString spath = vpath.toString();
    QString name = extractNameFromPath(spath);

    QModelIndex parent = m_childModel.index(m_pQuickLinkItem->parentRow(), 0);
    auto pNewChild = std::make_unique<TreeItem>(name, vpath);
    QList<TreeItem*> rows;
    rows.append(pNewChild.get());
    pNewChild.release();
    m_childModel.insertTreeItemRows(rows, m_pQuickLinkItem->childRows(), parent);

    m_quickLinkList.append(spath);
    saveQuickLinks();
}

void BrowseFeature::slotAddToLibrary() {
    if (!m_pLastRightClickedItem) {
        return;
    }
    QString spath = m_pLastRightClickedItem->getData().toString();
    emit requestAddDir(spath);

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
    if (!m_pLastRightClickedItem) {
        return;
    }

    QString spath = m_pLastRightClickedItem->getData().toString();
    int index = m_quickLinkList.indexOf(spath);

    if (index == -1) {
        return;
    }

    QModelIndex parent = m_childModel.index(m_pQuickLinkItem->parentRow(), 0);
    m_childModel.removeRow(index, parent);

    m_quickLinkList.removeAt(index);
    saveQuickLinks();
}

QIcon BrowseFeature::getIcon() {
    return m_icon;
}

TreeItemModel* BrowseFeature::getChildModel() {
    return &m_childModel;
}

void BrowseFeature::bindLibraryWidget(WLibrary* libraryWidget,
                               KeyboardEventFilter* keyboard) {
    Q_UNUSED(keyboard);
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    edit->setHtml(getRootViewHtml());
    libraryWidget->registerView("BROWSEHOME", edit);
}

void BrowseFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    // store the sidebar widget pointer for later use in onRightClickChild
    m_pSidebarWidget = pSidebarWidget;
}

void BrowseFeature::activate() {
    emit switchToView("BROWSEHOME");
    emit disableSearch();
    emit enableCoverArtDisplay(false);
}

// Note: This is executed whenever you single click on an child item
// Single clicks will not populate sub folders
void BrowseFeature::activateChild(const QModelIndex& index) {
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    qDebug() << "BrowseFeature::activateChild " << item->getLabel() << " "
             << item->getData();

    QString path = item->getData().toString();
    if (path == QUICK_LINK_NODE || path == DEVICE_NODE) {
        m_browseModel.setPath({});
    } else {
        // Open a security token for this path and if we do not have access, ask
        // for it.
        auto dir = mixxx::FileAccess(mixxx::FileInfo(path));
        if (!dir.isReadable()) {
            if (Sandbox::askForAccess(path)) {
                // Re-create to get a new token.
                dir = mixxx::FileAccess(mixxx::FileInfo(path));
            } else {
                // TODO(rryan): Activate an info page about sandboxing?
                return;
            }
        }
        m_browseModel.setPath(std::move(dir));
    }
    emit showTrackModel(&m_proxyModel);
    emit enableCoverArtDisplay(false);
}

void BrowseFeature::onRightClickChild(const QPoint& globalPos, const QModelIndex& index) {
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    m_pLastRightClickedItem = item;

    if (!item) {
        return;
    }

    QString path = item->getData().toString();

    if (path == QUICK_LINK_NODE || path == DEVICE_NODE) {
        return;
    }

    QMenu menu(m_pSidebarWidget);
    if (item->parent()->getData().toString() == QUICK_LINK_NODE) {
        menu.addAction(m_pRemoveQuickLinkAction);
        menu.exec(globalPos);
        onLazyChildExpandation(index);
        return;
    }

    foreach (const QString& str, m_quickLinkList) {
        if (str == path) {
            // if path is a QuickLink:
            // show remove action
            menu.addAction(m_pRemoveQuickLinkAction);
            menu.exec(globalPos);
            onLazyChildExpandation(index);
            return;
        }
     }

     menu.addAction(m_pAddQuickLinkAction);
     menu.addAction(m_pAddtoLibraryAction);
     menu.exec(globalPos);
     onLazyChildExpandation(index);
}

namespace {
// Get the list of devices (under "Removable Devices" section).
QList<TreeItem*> getRemovableDevices() {
    QList<TreeItem*> ret;
#if defined(__WINDOWS__)
    // Repopulate drive list
    QFileInfoList drives = QDir::drives();
    // show drive letters
    foreach (QFileInfo drive, drives) {
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
        TreeItem* driveLetter = new TreeItem(
            display_path, // Displays C:
            drive.filePath()); // Displays C:/
        ret << driveLetter;
    }
#elif defined(__LINUX__)
    // To get devices on Linux, we look for directories under /media and
    // /run/media/$USER.
    QFileInfoList devices;

    // Add folders under /media to devices.
    devices += QDir("/media").entryInfoList(
        QDir::AllDirs | QDir::NoDotAndDotDot);

    // Add folders under /run/media/$USER to devices.
    QDir run_media_user_dir(QStringLiteral("/run/media/") + QString::fromLocal8Bit(qgetenv("USER")));
    devices += run_media_user_dir.entryInfoList(
        QDir::AllDirs | QDir::NoDotAndDotDot);

    // Convert devices into a QList<TreeItem*> for display.
    foreach(QFileInfo device, devices) {
        TreeItem* folder = new TreeItem(
            device.fileName(),
            QVariant(device.filePath() + QStringLiteral("/")));
        ret << folder;
    }
#endif
    return ret;
}
} // namespace

// This is called whenever you double click or use the triangle symbol to expand
// the subtree. The method will read the subfolders.
void BrowseFeature::onLazyChildExpandation(const QModelIndex& index) {
    // The selected item might have been removed just before this function
    // is invoked!
    // NOTE(2018-04-21, uklotzde): The following checks prevent a crash
    // that would otherwise occur after a quick link has been removed.
    // Especially the check of QVariant::isValid() seems to be essential.
    // See also: https://bugs.launchpad.net/mixxx/+bug/1510068
    // After migration to Qt5 the implementation of all LibraryFeatures
    // should be revisited, because I consider these checks only as a
    // temporary workaround.
    if (!index.isValid()) {
        return;
    }
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    if (!(item && item->getData().isValid())) {
        return;
    }

    qDebug() << "BrowseFeature::onLazyChildExpandation " << item->getLabel()
             << " " << item->getData();

    QString path = item->getData().toString();

    // If the item is a build-in node, e.g., 'QuickLink' return
    if (path.isEmpty() || path == QUICK_LINK_NODE) {
        return;
    }

    // Before we populate the subtree, we need to delete old subtrees
    m_childModel.removeRows(0, item->childRows(), index);

    // List of subfolders or drive letters
    QList<TreeItem*> folders;

    // If we are on the special device node
    if (path == DEVICE_NODE) {
        folders += getRemovableDevices();
    } else {
        // we assume that the path refers to a folder in the file system
        // populate childs
        const auto dirAccess = mixxx::FileAccess(mixxx::FileInfo(path));

        QFileInfoList all = dirAccess.info().toQDir().entryInfoList(
                QDir::Dirs | QDir::NoDotAndDotDot);

        // loop through all the item and construct the childs
        foreach (QFileInfo one, all) {
            // Skip folders that end with .app on OS X
#if defined(__APPLE__)
            if (one.isDir() && one.fileName().endsWith(".app"))
                continue;
#endif
            // We here create new items for the sidebar models
            // Once the items are added to the TreeItemModel,
            // the models takes ownership of them and ensures their deletion
            TreeItem* folder = new TreeItem(
                one.fileName(),
                QVariant(one.absoluteFilePath() + QStringLiteral("/")));
            folders << folder;
        }
    }
    // we need to check here if subfolders are found
    // On Ubuntu 10.04, otherwise, this will draw an icon although the folder
    // has no subfolders
    if (!folders.isEmpty()) {
        m_childModel.insertTreeItemRows(folders, 0, index);
    }
}

QString BrowseFeature::getRootViewHtml() const {
    QString browseTitle = tr("Computer");
    QString browseSummary = tr("\"Computer\" lets you navigate, view, and load tracks"
                        " from folders on your hard disk and external devices.");

    QString html;
    html.append(QString("<h2>%1</h2>").arg(browseTitle));
    html.append(QString("<p>%1</p>").arg(browseSummary));
    return html;
}

void BrowseFeature::saveQuickLinks() {
    m_pConfig->set(ConfigKey("[Browse]","QuickLinks"),ConfigValue(
        m_quickLinkList.join(kQuickLinksSeparator)));
}

void BrowseFeature::loadQuickLinks() {
    if (m_pConfig->getValueString(ConfigKey("[Browse]","QuickLinks")).isEmpty()) {
        m_quickLinkList = getDefaultQuickLinks();
    } else {
        m_quickLinkList = m_pConfig->getValueString(
            ConfigKey("[Browse]","QuickLinks")).split(kQuickLinksSeparator);
    }
}

QString BrowseFeature::extractNameFromPath(const QString& spath) {
    QString path = spath.left(spath.count()-1);
    int index = path.lastIndexOf("/");
    QString name = (spath.count() > 1) ? path.mid(index+1) : spath;
    return name;
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
    const auto rootDirs = m_pLibrary->trackCollections()->internalCollection()->loadRootDirs();
    for (const mixxx::FileInfo& fileInfo : rootDirs) {
        const auto dir = fileInfo.toQDir();
        // Skip directories we don't have permission to.
        if (!Sandbox::canAccessDir(dir)) {
            continue;
        }
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

    if (!osDesktopDirIncluded &&
            Sandbox::canAccessDir(osDesktopDir)) {
        result << osDesktopDir.canonicalPath() + "/";
    }

    if (!osDocumentsDirIncluded &&
            Sandbox::canAccessDir(osDocumentsDir)) {
        result << osDocumentsDir.canonicalPath() + "/";
    }

    qDebug() << "Default quick links:" << result;
    return result;
}
