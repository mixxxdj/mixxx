// browsefeature.cpp
// Created 9/8/2009 by RJ Ryan (rryan@mit.edu)

#include <QStringList>
#include <QTreeView>
#include <QDirModel>
#include <QStringList>
#include <QFileInfo>

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
          //m_proxyModel(this),
          m_pTrackCollection(pTrackCollection) {
    //m_proxyModel.setSourceModel(&m_browseModel);
    
    //connect(this, SIGNAL(setRootIndex(const QModelIndex&)),
    //        &m_proxyModel, SLOT(setProxyParent(const QModelIndex&)));
            
    //The invisible root item of the child model
    TreeItem* rootItem = new TreeItem();
    
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
    TreeItem* mixxx_music_dir_item = new TreeItem("My Music", mixxx_music_dir +"/" ,this , rootItem);
    rootItem->appendChild(mixxx_music_dir_item);
    
#if defined(__WINDOWS__)
    QFileInfoList drives = QDir::drives();
    //show drive letters
    foreach(QFileInfo drive, drives){
        TreeItem* driveLetter = new TreeItem(
                        drive.canonicalPath(), // displays C:
                        drive.filePath(), //Displays C:/
                        this , 
                        rootItem);
        rootItem->appendChild(driveLetter);
    }
#elif defined(__APPLE__)
    //Apple hides the base Linux file structure
    //But all devices are mounted at /Volumes
    TreeItem* devices = new TreeItem("Devices", "/Volumes/", this , rootItem);
    rootItem->appendChild(devices);
#else
    //show root directory on UNIX-based operating systems
    TreeItem* root_folder_item = new TreeItem(QDir::rootPath(), QDir::rootPath(),this , rootItem);
    rootItem->appendChild(root_folder_item);
#endif
    
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

void BrowseFeature::bindWidget(WLibrarySidebar* sidebarWidget,
                               WLibrary* libraryWidget,
                               MixxxKeyboard* keyboard) {
    WBrowseTableView* pBrowseView = new WBrowseTableView(libraryWidget,
                                                         m_pConfig);

    connect(pBrowseView, SIGNAL(activated(const QModelIndex &)),
            this, SLOT(onFileActivate(const QModelIndex &)));
    connect(pBrowseView, SIGNAL(loadToPlayer(const QModelIndex&, QString)),
            this, SLOT(loadToPlayer(const QModelIndex&, QString)));
    connect(this, SIGNAL(setRootIndex(const QModelIndex&)),
            pBrowseView, SLOT(setRootIndex(const QModelIndex&)));
    connect(pBrowseView, SIGNAL(search(const QString&)),
            this, SLOT(search(const QString&)));
    connect(pBrowseView, SIGNAL(searchCleared()),
            this, SLOT(searchCleared()));
    connect(pBrowseView, SIGNAL(searchStarting()),
            this, SLOT(searchStarting()));

    pBrowseView->setDragEnabled(true);
    pBrowseView->setDragDropMode(QAbstractItemView::DragDrop);
    pBrowseView->setAcceptDrops(false);
    //pBrowseView->setModel(&m_proxyModel);

    QString startPath = m_pConfig->getValueString(ConfigKey("[Playlist]","Directory"));
    //m_browseModel.setRootPath(startPath);
    /*
    QModelIndex startIndex = m_browseModel.index(startPath);
    QModelIndex proxyIndex = m_proxyModel.mapFromSource(startIndex);
    emit(setRootIndex(proxyIndex));
    */
    libraryWidget->registerView("BROWSE", pBrowseView);
}

void BrowseFeature::activate() {
    emit(switchToView("BROWSE"));
    emit(restoreSearch(m_currentSearch));
}

void BrowseFeature::activateChild(const QModelIndex& index) {
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    qDebug() << "BrowseFeature::activateChild " << item->data() << " " << item->dataPath();
    
    //populate childs
    QDir dir(item->dataPath().toString());
    QFileInfoList all = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    // loop through all the item and construct the childs
    QList<TreeItem*> folders;
    foreach(QFileInfo one, all){
        //Skip folders that end with .app on OS X
        #if defined(__APPLE__)
        if(one.isDir() && one.fileName().endsWith(".app")) continue;
        #endif
        // We here create new items for the sidebar models
        // Once the items are added to the TreeItemModel, 
        // the models takes ownership of them and ensures their deletion
        TreeItem* folder = new TreeItem(one.fileName(), item->dataPath().toString().append(one.fileName() +"/"), this, item);
        folders << folder;  
    }
    //Before we populate the subtree, we need to delete old subtrees
    m_childModel.removeRows(0, item->childCount(), index);
    m_childModel.insertRows(folders, 0, folders.size() , index);
    /*
    QModelIndex startIndex = m_browseModel.index(item->dataPath().toString());
    QModelIndex proxyIndex = m_proxyModel.mapFromSource(startIndex);
    emit(setRootIndex(proxyIndex));
    emit(switchToView("BROWSE"));
    */
    //m_browseModel.setPath(item->dataPath().toString());
    //emit(switchToView("BROWSE"));
    emit(showTrackModel(&m_browseModel));

}

void BrowseFeature::onRightClick(const QPoint& globalPos) {
}

void BrowseFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
}

