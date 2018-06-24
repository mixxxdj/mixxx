// browsefeature.cpp
// Created 9/8/2009 by RJ Ryan (rryan@mit.edu)

#include <widget/wlibrarypane.h>
#include <QAction>
#include <QDesktopServices>
#include <QDirModel>
#include <QFileInfo>
#include <QMenu>
#include <QPushButton>
#include <QStringList>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/features/browse/browsefeature.h"
#include "library/trackcollection.h"
#include "util/sandbox.h"
#include "widget/wlibrarystack.h"
#include "widget/wlibrarytextbrowser.h"

#include "util/memory.h"

const QString kQuickLinksSeparator = "-+-";

BrowseFeature::BrowseFeature(UserSettingsPointer pConfig,
                             Library* pLibrary,
                             QObject* parent,
                             TrackCollection* pTrackCollection,
                             RecordingManager* pRecordingManager)
        : LibraryFeature(pConfig, pLibrary, pTrackCollection, parent),
          m_browseModel(this, pTrackCollection, pRecordingManager),
          m_proxyModel(&m_browseModel),
          m_pTrackCollection(pTrackCollection) {
    connect(this, SIGNAL(requestAddDir(QString)),
            parent, SLOT(slotRequestAddDir(QString)));

    m_pAddQuickLinkAction = new QAction(tr("Add to Quick Links"),this);
    connect(m_pAddQuickLinkAction, SIGNAL(triggered()), this, SLOT(slotAddQuickLink()));

    m_pRemoveQuickLinkAction = new QAction(tr("Remove from Quick Links"),this);
    connect(m_pRemoveQuickLinkAction, SIGNAL(triggered()), this, SLOT(slotRemoveQuickLink()));

    m_pAddtoLibraryAction = new QAction(tr("Add to Library"),this);
    connect(m_pAddtoLibraryAction, SIGNAL(triggered()),
            this, SLOT(slotAddToLibrary()));

    m_proxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel.setSortCaseSensitivity(Qt::CaseInsensitive);
    // BrowseThread sets the Qt::UserRole of every QStandardItem to the sort key
    // of the item.
    m_proxyModel.setSortRole(Qt::UserRole);
    // Dynamically re-sort contents as we add items to the source model.
    m_proxyModel.setDynamicSortFilter(true);

    // The invisible root item of the child model
    auto pRootItem = std::make_unique<TreeItem>(this);

    m_pQuickLinkItem = parented_ptr<TreeItem>(
            pRootItem->appendChild(tr("Quick Links"), QUICK_LINK_NODE));

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
    pRootItem->appendChild(tr("Removable Devices"), "/media/");

    // show root directory on UNIX-based operating systems
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

QString BrowseFeature::getIconPath() {
    return ":/images/library/ic_library_computer.png";
}

QString BrowseFeature::getSettingsName() const {
    return "BrowseFeature";
}

void BrowseFeature::slotAddQuickLink() {
    if (!m_lastRightClickedIndex.isValid()) {
        return;
    }

    QVariant vpath = m_lastRightClickedIndex.data(AbstractRole::RoleData);
    QString spath = vpath.toString();
    QString name = extractNameFromPath(spath);

    QModelIndex parent = m_childModel.index(m_pQuickLinkItem->parentRow(), 0);
    auto pNewChild = std::make_unique<TreeItem>(this, name, vpath);
    QList<TreeItem*> rows;
    rows.append(pNewChild.get());
    pNewChild.release();
    m_childModel.insertTreeItemRows(rows, m_pQuickLinkItem->childRows(), parent);

    m_quickLinkList.append(spath);
    saveQuickLinks();
}

void BrowseFeature::slotAddToLibrary() {
    if (!m_lastRightClickedIndex.isValid()) {
        return;
    }
    QString spath = m_lastRightClickedIndex.data(AbstractRole::RoleData).toString();
    emit(requestAddDir(spath));

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
        emit(scanLibrary());
    }
}

