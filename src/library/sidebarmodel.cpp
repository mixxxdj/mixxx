#include "library/sidebarmodel.h"

#include <QApplication>
#include <QUrl>
#include <QtDebug>

#include "library/browse/browsefeature.h"
#include "library/libraryfeature.h"
#include "library/treeitem.h"
#include "moc_sidebarmodel.cpp"
#include "util/assert.h"

namespace {

// The time between selecting and activating (= clicking) a feature item
// in the sidebar tree. This is essential to allow smooth scrolling through
// a list of items with an encoder or the keyboard! A value of 300 ms has
// been chosen as a compromise between usability and responsiveness.
const int kPressedUntilClickedTimeoutMillis = 300;

const QHash<int, QByteArray> kRoleNames = {
        // Only roles that are useful in QML are added here.
        {Qt::DisplayRole, "display"},
        {Qt::ToolTipRole, "tooltip"},
        {SidebarModel::IconNameRole, "iconName"},
};

} // anonymous namespace

SidebarModel::SidebarModel(
        QObject* parent)
        : QAbstractItemModel(parent),
          m_iDefaultSelectedIndex(0),
          m_pressedUntilClickedTimer(new QTimer(this)) {
    m_pressedUntilClickedTimer->setSingleShot(true);
    connect(m_pressedUntilClickedTimer,
            &QTimer::timeout,
            this,
            &SidebarModel::slotPressedUntilClickedTimeout);
}

void SidebarModel::addLibraryFeature(LibraryFeature* pFeature) {
    m_sFeatures.push_back(pFeature);
    connect(pFeature,
            &LibraryFeature::featureIsLoading,
            this,
            &SidebarModel::slotFeatureIsLoading);
    connect(pFeature,
            &LibraryFeature::featureLoadingFinished,
            this,
            &SidebarModel::slotFeatureLoadingFinished);
    connect(pFeature,
            &LibraryFeature::featureSelect,
            this,
            &SidebarModel::slotFeatureSelect);

    QAbstractItemModel* model = pFeature->sidebarModel();

    connect(model,
            &QAbstractItemModel::modelAboutToBeReset,
            this,
            &SidebarModel::slotModelAboutToBeReset);
    connect(model,
            &QAbstractItemModel::modelReset,
            this,
            &SidebarModel::slotModelReset);
    connect(model,
            &QAbstractItemModel::dataChanged,
            this,
            &SidebarModel::slotDataChanged);

    connect(model,
            &QAbstractItemModel::rowsAboutToBeInserted,
            this,
            &SidebarModel::slotRowsAboutToBeInserted);
    connect(model,
            &QAbstractItemModel::rowsAboutToBeRemoved,
            this,
            &SidebarModel::slotRowsAboutToBeRemoved);
    connect(model,
            &QAbstractItemModel::rowsInserted,
            this,
            &SidebarModel::slotRowsInserted);
    connect(model,
            &QAbstractItemModel::rowsRemoved,
            this,
            &SidebarModel::slotRowsRemoved);
}

QModelIndex SidebarModel::getDefaultSelection() {
    if (m_sFeatures.size() == 0) {
        return QModelIndex();
    }
    return createIndex(m_iDefaultSelectedIndex, 0, this);
}

void SidebarModel::setDefaultSelection(unsigned int index) {
    m_iDefaultSelectedIndex = index;
}

void SidebarModel::activateDefaultSelection() {
    if (m_iDefaultSelectedIndex <
            static_cast<unsigned int>(m_sFeatures.size())) {
        emit selectIndex(getDefaultSelection());
        // Selecting an index does not activate it.
        m_sFeatures[m_iDefaultSelectedIndex]->activate();
    }
}

QModelIndex SidebarModel::index(int row, int column,
                                const QModelIndex& parent) const {
    // qDebug() << "SidebarModel::index row=" << row
      //       << "column=" << column << "parent=" << parent.getData();
    if (parent.isValid()) {
        /* If we have selected the root of a library feature at position 'row'
         * its internal pointer is the current sidebar object model
         * we return its associated childmodel
         */
        if (parent.internalPointer() == this) {
            const QAbstractItemModel* childModel = m_sFeatures[parent.row()]->sidebarModel();
            QModelIndex childIndex = childModel->index(row, column);
            TreeItem* pTreeItem = static_cast<TreeItem*>(childIndex.internalPointer());
            if (pTreeItem && childIndex.isValid()) {
                return createIndex(childIndex.row(), childIndex.column(), pTreeItem);
            } else {
                return QModelIndex();
            }
        } else {
            // We have selected an item within the childmodel
            // This item has always an internal pointer of (sub)type TreeItem
            TreeItem* pTreeItem = static_cast<TreeItem*>(parent.internalPointer());
            if (row < pTreeItem->childRows()) {
                return createIndex(row, column, pTreeItem->child(row));
            } else {
                // Otherwise this row might have been removed just now
                // (just a dirty workaround for unmaintainable GUI code)
                return QModelIndex();
            }
        }
    }

    // `this` is const, but the function expects a non-const pointer.
    // TODO: Check if we can get rid of this const cast somehow.
    return createIndex(row, column, const_cast<SidebarModel*>(this));
}

