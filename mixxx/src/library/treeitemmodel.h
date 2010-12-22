#ifndef TAKTOR_SIDEBAR_MODEL_H
 #define TAKTOR_SIDEBAR_MODEL_H

 #include <QAbstractItemModel>
 #include <QModelIndex>
 #include <QVariant>
 #include <QList>



 class TreeItem;

 class TreeItemModel : public QAbstractItemModel
 {
     Q_OBJECT

 public:
     TreeItemModel(QObject *parent = 0);
     ~TreeItemModel();

     QVariant data(const QModelIndex &index, int role) const;
     Qt::ItemFlags flags(const QModelIndex &index) const;
     QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
     QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
     QModelIndex parent(const QModelIndex &index) const;

     bool insertRows(QList<QString>& data, int position, int rows, const QModelIndex &parent = QModelIndex());
     bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());

     int rowCount(const QModelIndex &parent = QModelIndex()) const;
     int columnCount(const QModelIndex &parent = QModelIndex()) const;
     void setRootItem(TreeItem *item);
     TreeItem* getItem(const QModelIndex &index) const;
    

 private:
     TreeItem *m_rootItem;
     
 };

 #endif
