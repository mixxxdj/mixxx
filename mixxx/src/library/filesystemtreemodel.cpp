#include <QtGui>
#include <QFileInfo>

 #include "treeitem.h"
 #include "filesystemtreemodel.h"

  FileSystemTreeModel::FileSystemTreeModel(QObject *parent, LibraryFeature* pfeature)
     : TreeItemModel(parent), m_browser(pfeature)
 {
    //The invisible root item of the model
     m_rootItem = new TreeItem("$root","$root");
     
    #if defined(__WINDOWS__)
    QFileInfoList drives = QDir::drives();
    //show drive letters
    foreach(QFileInfo drive, drives){
        TreeItem* driveLetter = new TreeItem(drive.fileName(), drive.fileName(), m_browser , m_rootItem);
        m_rootItem->appendChild(driveLetter);
    }
    #else
    //show root directory on UNIX-based operating systems
    TreeItem* driveLetter = new TreeItem(QDir::rootPath(), QDir::rootPath(), m_browser , m_rootItem);
    m_rootItem->appendChild(driveLetter);
    #endif

 }

 FileSystemTreeModel::~FileSystemTreeModel()
 {
    //Destroy the underlying tree structure
    delete m_rootItem;
 }
//Our Treeview Model supports exactly a single column
 int FileSystemTreeModel::columnCount(const QModelIndex &parent) const
 {
     return 1;
 }

 QVariant FileSystemTreeModel::data(const QModelIndex &index, int role) const
 {
     if (!index.isValid())
         return QVariant();

     if (role != Qt::DisplayRole)
         return QVariant();
      //TODO: File icon handling

     TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

     return item->data();
 }

 Qt::ItemFlags FileSystemTreeModel::flags(const QModelIndex &index) const
 {
     if (!index.isValid())
         return 0;

     return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
 }

 QVariant FileSystemTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
 {
     return QVariant();
 }

 QModelIndex FileSystemTreeModel::index(int row, int column, const QModelIndex &parent)
             const
 {
     if (!hasIndex(row, column, parent))
         return QModelIndex();

     TreeItem *parentItem = NULL;

     if (!parent.isValid())
         parentItem = m_rootItem;
     else
         parentItem = static_cast<TreeItem*>(parent.internalPointer());

     TreeItem *childItem = parentItem->child(row);
     if (childItem)
         return createIndex(row, column, childItem);
     else
         return QModelIndex();
 }

 QModelIndex FileSystemTreeModel::parent(const QModelIndex &index) const
 {
     if (!index.isValid())
         return QModelIndex();

     TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
     TreeItem *parentItem = childItem->parent();

     if (parentItem == m_rootItem)
         return QModelIndex();

     return createIndex(parentItem->row(), 0, parentItem);
 }

 int FileSystemTreeModel::rowCount(const QModelIndex &parent) const
 {
    if (parent.column() > 0)
         return 0;


    TreeItem *parentItem = NULL;
    //qDebug() << "parent data: " << parent.data();
     if (!parent.isValid()){
         parentItem = m_rootItem;
    }
     else{
         parentItem = static_cast<TreeItem*>(parent.internalPointer());

    }
 
    //qDebug() << "TreeItem data: " << parent.internalPointer();

     return parentItem->childCount();
 }

/**
  * Before you can resize the data model dynamically by using 'insertRows' and 'removeRows' 
  * make sure you have initialized 
  */ 
bool FileSystemTreeModel::insertRows(QList<QString>& data, int position, int rows, const QModelIndex &parent)
 {  
     TreeItem *parentItem = getItem(parent);
     bool success;

     beginInsertRows(parent, position, position + rows - 1);
     success = parentItem->insertChildren(data, position, rows);
     endInsertRows();

     return success;
 }
 bool FileSystemTreeModel::removeRows(int position, int rows, const QModelIndex &parent)
 {
     TreeItem *parentItem = getItem(parent);
     bool success = true;

     beginRemoveRows(parent, position, position + rows - 1);
     success = parentItem->removeChildren(position, rows);
     endRemoveRows();

     return success;
 }




