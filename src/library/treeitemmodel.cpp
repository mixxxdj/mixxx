#include "library/treeitemmodel.h"

#include "library/treeitem.h"
#include "moc_treeitemmodel.cpp"

/*
 * Just a word about how the TreeItem objects and TreeItemModels are used in general:
 * TreeItems are used by the TreeItemModel class to display tree
 * structures in the sidebar.
 *
 * The constructor has 4 arguments:
 * 1. argument represents a name shown in the sidebar view later on
 * 2. argument represents the absolute path of this tree item
 * 3. argument is a library feature object.
 *    This is necessary because in sidebar.cpp we handle 'activateChid' events
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
          m_pRootItem(std::make_unique<TreeItem>()) {
}

TreeItemModel::~TreeItemModel() {
}

//Our Treeview Model supports exactly a single column
int TreeItemModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 1;
}

QVariant TreeItemModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    // We use Qt::UserRole to ask for the data.
    switch (role) {
    case Qt::DisplayRole:
        return item->getLabel();
    case kDataRole:
        return item->getData();
    case kBoldRole:
        return item->isBold();
    default:
        return QVariant();
    }
}

bool TreeItemModel::setData(const QModelIndex &a_rIndex,
                            const QVariant &a_rValue, int a_iRole) {
    // Get the item referred to by this index.
    TreeItem *pItem = static_cast<TreeItem*>(a_rIndex.internalPointer());
    if (pItem == nullptr) {
        return false;
    }

    // Set the relevant data.
    switch (a_iRole) {
    case Qt::DisplayRole:
        pItem->setLabel(a_rValue.toString());
        break;
    case kDataRole:
        pItem->setData(a_rValue);
        break;
    case kBoldRole:
        pItem->setBold(a_rValue.toBool());
        break;
    default:
        return false;
    }

    emit dataChanged(a_rIndex, a_rIndex);
    return true;
}

Qt::ItemFlags TreeItemModel::flags(const QModelIndex &index) const {
    if (index.isValid()) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    } else {
        return Qt::NoItemFlags;
    }
}

QVariant TreeItemModel::headerData(int section, Qt::Orientation orientation, int role) const {
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return QVariant();
}

QModelIndex TreeItemModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    TreeItem *parentItem;
    if (parent.isValid()) {
        parentItem = static_cast<TreeItem*>(parent.internalPointer());
    } else {
        parentItem = getRootItem();
    }

    TreeItem *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return QModelIndex();
    }
}

QModelIndex TreeItemModel::parent(const QModelIndex& index) const {
    if (!index.isValid()) {
        return QModelIndex();
    }

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();
    if (!parentItem) {
        return QModelIndex();
    } else if (parentItem == getRootItem()) {
        return createIndex(0, 0, getRootItem());
    } else {
        return createIndex(parentItem->parentRow(), 0, parentItem);
    }
}

int TreeItemModel::rowCount(const QModelIndex& parent) const {
    if (parent.column() > 0) {
        return 0;
    }

    TreeItem* parentItem;
    if (parent.isValid()) {
        parentItem = static_cast<TreeItem*>(parent.internalPointer());
    } else {
        parentItem = getRootItem();
    }
    return parentItem->childRows();
}

/**
 * Populates the model and notifies the view.
 * Call this method first, before you do call any other methods.
 */
TreeItem* TreeItemModel::setRootItem(std::unique_ptr<TreeItem> pRootItem) {
    beginResetModel();
    m_pRootItem = std::move(pRootItem);
    endResetModel();
    return getRootItem();
}

const QModelIndex TreeItemModel::getRootIndex() {
    return createIndex(0, 0, getRootItem());
}

/**
 * Before you can resize the data model dynamically by using 'insertRows' and 'removeRows'
 * make sure you have initialized
 */
void TreeItemModel::insertTreeItemRows(
        QList<TreeItem*>& rows,
        int position,
        const QModelIndex& parent) {
    if (rows.isEmpty()) {
        return;
    }

    TreeItem* pParentItem = getItem(parent);
    DEBUG_ASSERT(pParentItem != nullptr);

    beginInsertRows(parent, position, position + rows.size() - 1);
    pParentItem->insertChildren(position, rows);
    DEBUG_ASSERT(rows.isEmpty());
    endInsertRows();
}

bool TreeItemModel::removeRows(int position, int rows, const QModelIndex &parent) {
    if (rows == 0) {
        return true;
    }
    TreeItem *parentItem = getItem(parent);

    beginRemoveRows(parent, position, position + rows - 1);
    parentItem->removeChildren(position, rows);
    endRemoveRows();

    return true;
}

TreeItem* TreeItemModel::getItem(const QModelIndex &index) const {
    if (index.isValid()) {
        TreeItem* pItem = static_cast<TreeItem*>(index.internalPointer());
        if (pItem != nullptr) {
            return pItem;
        }
    }
    return getRootItem();
}

void TreeItemModel::triggerRepaint(const QModelIndex& index) {
    emit dataChanged(index, index);
}

void TreeItemModel::triggerRepaint() {
    QModelIndex left = index(0, 0);
    QModelIndex right = index(rowCount() - 1, columnCount() - 1);
    emit dataChanged(left, right);
}