QModelIndex SidebarModel::parent(const QModelIndex& index) const {
    //qDebug() << "SidebarModel::parent index=" << index.getData();
    if (index.isValid()) {
        // If we have selected the root of a library feature
        // its internal pointer is the current sidebar object model
        // A root library feature has no parent and thus we return
        // an invalid QModelIndex
        if (index.internalPointer() == this) {
            return QModelIndex();
        } else {
            TreeItem* pTreeItem = static_cast<TreeItem*>(index.internalPointer());
            if (pTreeItem == nullptr) {
                return QModelIndex();
            }
            TreeItem* pTreeItemParent = pTreeItem->parent();
            // if we have selected an item at the first level of a childnode

            if (pTreeItemParent) {
                if (pTreeItemParent->isRoot()) {
                    LibraryFeature* pFeature = pTreeItem->feature();
                    for (int i = 0; i < m_sFeatures.size(); ++i) {
                        if (pFeature == m_sFeatures[i]) {
                            // create a ModelIndex for parent 'this' having a
                            // library feature at position 'i'
                            // `this` is const, but the function expects a
                            // non-const pointer.
                            // TODO: Check if we can get rid of this const cast
                            // somehow.
                            return createIndex(i, 0, const_cast<SidebarModel*>(this));
                        }
                    }
                }
                // if we have selected an item at some deeper level of a childnode
                return createIndex(pTreeItemParent->parentRow(), 0, pTreeItemParent);
            }
        }
    }
    return QModelIndex();
}

int SidebarModel::rowCount(const QModelIndex& parent) const {
    //qDebug() << "SidebarModel::rowCount parent=" << parent.getData();
    if (parent.isValid()) {
        if (parent.internalPointer() == this) {
            return m_sFeatures[parent.row()]->sidebarModel()->rowCount();
        } else {
            // We support tree models deeper than 1 level
            TreeItem* pTreeItem = static_cast<TreeItem*>(parent.internalPointer());
            if (pTreeItem) {
                return pTreeItem->childRows();
            }
            return 0;
        }
    }
    return m_sFeatures.size();
}

int SidebarModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    //qDebug() << "SidebarModel::columnCount parent=" << parent;
    // TODO(rryan) will we ever have columns? I don't think so.
    return 1;
}

bool SidebarModel::hasChildren(const QModelIndex& parent) const {
    if (parent.isValid()) {
        if (parent.internalPointer() == this) {
            return QAbstractItemModel::hasChildren(parent);
        } else {
            TreeItem* pTreeItem = static_cast<TreeItem*>(parent.internalPointer());
            if (pTreeItem) {
                LibraryFeature* pFeature = pTreeItem->feature();
                return pFeature->sidebarModel()->hasChildren(parent);
            }
        }
    }

    return QAbstractItemModel::hasChildren(parent);
}

QVariant SidebarModel::data(const QModelIndex& index, int role) const {
    // qDebug("SidebarModel::data row=%d column=%d pointer=%8x, role=%d",
    //        index.row(), index.column(), index.internalPointer(), role);
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.internalPointer() == this) {
        //If it points to SidebarModel
        switch (role) {
        case Qt::DisplayRole:
            return m_sFeatures[index.row()]->title();
        case Qt::DecorationRole:
            return m_sFeatures[index.row()]->icon();
        case SidebarModel::IconNameRole:
            return m_sFeatures[index.row()]->iconName();
        default:
            return QVariant();
        }
    } else {
        // If it points to a TreeItem
        TreeItem* pTreeItem = static_cast<TreeItem*>(index.internalPointer());
        if (!pTreeItem) {
            return QVariant();
        }

        switch (role) {
        case Qt::DisplayRole:
            return pTreeItem->getLabel();
        case Qt::ToolTipRole: {
            // If it's the "Quick Links" node, display it's name
            if (pTreeItem->getData().toString() == QUICK_LINK_NODE) {
                return pTreeItem->getLabel();
            }
            return pTreeItem->getData();
        }
        case Qt::FontRole: {
            QFont font;
            font.setBold(pTreeItem->isBold());
            return font;
        }
        case Qt::DecorationRole:
            return pTreeItem->getIcon();
        case SidebarModel::DataRole:
            return pTreeItem->getData();
        case SidebarModel::IconNameRole:
            // TODO: Add support for icon names in tree items
        default:
            return QVariant();
        }
    }
}

