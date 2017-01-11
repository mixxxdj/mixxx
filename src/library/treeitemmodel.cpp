#include <QLatin1String>
#include <QStringBuilder>

#include "library/treeitemmodel.h"

#include "util/stringhelper.h"

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
TreeItemModel::TreeItemModel(QObject* parent)
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

    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item == nullptr) {
        return QVariant();
    }

    // We use Qt::UserRole to ask for the datapath.    
    switch(role) {
        case Qt::DisplayRole:
            return item->getLabel();
        case Qt::SizeHintRole:
        {
            QIcon icon(item->getIcon());
            if (icon.isNull()) {
                return QVariant();
            }
            QSize size(getDefaultIconSize());
            size.setHeight(size.height() + 2);
            return size;
        }
        case Qt::DecorationRole:
            return item->getIcon();
        case AbstractRole::RoleDataPath:
            return item->getData();
        case AbstractRole::RoleBold:
            return item->isBold();
        case AbstractRole::RoleDivider:
            return item->isDivider();
        case AbstractRole::RoleBreadCrumb:
            return getBreadCrumbString(item);
        case AbstractRole::RoleGroupingLetter:
            return StringHelper::getFirstCharForGrouping(item->getData().toString());
    }

    return QVariant();
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
        case AbstractRole::RoleDataPath:
            pItem->setData(a_rValue);
            break;
        case AbstractRole::RoleBold:
            pItem->setBold(a_rValue.toBool());
            break;
        case AbstractRole::RoleDivider:
            pItem->setDivider(a_rValue.toBool());
            break;
        default:
            return false;
    }

    emit(dataChanged(a_rIndex, a_rIndex));
    return true;
}

Qt::ItemFlags TreeItemModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return 0;
    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    
    bool divider = index.data(AbstractRole::RoleDivider).toBool();
    if (!divider) {
        flags |= Qt::ItemIsSelectable;
    }
    
    return flags;
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
    if (parentItem == getRootItem()) {
        return QModelIndex();
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
    m_pRootItem = std::move(pRootItem);
    reset();
    return getRootItem();
}

/**
 * Before you can resize the data model dynamically by using 'insertRows' and 'removeRows'
 * make sure you have initialized
 */
bool TreeItemModel::insertRows(
        QList<TreeItem*>& data, int position, int rows, const QModelIndex &parent) {
    if (rows == 0) {
        return true;
    }
    TreeItem *parentItem = getItem(parent);

    beginInsertRows(parent, position, position + rows - 1);
    parentItem->insertChildren(data, position, rows);
    endInsertRows();

    return true;
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

void TreeItemModel::triggerRepaint() {
    QModelIndex left = index(0, 0);
    QModelIndex right = index(rowCount() - 1, columnCount() - 1);
    emit(dataChanged(left, right));
}

//static
QString TreeItemModel::getBreadCrumbString(TreeItem* pTree) {    
    // Base case
    if (pTree == nullptr || pTree->feature() == nullptr) {
        return QString();
    }
    else if (pTree->parent() == nullptr) {
        return pTree->feature()->title().toString();
    }
    
    // Recursive case
    QString text = pTree->getLabel();
    QString next = getBreadCrumbString(pTree->parent());
    return next % QLatin1String(" > ") % text;
}

//static
QSize TreeItemModel::getDefaultIconSize() {
    return QSize(32, 32);
}

void TreeItemModel::reloadTree() {
    triggerRepaint();
}

bool TreeItemModel::dropAccept(const QModelIndex& index, QList<QUrl> urls,
                               QObject* pSource) {
    //qDebug() << "TreeItemModel::dropAccept() index=" << index << urls;
    LibraryFeature* pFeature = getFeatureFromIndex(index);
    if (pFeature == nullptr) {
        return false;
    }
    
    return pFeature->dropAcceptChild(index, urls, pSource);
}

bool TreeItemModel::dragMoveAccept(const QModelIndex& index, QUrl url) {
    //qDebug() << "TreeItemModel::dragMoveAccept() index=" << index << url;    
    LibraryFeature* pFeature = getFeatureFromIndex(index);
    if (pFeature == nullptr) {
        return false;
    }
    
    return pFeature->dragMoveAcceptChild(index, url);
}

LibraryFeature* TreeItemModel::getFeatureFromIndex(const QModelIndex& index) const {
    TreeItem* pTree = getItem(index);
    if (pTree == nullptr) {
        return nullptr;
    }
    return pTree->feature();
}
