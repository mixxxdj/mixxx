#ifndef FILE_SYSTEM_TREE_MODEL_H
 #define FILE_SYSTEM_TREE_MODEL_H

 #include <QAbstractItemModel>
 #include <QModelIndex>
 #include <QVariant>
 #include <QList>

#include "treeitemmodel.h"
#include "library/browsefeature.h"

 class FileSystemTreeModel : public TreeItemModel
 {
     Q_OBJECT

 public:
     FileSystemTreeModel(QObject *parent = 0, LibraryFeature* pfeature = 0);
     virtual ~FileSystemTreeModel();

     virtual QVariant data(const QModelIndex &index, int role) const;
     virtual Qt::ItemFlags flags(const QModelIndex &index) const;
     virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
     virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
     virtual QModelIndex parent(const QModelIndex &index) const;

     virtual bool insertRows(QList<QString>& data, int position, int rows, const QModelIndex &parent = QModelIndex());
     virtual bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());

     virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
     virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    
    

 private:
     LibraryFeature* m_browser;
     
 };

 #endif