QHash<int, QByteArray> SidebarModel::roleNames() const {
    return kRoleNames;
}

void SidebarModel::startPressedUntilClickedTimer(const QModelIndex& pressedIndex) {
    m_pressedIndex = pressedIndex;
    m_pressedUntilClickedTimer->start(kPressedUntilClickedTimeoutMillis);
}

void SidebarModel::stopPressedUntilClickedTimer() {
    m_pressedUntilClickedTimer->stop();
    m_pressedIndex = QModelIndex();
}

void SidebarModel::slotPressedUntilClickedTimeout() {
    if (m_pressedIndex.isValid()) {
        QModelIndex clickedIndex = m_pressedIndex;
        stopPressedUntilClickedTimer();
        clicked(clickedIndex);
    }
}

void SidebarModel::pressed(const QModelIndex& index) {
    stopPressedUntilClickedTimer();
    if (index.isValid()) {
        startPressedUntilClickedTimer(index);
    }
}

void SidebarModel::clicked(const QModelIndex& index) {
    // When triggered by a mouse event pressed() has been
    // invoked immediately before. That doesn't matter,
    // because we stop any running timer before handling
    // this event.
    stopPressedUntilClickedTimer();
    if (index.isValid()) {
        if (index.internalPointer() == this) {
            m_sFeatures[index.row()]->activate();
        } else {
            TreeItem* pTreeItem = static_cast<TreeItem*>(index.internalPointer());
            if (pTreeItem) {
                LibraryFeature* pFeature = pTreeItem->feature();
                DEBUG_ASSERT(pFeature);
                pFeature->activateChild(index);
            }
        }
    }
}

void SidebarModel::doubleClicked(const QModelIndex& index) {
    stopPressedUntilClickedTimer();
    if (index.isValid()) {
        if (index.internalPointer() == this) {
           return;
        } else {
            TreeItem* pTreeItem = static_cast<TreeItem*>(index.internalPointer());
            if (pTreeItem) {
                LibraryFeature* pFeature = pTreeItem->feature();
                pFeature->onLazyChildExpandation(index);
            }
        }
    }
}

void SidebarModel::rightClicked(const QPoint& globalPos, const QModelIndex& index) {
    stopPressedUntilClickedTimer();
    if (index.isValid()) {
        if (index.internalPointer() == this) {
            m_sFeatures[index.row()]->onRightClick(globalPos);
        }
        else
        {
            TreeItem* pTreeItem = static_cast<TreeItem*>(index.internalPointer());
            if (pTreeItem) {
                LibraryFeature* pFeature = pTreeItem->feature();
                pFeature->onRightClickChild(globalPos, index);
            }
        }
    }
}

bool SidebarModel::dropAccept(const QModelIndex& index, const QList<QUrl>& urls, QObject* pSource) {
    //qDebug() << "SidebarModel::dropAccept() index=" << index << url;
    bool result = false;
    if (index.isValid()) {
        if (index.internalPointer() == this) {
            result = m_sFeatures[index.row()]->dropAccept(urls, pSource);
        } else {
            TreeItem* pTreeItem = static_cast<TreeItem*>(index.internalPointer());
            if (pTreeItem) {
                LibraryFeature* pFeature = pTreeItem->feature();
                result = pFeature->dropAcceptChild(index, urls, pSource);
            }
        }
    }
    return result;
}

bool SidebarModel::hasTrackTable(const QModelIndex& index) const {
    if (index.internalPointer() == this) {
     return m_sFeatures[index.row()]->hasTrackTable();
    }
    return false;
}

bool SidebarModel::dragMoveAccept(const QModelIndex& index, const QUrl& url) {
    //qDebug() << "SidebarModel::dragMoveAccept() index=" << index << url;
    bool result = false;

    if (index.isValid()) {
        if (index.internalPointer() == this) {
            result = m_sFeatures[index.row()]->dragMoveAccept(url);
        } else {
            TreeItem* pTreeItem = static_cast<TreeItem*>(index.internalPointer());
            if (pTreeItem) {
                LibraryFeature* pFeature = pTreeItem->feature();
                result = pFeature->dragMoveAcceptChild(index, url);
            }
        }
    }
    return result;
}