void BrowseFeature::slotLibraryScanStarted() {
    m_pAddtoLibraryAction->setEnabled(false);
}

void BrowseFeature::slotLibraryScanFinished() {
    m_pAddtoLibraryAction->setEnabled(true);
}

void BrowseFeature::slotRemoveQuickLink() {
    if (!m_lastRightClickedIndex.isValid()) {
        return;
    }

    QString spath = m_lastRightClickedIndex.data(AbstractRole::RoleData).toString();
    int index = m_quickLinkList.indexOf(spath);

    if (index == -1) {
        return;
    }

    QModelIndex parent = m_childModel.index(m_pQuickLinkItem->parentRow(), 0);
    m_childModel.removeRow(index, parent);

    m_quickLinkList.removeAt(index);
    saveQuickLinks();
}

QPointer<TreeItemModel> BrowseFeature::getChildModel() {
    return &m_childModel;
}

parented_ptr<QWidget> BrowseFeature::createPaneWidget(KeyboardEventFilter* pKeyboard,
            int paneId, QWidget* parent) {
    auto pStack = make_parented<WLibraryStack>(parent);
    m_panes[paneId] = pStack.toWeakRef();

    auto pEdit = make_parented<WLibraryTextBrowser>(pStack.get());
    pEdit->setHtml(getRootViewHtml());
    pEdit->installEventFilter(pKeyboard);
    m_idBrowse[paneId] = pStack->addWidget(pEdit.get());

    auto pTable = LibraryFeature::createPaneWidget(pKeyboard, paneId, pStack.get());
    m_idTable[paneId] = pStack->addWidget(pTable.get());

    return pStack;
}

void BrowseFeature::activate() {
    if (m_lastClickedChild.isValid()) {
        activateChild(m_lastClickedChild);
        return;
    }

    showBrowse(m_featurePane);
    switchToFeature();
    showBreadCrumb();
    restoreSearch(QString());

}

// Note: This is executed whenever you single click on an child item
// Single clicks will not populate sub folders
void BrowseFeature::activateChild(const QModelIndex& index) {
    m_lastClickedChild = index;
    QString data = index.data().toString();
    QString dataPath = index.data(AbstractRole::RoleData).toString();
    qDebug() << "BrowseFeature::activateChild " << data << dataPath;

    if (dataPath == QUICK_LINK_NODE || dataPath == DEVICE_NODE) {
        m_browseModel.setPath(MDir());
    } else {
        // Open a security token for this path and if we do not have access, ask
        // for it.
        MDir dir(dataPath);
        if (!dir.canAccess()) {
            if (Sandbox::askForAccess(dataPath)) {
                // Re-create to get a new token.
                dir = MDir(dataPath);
            } else {
                // TODO(rryan): Activate an info page about sandboxing?
                return;
            }
        }
        m_browseModel.setPath(dir);
    }

    showTable(m_featurePane);
    showTrackModel(&m_proxyModel);
    QString bread = index.data(AbstractRole::RoleBreadCrumb).toString();
    showBreadCrumb(bread, getIcon());

}

