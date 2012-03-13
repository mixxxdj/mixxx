// browsefeature.cpp
// Created 9/8/2009 by RJ Ryan (rryan@mit.edu)

#include <QStringList>
#include <QTreeView>
#include <QDirModel>
#include <QStringList>
#include <QFileInfo>
#include <QDesktopServices>

#include "trackinfoobject.h"
#include "library/treeitem.h"
#include "library/browse/browsefeature.h"
#include "library/trackcollection.h"
#include "library/dao/trackdao.h"

BrowseFeature::BrowseFeature(QObject* parent, ConfigObject<ConfigValue>* pConfig,
                             TrackCollection* pTrackCollection,
                             RecordingManager* pRecordingManager)
        : LibraryFeature(parent),
          m_pConfig(pConfig),
          m_browseModel(this, pTrackCollection, pRecordingManager),
          m_proxyModel(&m_browseModel),
          m_pTrackCollection(pTrackCollection) {

    m_proxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel.setSortCaseSensitivity(Qt::CaseInsensitive);

    //The invisible root item of the child model
    TreeItem* rootItem = new TreeItem();

    TreeItem* quick_link = new TreeItem(tr("Quick Links"), QUICK_LINK_NODE ,this , rootItem);
    rootItem->appendChild(quick_link);

    //Create the 'devices' shortcut
#if defined(__WINDOWS__)
    TreeItem* devices_link = new TreeItem(tr("Devices"), DEVICE_NODE ,this , rootItem);
    rootItem->appendChild(devices_link);
    //show drive letters
    QFileInfoList drives = QDir::drives();
    //show drive letters
    foreach(QFileInfo drive, drives){
        TreeItem* driveLetter = new TreeItem(
                        drive.canonicalPath(), // displays C:
                        drive.filePath(), //Displays C:/
                        this ,
                        devices_link);
        devices_link->appendChild(driveLetter);
    }
#elif defined(__APPLE__)
    //Apple hides the base Linux file structure
    //But all devices are mounted at /Volumes
    TreeItem* devices_link = new TreeItem(tr("Devices"), "/Volumes/", this , rootItem);
    rootItem->appendChild(devices_link);
#else //LINUX
    TreeItem* devices_link = new TreeItem(tr("Removable Devices"), "/media/", this , rootItem);
    rootItem->appendChild(devices_link);

    //show root directory on UNIX-based operating systems
    TreeItem* root_folder_item = new TreeItem(QDir::rootPath(), QDir::rootPath(),this , rootItem);
    rootItem->appendChild(root_folder_item);

#endif

    /*
     * Just a word about how the TreeItem objects are used for the BrowseFeature:
     * The constructor has 4 arguments:
     * 1. argument represents the folder name shown in the sidebar later on
     * 2. argument represents the folder path which MUST end with '/'
     * 3. argument is the library feature itself
     * 4. the parent TreeItem object
     *
     * Except the invisible root item, you must always state all 4 arguments.
     *
     * Once the TreeItem objects are inserted to models, the models take care of their
     * deletion.
     */

    //Add a shortcut to the Music folder which Mixxx uses
    QString mixxx_music_dir = m_pConfig->getValueString(ConfigKey("[Playlist]","Directory"));
    QString os_music_folder_dir = QDesktopServices::storageLocation(QDesktopServices::MusicLocation);
    QString os_documents_folder_dir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    QString os_home_folder_dir = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
    QString os_desktop_folder_dir = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);

    TreeItem* mixxx_library_dir_item = new TreeItem(tr("Mixxx Library"), mixxx_music_dir +"/" ,this , quick_link);
    quick_link->appendChild(mixxx_library_dir_item);

    TreeItem*os_home_dir_item = new TreeItem(tr("Home"), os_home_folder_dir +"/" , this , quick_link);
    quick_link->appendChild(os_home_dir_item);

    TreeItem*os_music_dir_item = new TreeItem(tr("Music"), os_music_folder_dir +"/" , this , quick_link);
    quick_link->appendChild(os_music_dir_item);

    TreeItem*os_docs_dir_item = new TreeItem(tr("Documents"), os_documents_folder_dir +"/" , this , quick_link);
    quick_link->appendChild(os_docs_dir_item);

    TreeItem*os_desktop_dir_item = new TreeItem(tr("Desktop"), os_desktop_folder_dir +"/" , this , quick_link);
    quick_link->appendChild(os_desktop_dir_item);

    //initialize the model
    m_childModel.setRootItem(rootItem);
}