void BrowseFeature::onFileActivate(const QModelIndex& index) {
    /*
    QModelIndex sourceIndex = m_proxyModel.mapToSource(index);
    QFileInfo info = m_browseModel.fileInfo(sourceIndex);
    QString absPath = info.absoluteFilePath();

    if (m_browseModel.isDir(sourceIndex)) {
        if (!info.isReadable()) {
            // Alert that the user didn't have permissions.
            WBrowseTableView* pBrowseTableView = dynamic_cast<WBrowseTableView*>(sender());
            if (pBrowseTableView) {
                QMessageBox::warning(pBrowseTableView,
                                     tr("Permission Denied"),
                                     tr("You don't have permission to view this folder."));
            }
            return;
        }
        
        //Appending ".." to the path doesn't always work on Windows.
        //The right way to do it is to use QDir::cdUp(), we think... - Albert Nov 27, 2010
        if (info.fileName() == "..") {
            qDebug() << "cdUp";
            QDir rootDir = m_browseModel.rootDirectory();
            if (!rootDir.cdUp())
                return; //Parent does not exist.

            absPath = rootDir.absolutePath();
        }

        m_browseModel.setRootPath(absPath);

        QModelIndex absIndex = m_browseModel.index(absPath);
        QModelIndex absIndexProxy = m_proxyModel.mapFromSource(absIndex);
        emit(setRootIndex(absIndexProxy));
    } else {
        TrackDAO& trackDao = m_pTrackCollection->getTrackDAO();
        TrackPointer track = trackDao.getTrack(trackDao.getTrackId(absPath));

        // The track doesn't exist in the database.
        if (track == NULL) {
            track = TrackPointer(new TrackInfoObject(info), &QObject::deleteLater);
        }

        emit(loadTrack(track));
    }
    */
}

void BrowseFeature::loadToPlayer(const QModelIndex& index, QString group) {
    /*
    QModelIndex sourceIndex = m_proxyModel.mapToSource(index);
    QString path = m_browseModel.filePath(sourceIndex);
    QFileInfo info(path);
    QString absPath = info.absoluteFilePath();

    if (!m_browseModel.isDir(sourceIndex)) {
        TrackDAO& trackDao = m_pTrackCollection->getTrackDAO();
        TrackPointer track = trackDao.getTrack(trackDao.getTrackId(absPath));

        // The track doesn't exist in the database.
        if (track == NULL) {
            track = TrackPointer(new TrackInfoObject(info), &QObject::deleteLater);
        }

        emit(loadTrackToPlayer(track, group));
    }
    */
}

void BrowseFeature::searchStarting() {
    m_currentSearch = "";
}

void BrowseFeature::search(const QString& text) {
    m_currentSearch = text;
    //m_proxyModel.setFilterFixedString(text);
}

void BrowseFeature::searchCleared() {
    m_currentSearch = "";
    //m_proxyModel.setFilterFixedString("");
}