void BrowseFeature::onRightClickChild(const QPoint& globalPos,
                                      const QModelIndex& index) {
    m_lastRightClickedIndex = index;

    if (!index.isValid() || !index.parent().isValid())
        return;

    QString path = index.data(AbstractRole::RoleData).toString();

    if (path == QUICK_LINK_NODE || path == DEVICE_NODE) {
        return;
    }

    QMenu menu(nullptr);
    if (index.parent().data(AbstractRole::RoleData).toString() == QUICK_LINK_NODE) {
        // Store as persistent because otherwise if the user decides to remove
        // the quick link it causes a segmentation fault.
        QPersistentModelIndex persitstentIndex = index;
        menu.addAction(m_pRemoveQuickLinkAction);
        menu.exec(globalPos);
        onLazyChildExpandation(persitstentIndex);
        return;
    }

    for (const QString& str : m_quickLinkList) {
        if (str == path) {
             return;
        }
     }

     menu.addAction(m_pAddQuickLinkAction);
     menu.addAction(m_pAddtoLibraryAction);
     menu.exec(globalPos);
     onLazyChildExpandation(index);
}

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

    QString label = index.data().toString();
    QString path = index.data(AbstractRole::RoleData).toString();

    qDebug() << "BrowseFeature::onLazyChildExpandation " << label
             << " " << path;

    // If the item is a build-in node, e.g., 'QuickLink' return
    if (path.isEmpty() || path == QUICK_LINK_NODE) {
        return;
    }

    // Before we populate the subtree, we need to delete old subtrees
    int rows = m_childModel.rowCount(index);
    m_childModel.removeRows(0, rows, index);

    // List of subfolders or drive letters
    QList<TreeItem*> folders;

    // If we are on the special device node
    if (path == DEVICE_NODE) {
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
                this,
                display_path, // Displays C:
                drive.filePath()); // Displays C:/
            folders << driveLetter;
        }
    } else {
        // we assume that the path refers to a folder in the file system
        // populate childs
        MDir dir(path);

        QFileInfoList all = dir.dir().entryInfoList(
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
                this,
                one.fileName(),
                one.absoluteFilePath() + "/");
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

QString BrowseFeature::extractNameFromPath(QString spath) {
    QString path = spath.left(spath.count()-1);
    int index = path.lastIndexOf("/");
    QString name = (spath.count() > 1) ? path.mid(index+1) : spath;
    return name;
}

QStringList BrowseFeature::getDefaultQuickLinks() const {
    // Default configuration
    QStringList mixxxMusicDirs = m_pTrackCollection->getDirectoryDAO().getDirs();
    QDir osMusicDir(QDesktopServices::storageLocation(
            QDesktopServices::MusicLocation));
    QDir osDocumentsDir(QDesktopServices::storageLocation(
            QDesktopServices::DocumentsLocation));
    QDir osHomeDir(QDesktopServices::storageLocation(
            QDesktopServices::HomeLocation));
    QDir osDesktopDir(QDesktopServices::storageLocation(
            QDesktopServices::DesktopLocation));
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
    foreach (QString dirPath, mixxxMusicDirs) {
        QDir dir(dirPath);
        // Skip directories we don't have permission to.
        if (!Sandbox::canAccessFile(dir)) {
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

    if (!osMusicDirIncluded && Sandbox::canAccessFile(osMusicDir)) {
        result << osMusicDir.canonicalPath() + "/";
    }

    if (downloadsExists && !osDownloadsDirIncluded &&
            Sandbox::canAccessFile(osDownloadsDir)) {
        result << osDownloadsDir.canonicalPath() + "/";
    }

    if (!osDesktopDirIncluded &&
            Sandbox::canAccessFile(osDesktopDir)) {
        result << osDesktopDir.canonicalPath() + "/";
    }

    if (!osDocumentsDirIncluded &&
            Sandbox::canAccessFile(osDocumentsDir)) {
        result << osDocumentsDir.canonicalPath() + "/";
    }

    qDebug() << "Default quick links:" << result;
    return result;
}

void BrowseFeature::showBrowse(int paneId) {
    auto it = m_panes.find(paneId);
    auto itId = m_idBrowse.find(paneId);
    if (it != m_panes.end() && !it->isNull() && itId != m_idBrowse.end()) {
        (*it)->setCurrentIndex(*itId);
    }
}

void BrowseFeature::showTable(int paneId) {
    auto itId = m_idTable.find(paneId);
    auto it = m_panes.find(paneId);
    if (it != m_panes.end() && !it->isNull() && itId != m_idTable.end()) {
        (*it)->setCurrentIndex(*itId);
    }
}
