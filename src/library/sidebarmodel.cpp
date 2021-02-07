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

void SidebarModel::addLibraryFeature(LibraryFeature* feature) {
    m_sFeatures.push_back(feature);
    connect(feature,
            &LibraryFeature::featureIsLoading,
            this,
            &SidebarModel::slotFeatureIsLoading);
    connect(feature,
            &LibraryFeature::featureLoadingFinished,
            this,
            &SidebarModel::slotFeatureLoadingFinished);
    connect(feature,
            &LibraryFeature::featureSelect,
            this,
            &SidebarModel::slotFeatureSelect);

    QAbstractItemModel* model = feature->getChildModel();

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
    return createIndex(m_iDefaultSelectedIndex, 0, (void*)this);
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
            const QAbstractItemModel* childModel = m_sFeatures[parent.row()]->getChildModel();
            QModelIndex childIndex = childModel->index(row, column);
            TreeItem* tree_item = (TreeItem*)childIndex.internalPointer();
            if (tree_item && childIndex.isValid()) {
                return createIndex(childIndex.row(), childIndex.column(), (void*)tree_item);
            } else {
                return QModelIndex();
            }
        } else {
            // We have selected an item within the childmodel
            // This item has always an internal pointer of (sub)type TreeItem
            TreeItem* tree_item = (TreeItem*)parent.internalPointer();
            if (row < tree_item->childRows()) {
                return createIndex(row, column, (void*) tree_item->child(row));
            } else {
                // Otherwise this row might have been removed just now
                // (just a dirty workaround for unmaintainable GUI code)
                return QModelIndex();
            }
        }
    }
    return createIndex(row, column, (void*)this);
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
            TreeItem* tree_item = (TreeItem*)index.internalPointer();
            if (tree_item == nullptr) {
                return QModelIndex();
            }
            TreeItem* tree_item_parent = tree_item->parent();
            // if we have selected an item at the first level of a childnode

            if (tree_item_parent) {
                if (tree_item_parent->isRoot()) {
                    LibraryFeature* feature = tree_item->feature();
                    for (int i = 0; i < m_sFeatures.size(); ++i) {
                        if (feature == m_sFeatures[i]) {
                            // create a ModelIndex for parent 'this' having a
                            // library feature at position 'i'
                            return createIndex(i, 0, (void*)this);
                        }
                    }
                }
                // if we have selected an item at some deeper level of a childnode
                return createIndex(tree_item_parent->parentRow(), 0 , tree_item_parent);
            }
        }
    }
    return QModelIndex();
}

int SidebarModel::rowCount(const QModelIndex& parent) const {
    //qDebug() << "SidebarModel::rowCount parent=" << parent.getData();
    if (parent.isValid()) {
        if (parent.internalPointer() == this) {
            return m_sFeatures[parent.row()]->getChildModel()->rowCount();
        } else {
            // We support tree models deeper than 1 level
            TreeItem* tree_item = (TreeItem*)parent.internalPointer();
            if (tree_item) {
                return tree_item->childRows();
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
        }
        else
        {
            TreeItem* tree_item = (TreeItem*)parent.internalPointer();
            if (tree_item) {
                LibraryFeature* feature = tree_item->feature();
                return feature->getChildModel()->hasChildren(parent);
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
        if (role == Qt::DisplayRole) {
            return m_sFeatures[index.row()]->title();
        } else if (role == Qt::DecorationRole) {
            return m_sFeatures[index.row()]->getIcon();
        }
    }

    if (index.internalPointer() != this) {
        // If it points to a TreeItem
        TreeItem* tree_item = (TreeItem*)index.internalPointer();
        if (tree_item) {
            if (role == Qt::DisplayRole) {
                return tree_item->getLabel();
            } else if (role == Qt::ToolTipRole) {
                // If it's the "Quick Links" node, display it's name
                if (tree_item->getData().toString() == QUICK_LINK_NODE) {
                    return tree_item->getLabel();
                } else {
                    return tree_item->getData();
                }
            } else if (role == TreeItemModel::kDataRole) {
                // We use Qt::UserRole to ask for the datapath.
                return tree_item->getData();
            } else if (role == Qt::FontRole) {
                QFont font;
                font.setBold(tree_item->isBold());
                return font;
            } else if (role == Qt::DecorationRole) {
                return tree_item->getIcon();
            }
        }
    }

    return QVariant();
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
            TreeItem* tree_item = static_cast<TreeItem*>(index.internalPointer());
            if (tree_item) {
                LibraryFeature* feature = tree_item->feature();
                DEBUG_ASSERT(feature);
                feature->activateChild(index);
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
            TreeItem* tree_item = (TreeItem*)index.internalPointer();
            if (tree_item) {
                LibraryFeature* feature = tree_item->feature();
                feature->onLazyChildExpandation(index);
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
            TreeItem* tree_item = (TreeItem*)index.internalPointer();
            if (tree_item) {
                LibraryFeature* feature = tree_item->feature();
                feature->onRightClickChild(globalPos, index);
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
            TreeItem* tree_item = (TreeItem*)index.internalPointer();
            if (tree_item) {
                LibraryFeature* feature = tree_item->feature();
                result = feature->dropAcceptChild(index, urls, pSource);
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
            TreeItem* tree_item = (TreeItem*)index.internalPointer();
            if (tree_item) {
                LibraryFeature* feature = tree_item->feature();
                result = feature->dragMoveAcceptChild(index, url);
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
        const QModelIndex& index, const QAbstractItemModel* model) {
    QModelIndex translatedIndex;

    if (index.isValid()) {
       TreeItem* item = (TreeItem*)index.internalPointer();
       translatedIndex = createIndex(index.row(), index.column(), item);
    }
    else
    {
        //Comment from Tobias Rafreider --> Dead Code????

        for (int i = 0; i < m_sFeatures.size(); ++i) {
            if (m_sFeatures[i]->getChildModel() == model) {
                translatedIndex = createIndex(i, 0, (void*)this);
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
void SidebarModel::slotFeatureIsLoading(LibraryFeature * feature, bool selectFeature) {
    featureRenamed(feature);
    if (selectFeature) {
        slotFeatureSelect(feature);
    }
}

/* Tobias: This slot is somewhat redundant but I decided
 * to leave it for code readability reasons
 */
void SidebarModel::slotFeatureLoadingFinished(LibraryFeature * feature) {
    featureRenamed(feature);
    slotFeatureSelect(feature);
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
        TreeItem* item = (TreeItem*)featureIndex.internalPointer();
        ind = createIndex(featureIndex.row(), featureIndex.column(), item);
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