// Translates an index from the child models to an index of the sidebar models
QModelIndex SidebarModel::translateSourceIndex(const QModelIndex& index) {
    /* These method is called from the slot functions below.
     * QObject::sender() return the object which emitted the signal
     * handled by the slot functions.

     * For child models, this always the child models itself
     */

    const QAbstractItemModel* model = qobject_cast<QAbstractItemModel*>(sender());
    VERIFY_OR_DEBUG_ASSERT(model != nullptr) {
        return QModelIndex();
    }

    return translateIndex(index, model);
}

QModelIndex SidebarModel::translateIndex(
        const QModelIndex& index, const QAbstractItemModel* pModel) {
    QModelIndex translatedIndex;

    if (index.isValid()) {
        TreeItem* pItem = static_cast<TreeItem*>(index.internalPointer());
        translatedIndex = createIndex(index.row(), index.column(), pItem);
    } else {
        //Comment from Tobias Rafreider --> Dead Code????

        for (int i = 0; i < m_sFeatures.size(); ++i) {
            if (m_sFeatures[i]->sidebarModel() == pModel) {
                translatedIndex = createIndex(i, 0, this);
            }
        }
    }
    return translatedIndex;
}

void SidebarModel::slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight) {
    //qDebug() << "slotDataChanged topLeft:" << topLeft << "bottomRight:" << bottomRight;
    QModelIndex topLeftTranslated = translateSourceIndex(topLeft);
    QModelIndex bottomRightTranslated = translateSourceIndex(bottomRight);
    emit dataChanged(topLeftTranslated, bottomRightTranslated);
}

void SidebarModel::slotRowsAboutToBeInserted(const QModelIndex& parent, int start, int end) {
    //qDebug() << "slotRowsABoutToBeInserted" << parent << start << end;

    QModelIndex newParent = translateSourceIndex(parent);
    beginInsertRows(newParent, start, end);
}

void SidebarModel::slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end) {
    //qDebug() << "slotRowsABoutToBeRemoved" << parent << start << end;

    QModelIndex newParent = translateSourceIndex(parent);
    beginRemoveRows(newParent, start, end);
}

void SidebarModel::slotRowsInserted(const QModelIndex& parent, int start, int end) {
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    //qDebug() << "slotRowsInserted" << parent << start << end;
    //QModelIndex newParent = translateSourceIndex(parent);
    endInsertRows();
}

void SidebarModel::slotRowsRemoved(const QModelIndex& parent, int start, int end) {
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    //qDebug() << "slotRowsRemoved" << parent << start << end;
    //QModelIndex newParent = translateSourceIndex(parent);
    endRemoveRows();
}

void SidebarModel::slotModelAboutToBeReset() {
    beginResetModel();
}

void SidebarModel::slotModelReset() {
    endResetModel();
}

/*
 * Call this slot whenever the title of the feature has changed.
 * See RhythmboxFeature for an example, in which the title becomes '(loading) Rhythmbox'
 * If selectFeature is true, the feature is selected when the title change occurs.
 */
void SidebarModel::slotFeatureIsLoading(LibraryFeature* pFeature, bool selectFeature) {
    featureRenamed(pFeature);
    if (selectFeature) {
        slotFeatureSelect(pFeature);
    }
}

/* Tobias: This slot is somewhat redundant but I decided
 * to leave it for code readability reasons
 */
void SidebarModel::slotFeatureLoadingFinished(LibraryFeature* pFeature) {
    featureRenamed(pFeature);
    slotFeatureSelect(pFeature);
}

void SidebarModel::featureRenamed(LibraryFeature* pFeature) {
    for (int i=0; i < m_sFeatures.size(); ++i) {
        if (m_sFeatures[i] == pFeature) {
            QModelIndex ind = index(i, 0);
            emit dataChanged(ind, ind);
        }
    }
}

void SidebarModel::slotFeatureSelect(LibraryFeature* pFeature, const QModelIndex& featureIndex) {
    QModelIndex ind;
    if (featureIndex.isValid()) {
        TreeItem* pTreeItem = static_cast<TreeItem*>(featureIndex.internalPointer());
        ind = createIndex(featureIndex.row(), featureIndex.column(), pTreeItem);
    } else {
        for (int i=0; i < m_sFeatures.size(); ++i) {
            if (m_sFeatures[i] == pFeature) {
                ind = index(i, 0);
                break;
            }
        }
    }
    emit selectIndex(ind);
}
