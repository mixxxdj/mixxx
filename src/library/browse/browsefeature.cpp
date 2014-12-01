// browsefeature.cpp
// Created 9/8/2009 by RJ Ryan (rryan@mit.edu)

#include <QStringList>
#include <QTreeView>
#include <QDirModel>
#include <QStringList>
#include <QFileInfo>
#include <QDesktopServices>
#include <QAction>
#include <QMenu>
#include <QPushButton>

#include "trackinfoobject.h"
#include "library/treeitem.h"
#include "library/browse/browsefeature.h"
#include "library/trackcollection.h"
#include "widget/wlibrarytextbrowser.h"
#include "widget/wlibrary.h"
#include "mixxxkeyboard.h"
#include "util/sandbox.h"

const QString kQuickLinksSeparator = "-+-";

BrowseFeature::BrowseFeature(QObject* parent,
                             ConfigObject<ConfigValue>* pConfig,
                             TrackCollection* pTrackCollection,
                             RecordingManager* pRecordingManager)
        : LibraryFeature(parent),
          m_pConfig(pConfig),
          m_browseModel(this, pTrackCollection, pRecordingManager),
          m_proxyModel(&m_browseModel),
          m_pTrackCollection(pTrackCollection),
          m_pLastRightClickedItem(NULL) {
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
    TreeItem* rootItem = new TreeItem();

    m_pQuickLinkItem = new TreeItem(tr("Quick Links"), QUICK_LINK_NODE, this, rootItem);
    rootItem->appendChild(m_pQuickLinkItem);

    // Create the 'devices' shortcut
#if defined(__WINDOWS__)
    TreeItem* devices_link = new TreeItem(
        tr("Devices"), DEVICE_NODE, this, rootItem);
    rootItem->appendChild(devices_link);
    // show drive letters
    QFileInfoList drives = QDir::drives();
    // show drive letters
    foreach (QFileInfo drive, drives) {
        TreeItem* driveLetter = new TreeItem(
            drive.canonicalPath(),  //  displays C:
            drive.filePath(),  // Displays C:/
            this ,
            devices_link);
        devices_link->appendChild(driveLetter);
    }
#elif defined(__APPLE__)
    // Apple hides the base Linux file structure But all devices are mounted at
    // /Volumes
    TreeItem* devices_link = new TreeItem(
        tr("Devices"), "/Volumes/", this, rootItem);
    rootItem->appendChild(devices_link);
#else  // LINUX
    TreeItem* devices_link = new TreeItem(
        tr("Removable Devices"), "/media/", this, rootItem);
    rootItem->appendChild(devices_link);

    // show root directory on UNIX-based operating systems
    TreeItem* root_folder_item = new TreeItem(
        QDir::rootPath(), QDir::rootPath(), this, rootItem);
    rootItem->appendChild(root_folder_item);
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
        TreeItem *item = new TreeItem(name, quickLinkPath, this, m_pQuickLinkItem);
        m_pQuickLinkItem->appendChild(item);
    }

    // initialize the model
    m_childModel.setRootItem(rootItem);
}

BrowseFeature::~BrowseFeature() {
}

QVariant BrowseFeature::title() {
    return QVariant(tr("Browse"));
}

void BrowseFeature::slotAddQuickLink() {
    if (!m_pLastRightClickedItem) {
        return;
    }

    QString spath = m_pLastRightClickedItem->dataPath().toString();
    QString name = extractNameFromPath(spath);
    TreeItem *item = new TreeItem(name, spath, this, m_pQuickLinkItem);
    m_pQuickLinkItem->appendChild(item);
    m_quickLinkList.append(spath);
    saveQuickLinks();
}

void BrowseFeature::slotAddToLibrary() {
    if (!m_pLastRightClickedItem) {
        return;
    }
    QString spath = m_pLastRightClickedItem->dataPath().toString();
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
    if (!m_pLastRightClickedItem) {
        return;
    }

    QString spath = m_pLastRightClickedItem->dataPath().toString();
    int index = m_quickLinkList.indexOf(spath);

    if (index == -1) {
        return;
    }
    m_pQuickLinkItem->removeChild(index);
    m_quickLinkList.removeAt(index);
    saveQuickLinks();
}

