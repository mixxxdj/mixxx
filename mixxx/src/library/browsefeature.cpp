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
#include "library/browsefeature.h"
#include "library/browsefilter.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"
#include "library/dao/trackdao.h"
#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytableview.h"
#include "widget/wbrowsetableview.h"



BrowseFeature::BrowseFeature(QObject* parent, ConfigObject<ConfigValue>* pConfig, TrackCollection* pTrackCollection)
        : LibraryFeature(parent),
          m_pConfig(pConfig),
          m_browseModel(this),
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

    TreeItem* mixxx_library_dir_item = new TreeItem("Mixxx Library", mixxx_music_dir +"/" ,this , quick_link);
    quick_link->appendChild(mixxx_library_dir_item);

    TreeItem*os_home_dir_item = new TreeItem("Home Directory", os_home_folder_dir +"/" , this , quick_link);
    quick_link->appendChild(os_home_dir_item);

    TreeItem*os_music_dir_item = new TreeItem("Music Directory", os_music_folder_dir +"/" , this , quick_link);
    quick_link->appendChild(os_music_dir_item);

    TreeItem*os_docs_dir_item = new TreeItem("Document Directory", os_documents_folder_dir +"/" , this , quick_link);
    quick_link->appendChild(os_docs_dir_item);

    TreeItem*os_desktop_dir_item = new TreeItem("Desktop", os_desktop_folder_dir +"/" , this , quick_link);
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
    return false;
}

bool BrowseFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

bool BrowseFeature::dragMoveAccept(QUrl url) {
    return false;
}

bool BrowseFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

void BrowseFeature::activate() {
    emit(restoreSearch(m_currentSearch));
}

void BrowseFeature::activateChild(const QModelIndex& index) {
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    qDebug() << "BrowseFeature::activateChild " << item->data() << " " << item->dataPath();

    //if(m_childModel.canFetchMore(index))
        m_childModel.fetchMore(index);

    m_browseModel.setPath(item->dataPath().toString());
    emit(showTrackModel(&m_proxyModel));

}

void BrowseFeature::onRightClick(const QPoint& globalPos) {
}

void BrowseFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
}
