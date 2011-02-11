#include <QtGui>
#include <QDirModel>
#include <QStringList>
#include <QFileInfo>
#include <QDesktopServices>

#include "treeitem.h"
#include "foldertreemodel.h"
#include "browsefeature.h"

#if defined (__WINDOWS__)
    #include <windows.h>
    #include <Shellapi.h>
    #include <Shobjidl.h>
#else
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <dirent.h>
    #include <unistd.h>
    #include <errno.h>
#endif



 FolderTreeModel::FolderTreeModel(QObject *parent)
     : TreeItemModel(parent)
{


}

 FolderTreeModel::~FolderTreeModel()
 {
     
 }
 /* A tree model of the filesystem should be initialized lazy.
  * It will take the universe to iterate over all files over filesystem
  * hasChildren() returns true if a folder has subfolders although 
  * we do not know the precise number of subfolders.
  *
  * Note that BrowseFeature inserts folder trees dynamically and rowCount()
  * is only called if necessary.
  */
bool FolderTreeModel::hasChildren( const QModelIndex & parent) const
{

    TreeItem *item = static_cast<TreeItem*>(parent.internalPointer());
    /* Usually the child count is 0 becuase we do lazy initalization
     * However, for, buid-in items such as 'Quick Links' there exist
     * child items at init time
     */
    if(item->dataPath().toString() == QUICK_LINK_NODE)
        return true;
    if(item->dataPath().toString() == DEVICE_NODE)
        return true;

    QString folder = item->dataPath().toString();

    /*
     *  The following code is too expensive, general and SLOW since
     *  QDIR::EntryInfoList returns a full QFileInfolist
     *
     *
     *  QDir dir(item->dataPath().toString());
     *  QFileInfoList all = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
     *  return (all.count() > 0);
     *
     *  We can benefit from low-level filesystem APIs, i.e.,
     *  Windows API or SystemCalls
     */
	
#if defined (__WINDOWS__)
    folder.replace("/","\\");

    //quick subfolder test
    SHFILEINFOW sfi;
    SHGetFileInfo((LPCWSTR) folder.constData(), NULL, &sfi, sizeof(sfi), SHGFI_ATTRIBUTES);
    return (sfi.dwAttributes & SFGAO_HASSUBFOLDER);
#else
    // For OS X and Linux
    // http://stackoverflow.com/questions/2579948/checking-if-subfolders-exist-linux

    std::string dot("."), dotdot("..");
    bool found_subdir = false;
    DIR *directory = opendir(folder.toStdString().c_str());

    if (directory == NULL){
        return false;
    }
    struct dirent *entry;
    while (!found_subdir && ((entry = readdir(directory)) != NULL)) {
        if (entry->d_name != dot && entry->d_name != dotdot) 
        {
            found_subdir = (entry->d_type == DT_DIR || entry->d_type == DT_LNK);
            //qDebug() << "Subfolder of " << folder << " : " << entry->d_name << "type :" << entry->d_type; 
            
        }
    }
    closedir(directory);
    return found_subdir;

#endif
    
}
bool FolderTreeModel::canFetchMore(const QModelIndex &parent) const
{
    qDebug() << "FolderTreemodel::canFetchMore :" << parent.data() << " " <<QAbstractItemModel::canFetchMore(parent);

    TreeItem *item = static_cast<TreeItem*>(parent.internalPointer());
    if(item->dataPath().toString() == QUICK_LINK_NODE)
        return QAbstractItemModel::canFetchMore(parent);


    //return hasChildren(parent);
     return QAbstractItemModel::canFetchMore(parent);

}
void FolderTreeModel::fetchMore(const QModelIndex &parent)
{
    qDebug() << "FolderTreemodel::fetchMore :" << parent.data();

    QModelIndex index = parent;
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    // If the item is a build-in node, e.g., 'QuickLink' return
    if(item->dataPath().toString() == QUICK_LINK_NODE)
        return;

    //Before we populate the subtree, we need to delete old subtrees
   removeRows(0, item->childCount(), index);

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
                            item->getFeature() ,
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
            if(one.isDir() && one.fileName().endsWith(".app")) continue;
            #endif
            // We here create new items for the sidebar models
            // Once the items are added to the TreeItemModel,
            // the models takes ownership of them and ensures their deletion
            TreeItem* folder = new TreeItem(one.fileName(),
                                            item->dataPath().toString().append(one.fileName() +"/"),
                                           item->getFeature(),
                                            item);
            folders << folder;
        }
    }
    //we need to check here if subfolders are found
    //On Ubuntu 10.04, otherwise, this will draw an icon although the folder has no subfolders
    qDebug() << "HERERERRT";
    if(!folders.isEmpty())
       insertRows(folders, 0, folders.size() , index);
    qDebug() << "HERERERRT22222222222";
}
