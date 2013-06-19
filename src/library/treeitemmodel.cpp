#include <QtGui>

#include "library/treeitem.h"
#include "library/treeitemmodel.h"

/*
 * Just a word about how the TreeItem objects and TreeItemModels are used in general:
 * TreeItems are used by the TreeItemModel class to display tree
 * structures in the sidebar.
 *
 * The constructor has 4 arguments:
 * 1. argument represents a name shown in the sidebar view later on
 * 2. argument represents the absolute path of this tree item
 * 3. argument is a library feature object.
 *    This is necessary because in sidebar.cpp we hanlde 'activateChid' events
 * 4. the parent TreeItem object
 *    The constructor does not add this TreeItem object to the parent's child list
 *
 * In case of no arguments, the standard constructor creates a
 * root item that is not visible in the sidebar.
 *
 * Once the TreeItem objects are inserted to models, the models take care of their
 * deletion.
 *
 * Examples on how to use TreeItem and TreeItemModels can be found in
 * - playlistfeature.cpp
 * - cratefeature.cpp
 * - *feature.cpp
 */
TreeItemModel::TreeItemModel(QObject *parent)
        : QAbstractItemModel(parent),
          m_pRootItem(new TreeItem()) {
}

TreeItemModel::~TreeItemModel() {
    delete m_pRootItem;
}

//Our Treeview Model supports exactly a single column
int TreeItemModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return 1;
}

QVariant TreeItemModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::UserRole)
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    // We use Qt::UserRole to ask for the datapath.
    if (role == Qt::UserRole) {
        return item->dataPath();
    }
    return item->data();
}

Qt::ItemFlags TreeItemModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant TreeItemModel::headerData(int section, Qt::Orientation orientation, int role) const {
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return QVariant();
}

QModelIndex TreeItemModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem = NULL;

    if (!parent.isValid())
        parentItem = m_pRootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TreeItemModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == m_pRootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TreeItemModel::rowCount(const QModelIndex &parent) const {
    if (parent.column() > 0)
        return 0;

    TreeItem *parentItem = NULL;
    //qDebug() << "parent data: " << parent.data();
    if (!parent.isValid()){
        parentItem = m_pRootItem;
    }
    else{
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    }

    //qDebug() << "TreeItem data: " << parent.internalPointer();

    return parentItem->childCount();
}

/**
 * Populates the model and notifies the view.
 * Call this method first, before you do call any other methods.
 */
void TreeItemModel::setRootItem(TreeItem *item) {
    if(m_pRootItem) delete m_pRootItem;

    m_pRootItem = item;
    reset();
}

/**
 * Before you can resize the data model dynamically by using 'insertRows' and 'removeRows'
 * make sure you have initialized
 */
bool TreeItemModel::insertRows(QList<TreeItem*> &data, int position, int rows, const QModelIndex &parent) {
    TreeItem *parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(data, position, rows);
    endInsertRows();

    return success;
}

bool TreeItemModel::removeRows(int position, int rows, const QModelIndex &parent) {
    TreeItem *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

TreeItem* TreeItemModel::getItem(const QModelIndex &index) const {
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item) return item;
    }
    return m_pRootItem;
}