QIcon BrowseFeature::getIcon() {
    return QIcon(":/images/library/ic_library_browse.png");
}

TreeItemModel* BrowseFeature::getChildModel() {
    return &m_childModel;
}

void BrowseFeature::bindWidget(WLibrary* libraryWidget,
                               MixxxKeyboard* keyboard) {
    Q_UNUSED(keyboard);
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    edit->setHtml(getRootViewHtml());
    libraryWidget->registerView("BROWSEHOME", edit);
}

void BrowseFeature::activate() {
    emit(switchToView("BROWSEHOME"));
    emit(restoreSearch(QString()));
    emit(enableCoverArtDisplay(false));
}

// Note: This is executed whenever you single click on an child item
// Single clicks will not populate sub folders
void BrowseFeature::activateChild(const QModelIndex& index) {
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    qDebug() << "BrowseFeature::activateChild " << item->data() << " "
             << item->dataPath();

    QString path = item->dataPath().toString();
    if (path == QUICK_LINK_NODE || path == DEVICE_NODE) {
        m_browseModel.setPath(MDir());
    } else {
        // Open a security token for this path and if we do not have access, ask
        // for it.
        MDir dir(path);
        if (!dir.canAccess()) {
            if (Sandbox::askForAccess(path)) {
                // Re-create to get a new token.
                dir = MDir(path);
            } else {
                // TODO(rryan): Activate an info page about sandboxing?
                return;
            }
        }
        m_browseModel.setPath(dir);
    }
    emit(showTrackModel(&m_proxyModel));
    emit(enableCoverArtDisplay(false));
}

void BrowseFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    m_pLastRightClickedItem = item;

    if (!item) {
        return;
    }

    QString path = item->dataPath().toString();

    if (path == QUICK_LINK_NODE || path == DEVICE_NODE) {
        return;
    }

    QMenu menu(NULL);
    if (item->parent()->dataPath().toString() == QUICK_LINK_NODE) {
        menu.addAction(m_pRemoveQuickLinkAction);
        menu.exec(globalPos);
        onLazyChildExpandation(index);
        return;
    }

    foreach (const QString& str, m_quickLinkList) {
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
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    if (!item) {
        return;
    }

    qDebug() << "BrowseFeature::onLazyChildExpandation " << item->data()
             << " " << item->dataPath();

    QString path = item->dataPath().toString();

    // If the item is a build-in node, e.g., 'QuickLink' return
    if (path == QUICK_LINK_NODE) {
        return;
    }

    // Before we populate the subtree, we need to delete old subtrees
    m_childModel.removeRows(0, item->childCount(), index);

    // List of subfolders or drive letters
    QList<TreeItem*> folders;

    // If we are on the special device node
    if (path == DEVICE_NODE) {
        // Repopulate drive list
        QFileInfoList drives = QDir::drives();
        // show drive letters
        foreach (QFileInfo drive, drives) {
            TreeItem* driveLetter = new TreeItem(
                drive.canonicalPath(), // displays C:
                drive.filePath(), //Displays C:/
                this,
                item);
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
                one.fileName(),
                one.absoluteFilePath() + "/",
                this, item);
            folders << folder;
        }
    }
    // we need to check here if subfolders are found
    // On Ubuntu 10.04, otherwise, this will draw an icon although the folder
    // has no subfolders
    if (!folders.isEmpty()) {
        m_childModel.insertRows(folders, 0, folders.size(), index);
    }
}

QString BrowseFeature::getRootViewHtml() const {
    QString browseTitle = tr("Browse");
    QString browseSummary = tr("Browse lets you navigate, view, and load tracks"
                        " from folders on your hard disk and external devices.");

    QString html;
    html.append(QString("<h2>%1</h2>").arg(browseTitle));
    html.append("<table border=\"0\" cellpadding=\"5\"><tr><td>");
    html.append(QString("<p>%1</p>").arg(browseSummary));
    html.append("</td></tr></table>");
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
