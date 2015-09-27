#ifndef TREE_ITEM_MODEL_H
#define TREE_ITEM_MODEL_H

#include <functional>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QList>

class TreeItem;

class TreeItemModel : public QAbstractItemModel {
    Q_OBJECT
  public:
    static const int kDataPathRole = Qt::UserRole;
    static const int kBoldRole = Qt::UserRole + 1;

    TreeItemModel(QObject *parent = 0);
    virtual ~TreeItemModel();

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    // if m_fHasChildren was set up, this function uses passed fuction,
    // if it wasn't set up the default hasChildren function is used
    // watch out that non-default function doesn't use QModelIndex!!
    virtual bool hasChildren (const QModelIndex &index) const;
    void setHasChildrenFunction(std::function<bool(QVariant)> function);
    // Tell the compiler we don't mean to shadow insertRows.
    using QAbstractItemModel::insertRows;
    virtual bool insertRows(QList<TreeItem*> &data, int position, int rows, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());
    virtual bool setData(const QModelIndex &a_rIndex, const QVariant &a_rValue,
                         int a_iRole = Qt::EditRole);
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    void setRootItem(TreeItem *item);
    // Return the underlying TreeItem.
    // If the index is invalid, the root item is returned.
    TreeItem* getItem(const QModelIndex &index) const;

    void triggerRepaint();

  private:
    TreeItem *m_pRootItem;
    // This allows to use feature-based selection of hasChildren function
    // without excessive use of inheritance (derived class for each feature)
    // The function is set up by function setHasChildrenFunction
    std::function<bool(QVariant)> m_fHasChildren;
};

#endif