BrowseFeature::~BrowseFeature() {

}

QVariant BrowseFeature::title() {
    return QVariant(tr("Browse"));
}

QIcon BrowseFeature::getIcon() {
    return QIcon(":/images/library/ic_library_browse.png");
}

TreeItemModel* BrowseFeature::getChildModel() {
    return &m_childModel;
}

bool BrowseFeature::dropAccept(QUrl url) {
    Q_UNUSED(url);
    return false;
}

bool BrowseFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    Q_UNUSED(index);
    Q_UNUSED(url);
    return false;
}

bool BrowseFeature::dragMoveAccept(QUrl url) {
    Q_UNUSED(url);
    return false;
}

bool BrowseFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    Q_UNUSED(index);
    Q_UNUSED(url);
    return false;
}

void BrowseFeature::activate() {
    emit(restoreSearch(m_currentSearch));
}
/*
 * Note: This is executed whenever you single click on an child item
 * Single clicks will not populate sub folders
 */
void BrowseFeature::activateChild(const QModelIndex& index) {
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    qDebug() << "BrowseFeature::activateChild " << item->data() << " " << item->dataPath();
    m_browseModel.setPath(item->dataPath().toString());
    emit(showTrackModel(&m_proxyModel));

}

void BrowseFeature::onRightClick(const QPoint& globalPos) {
    Q_UNUSED(globalPos);
}

void BrowseFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
    Q_UNUSED(globalPos);
    Q_UNUSED(index);
}

/*
 * This is called whenever you double click or use the triangle symbol to expand
 * the subtree. The method will read the subfolders.
 */
void BrowseFeature::onLazyChildExpandation(const QModelIndex &index){
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    qDebug() << "BrowseFeature::onLazyChildExpandation " << item->data() << " " << item->dataPath();

    // If the item is a build-in node, e.g., 'QuickLink' return
    if(item->dataPath().toString() == QUICK_LINK_NODE)
        return;

    //Before we populate the subtree, we need to delete old subtrees
   m_childModel.removeRows(0, item->childCount(), index);

    // List of subfolders or drive letters
    QList<TreeItem*> folders;

    // If we are on the special device node
    if(item->dataPath().toString() == DEVICE_NODE){
       //Repopulate drive list
        QFileInfoList drives = QDir::drives();
        //show drive letters
        foreach(QFileInfo drive, drives){
            TreeItem* driveLetter = new TreeItem(
                            drive.canonicalPath(), // displays C:
                            drive.filePath(), //Displays C:/
                            this ,
                            item);
            folders << driveLetter;
        }

    }
    else // we assume that the path refers to a folder in the file system
    {
        //populate childs
        QDir dir(item->dataPath().toString());
        QFileInfoList all = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

        // loop through all the item and construct the childs
        foreach(QFileInfo one, all){
            //Skip folders that end with .app on OS X
#if defined(__APPLE__)
            if (one.isDir() && one.fileName().endsWith(".app"))
                continue;
#endif
            // We here create new items for the sidebar models
            // Once the items are added to the TreeItemModel,
            // the models takes ownership of them and ensures their deletion
            TreeItem* folder = new TreeItem(one.fileName(),
                                            item->dataPath().toString().append(one.fileName() +"/"),
                                            this,
                                            item);
            folders << folder;
        }
    }
    //we need to check here if subfolders are found
    //On Ubuntu 10.04, otherwise, this will draw an icon although the folder has no subfolders
    if(!folders.isEmpty())
       m_childModel.insertRows(folders, 0, folders.size() , index);
}
