#include "library/sidebarmodel.h"

#include <QTimer>
#include <QUrl>

#include "library/libraryfeature.h"
#include "library/treeitem.h"
#include "moc_sidebarmodel.cpp"
#include "util/assert.h"
#include "util/cmdlineargs.h"

namespace {

/// The time between selecting and activating (= clicking) a feature item
/// in the sidebar tree. This is essential to allow smooth scrolling through
/// a list of items with an encoder or the keyboard! A value of 300 ms has
/// been chosen as a compromise between usability and responsiveness.
constexpr int kPressedUntilClickedTimeoutMillis = 300;

/// Enables additional debugging output.
constexpr bool kDebug = false;

const QBrush kBookmarkBgBrush(QColor::fromHsl(161, 255, 70)); // teal
const QBrush kBookmarkFgBrush(QColor::fromHsl(0, 0, 255));    // white

} // anonymous namespace

SidebarModel::SidebarModel(
        QObject* parent)
        : QAbstractItemModel(parent),
          m_iDefaultSelectedIndex(0), // Tracks
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
    if constexpr (kDebug) {
        qDebug() << "SidebarModel::index row=" << row
                 << "column=" << column << "parent=" << parent;
    }

    if (parent.isValid()) {
        // If we have selected the root of a library feature at position 'row'
        // its internal pointer is the current sidebar object model
        // we return its associated childmodel.
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

QModelIndex SidebarModel::getFeatureRootIndex(LibraryFeature* pFeature) {
    if constexpr (kDebug) {
        qDebug() << "SidebarModel::getFeatureRootIndex for" << pFeature->title().toString();
    }
    int featureRow = m_sFeatures.indexOf(pFeature);
    VERIFY_OR_DEBUG_ASSERT(featureRow != -1) {
        return {};
    }
    return index(featureRow, 0);
}

void SidebarModel::clear(const QModelIndex& index) {
    if (index.internalPointer() == this) {
        m_sFeatures[index.row()]->clear();
    }
}

void SidebarModel::paste(const QModelIndex& index) {
    if (index.internalPointer() == this) {
        m_sFeatures[index.row()]->paste();
    } else {
        TreeItem* pTreeItem = (TreeItem*)index.internalPointer();
        if (pTreeItem) {
            LibraryFeature* feature = pTreeItem->feature();
            feature->pasteChild(index);
        }
    }
}

QModelIndex SidebarModel::parent(const QModelIndex& index) const {
    if constexpr (kDebug) {
        qDebug() << "SidebarModel::parent index=" << index;
    }
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
                    int featureRow = m_sFeatures.indexOf(pFeature);
                    VERIFY_OR_DEBUG_ASSERT(featureRow != -1) {
                        return {};
                    }
                    // create a ModelIndex for parent 'this' having a
                    // library feature at position 'i'
                    // `this` is const, but the function expects a
                    // non-const pointer.
                    // TODO: Check if we can get rid of this const cast
                    // somehow.
                    return createIndex(featureRow, 0, const_cast<SidebarModel*>(this));
                }
                // if we have selected an item at some deeper level of a childnode
                return createIndex(pTreeItemParent->parentRow(), 0, pTreeItemParent);
            }
        }
    }
    return QModelIndex();
}

int SidebarModel::rowCount(const QModelIndex& parent) const {
    if constexpr (kDebug) {
        qDebug() << "SidebarModel::rowCount parent=" << parent;
    }
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
    if constexpr (kDebug) {
        qDebug() << "SidebarModel::columnCount parent=" << parent;
    }
    // TODO(rryan) will we ever have columns? I don't think so.
    return 1;
}

/// Used to decide whether to display the branch expand icon.
/// Note that for the Browse feature's DEVICE node this always returns true,
/// regardless the actual chidlren().size().
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
    if constexpr (kDebug) {
        qDebug("SidebarModel::data() row=%d column=%d pointer=%p, role=%d",
                index.row(),
                index.column(),
                index.internalPointer(),
                role);
    }

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
        case Qt::BackgroundRole:
            // Paint bookmarks.
            // Get colors from skin template via WLibrarySideBar
            if (featureRootIsBookmark(index.row())) {
                return kBookmarkBgBrush; // teal
            }
            return QVariant();
        case Qt::ForegroundRole:
            if (featureRootIsBookmark(index.row())) {
                return kBookmarkFgBrush;
            }
            return QVariant();
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
            if (CmdlineArgs::Instance().getDeveloper()) {
                // Display the internal data for debugging
                return pTreeItem->getData();
            }
            // Show the label. Helpful for long names with a narrow sidebar.
            return pTreeItem->getLabel();
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
        case Qt::BackgroundRole:
            // Paint bookmarks.
            // Get colors from skin template via WLibrarySideBar
            if (treeItemIsBookmark(pTreeItem)) {
                return kBookmarkBgBrush;
            }
            return QVariant();
        case Qt::ForegroundRole:
            if (treeItemIsBookmark(pTreeItem)) {
                return kBookmarkFgBrush;
            }
            return QVariant();
        case SidebarModel::IconNameRole:
            // TODO: Add support for icon names in tree items
        default:
            return QVariant();
        }
    }
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

/// Connected to WLibrarySidebar::pressed signal, called after left click and
/// selection change via keyboard or controller
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

/// Invoked by double click and click on tree node expand icons
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
        } else {
            TreeItem* pTreeItem = static_cast<TreeItem*>(index.internalPointer());
            if (pTreeItem) {
                LibraryFeature* pFeature = pTreeItem->feature();
                pFeature->onRightClickChild(globalPos, index);
            }
        }
    }
}

void SidebarModel::renameItem(const QModelIndex& index) {
    stopPressedUntilClickedTimer();
    m_pressedIndex = index;
    if (!index.isValid()) {
        return;
    }

    if (index.internalPointer() == this) {
        // can't rename root features
        return;
    } else {
        TreeItem* pTreeItem = static_cast<TreeItem*>(index.internalPointer());
        if (pTreeItem) {
            LibraryFeature* pFeature = pTreeItem->feature();
            pFeature->renameItem(index);
        }
    }
}

void SidebarModel::deleteItem(const QModelIndex& index) {
    stopPressedUntilClickedTimer();
    m_pressedIndex = index;
    if (!index.isValid()) {
        return;
    }

    if (index.internalPointer() == this) {
        // Used only to call AutoDJFeature::clear()
        m_sFeatures[index.row()]->clear();
        return;
    } else {
        TreeItem* pTreeItem = static_cast<TreeItem*>(index.internalPointer());
        if (pTreeItem) {
            LibraryFeature* pFeature = pTreeItem->feature();
            pFeature->deleteItem(index);
        }
    }
}

bool SidebarModel::dropAccept(const QModelIndex& index, const QList<QUrl>& urls, QObject* pSource) {
    if constexpr (kDebug) {
        qDebug() << "SidebarModel::dropAccept() index=" << index << urls;
    }
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

bool SidebarModel::dragMoveAccept(const QModelIndex& index, const QUrl& url) const {
    if constexpr (kDebug) {
        qDebug() << "SidebarModel::dragMoveAccept() index=" << index << url;
    }
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

/// Translates an index from the child models to an index of the sidebar models
QModelIndex SidebarModel::translateSourceIndex(const QModelIndex& index) {
    // These method is called from the slot functions below.
    // QObject::sender() return the object which emitted the signal
    // handled by the slot functions.
    // For child models, this always the child models itself
    const QAbstractItemModel* pModel = qobject_cast<QAbstractItemModel*>(sender());
    VERIFY_OR_DEBUG_ASSERT(pModel != nullptr) {
        return QModelIndex();
    }

    return translateIndex(index, pModel);
}

QModelIndex SidebarModel::translateIndex(
        const QModelIndex& index, const QAbstractItemModel* pModel) {
    QModelIndex translatedIndex;

    if (index.isValid()) {
        TreeItem* pItem = static_cast<TreeItem*>(index.internalPointer());
        translatedIndex = createIndex(index.row(), index.column(), pItem);
    } else {
        for (int i = 0; i < m_sFeatures.size(); ++i) {
            if (m_sFeatures[i]->sidebarModel() == pModel) {
                translatedIndex = createIndex(i, 0, this);
            }
        }
    }
    return translatedIndex;
}

void SidebarModel::slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight) {
    // qDebug() << "slotDataChanged topLeft:" << topLeft;
    // qDebug() << "            bottomRight:" << bottomRight;
    QModelIndex topLeftTranslated = translateSourceIndex(topLeft);
    QModelIndex bottomRightTranslated = translateSourceIndex(bottomRight);
    emit dataChanged(topLeftTranslated, bottomRightTranslated);
}

void SidebarModel::slotRowsAboutToBeInserted(const QModelIndex& parent, int start, int end) {
    // qDebug() << "slotRowsABoutToBeInserted" << parent << start << end;
    QModelIndex newParent = translateSourceIndex(parent);
    beginInsertRows(newParent, start, end);
}

void SidebarModel::slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end) {
    // qDebug() << "slotRowsABoutToBeRemoved" << parent << start << end;
    QModelIndex newParent = translateSourceIndex(parent);
    beginRemoveRows(newParent, start, end);
}

void SidebarModel::slotRowsInserted(const QModelIndex& parent, int start, int end) {
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    // qDebug() << "slotRowsInserted" << parent << start << end;
    // QModelIndex newParent = translateSourceIndex(parent);
    endInsertRows();
}

void SidebarModel::slotRowsRemoved(const QModelIndex& parent, int start, int end) {
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    // qDebug() << "slotRowsRemoved" << parent << start << end;
    // QModelIndex newParent = translateSourceIndex(parent);
    endRemoveRows();
}

void SidebarModel::slotModelAboutToBeReset() {
    // qDebug() << "slotModelAboutToBeReset";
    beginResetModel();
}

void SidebarModel::slotModelReset() {
    // qDebug() << "slotModelReset";
    endResetModel();
}

// Call this slot whenever the title of the feature has changed.
// See RhythmboxFeature for an example, in which the title becomes '(loading) Rhythmbox'
// If selectFeature is true, the feature is selected when the title change occurs.
void SidebarModel::slotFeatureIsLoading(LibraryFeature* pFeature, bool selectFeature) {
    featureRenamed(pFeature);
    if (selectFeature) {
        slotFeatureSelect(pFeature);
    }
}

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

void SidebarModel::bookmarkSelectedItem(const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }

    Bookmark bookmark = createBookmarkFromIndex(index);
    if (!bookmark.isValid()) {
        return;
    }

    if (m_bookmarks.contains(bookmark)) {
        m_bookmarks.removeOne(bookmark);
        qWarning() << "---------- removed sidebar bookmark" << bookmark.data;
        return;
    }

    m_bookmarks.append(bookmark);
    qWarning() << "---------- added sidebar bookmark" << bookmark.data;

    // Sort by position in the tree so selectNextPrevBookmark()
    // switches to bookmark bewlo/above:
    // feature row -> child level -> child row
    std::sort(m_bookmarks.begin(), m_bookmarks.end());
}

QModelIndex SidebarModel::selectNextPrevBookmark(const QModelIndex& selIndex, int direction) {
    if (!selIndex.isValid() || direction == 0) {
        qWarning() << " ! SM selectNextPrevBookmark: invalid index or dir == 0" << selIndex;
        return {};
    }
    if (m_bookmarks.isEmpty()) {
        qWarning() << " ! SM selectNextPrevBookmark: no bookmarks stored";
        return {};
    }

    const Bookmark currBM = createBookmarkFromIndex(selIndex);
    int bookmarkPos = 0;
    if (m_bookmarks.size() == 0) {
        if (currBM == m_bookmarks[0]) {
            // We already have the only bookmark selected
            return selIndex;
        }
    } else {
        bookmarkPos = m_bookmarks.indexOf(currBM);
        if (bookmarkPos == -1) {
            // not found, select first
            bookmarkPos = 0;
        } else {
            bookmarkPos += direction;
            if (bookmarkPos < 0) {
                // wrap-around, pick last
                bookmarkPos = m_bookmarks.size() - 1;
            } else if (bookmarkPos >= m_bookmarks.size()) {
                // pick first
                bookmarkPos = 0;
            }
        }
    }
    return selectBookmarkByPos(bookmarkPos);
}

// Not necessary atm, but will help for bookmark select buttons.
// Those would work like hotcue buttons:
// click to assign or activate, right-click to unset
QModelIndex SidebarModel::selectBookmarkByPos(int pos) {
    if (m_bookmarks.isEmpty()) {
        return {};
    }
    if (pos < 0 || pos > m_bookmarks.size() - 1) {
        return {};
    }
    // qWarning() << " -- new pos:" << pos;
    // the item we want to find and select
    Bookmark nextBookmark = m_bookmarks[pos];
    // qWarning() << " -> bookmark" << pos << nextBookmark.data;
    if (nextBookmark.data.isNull()) {
        // top-level item, feature root item
        // qWarning() << " -> bookmark.data == null, bookmark is root item";
        return index(nextBookmark.featureRow, 0);
    }

    // Item is a child (branch or leave)
    TreeItemModel* pChildModel = m_sFeatures[nextBookmark.featureRow]->sidebarModel();
    TreeItem* pBaseItem = pChildModel->getRootItem();
    QModelIndex baseIdx = index(nextBookmark.featureRow, 0);
    // qWarning() << " -> checking children...";
    // qWarning() << " -> base index:" << baseIdx;
    QModelIndex targetIdx = getBookmarkIndex(pBaseItem, baseIdx, nextBookmark);
    if (targetIdx.isValid()) {
        // qWarning() << " -> found matching child, index:" << targetIdx;
        return targetIdx;
    }
    // no matching child found
    // TODO remove invalid bookmark?
    // Or can this happen after rebuilding a Browse feature's device tree
    // which is not yet expanded down to the previously bookmarked item?
    return {};
}

/// Try to find the TreeItem in a feature's childmodel that matches the bookmark.
/// Return a valid index when found, else return invalid QModelIndex().
/// Scans the entire tree from the top.
QModelIndex SidebarModel::getBookmarkIndex(
        const TreeItem* pItem,
        const QModelIndex& baseIndex,
        const SidebarModel::Bookmark& bookmark) {
    // qWarning() << " gBIdx   index:" << baseIndex;

    // qWarning() << "   data: " << pItem->getData();
    // qWarning() << "   curr ch level: " << pItem->childLevel();
    // qWarning() << "   bookm ch level:" << bookmark.levelAndRow.x();
    // No need to recurse into children if tree level is already higher than the
    // that of the bookmark. This is relevant only for Browsefeature where
    // and SetlogFeature.
    if (pItem->childLevel() > bookmark.levelAndRow.x()) {
        return {};
    }
    QModelIndex parentIndex = baseIndex;
    int parentRow = pItem->parentRow();
    // TreeItem::kInvalidRow indicates this is a feature root item.
    // No need to adjust the parent index in that case.
    // Else we have a child and
    if (parentRow != TreeItem::kInvalidRow) {
        parentIndex = index(parentRow, 0, baseIndex);
        // qWarning() << "   child at row" << parentRow;
        // qWarning() << "     parentIdx:" << parentIndex;
    }

    // The first item passed to this is the feature root item.
    // Later, item is a child for which we already created the associated index above.
    // So just return the index.
    if (pItem->getData() == bookmark.data) {
        // qWarning() << "   MATCH------" << baseIndex;
        return parentIndex;
    }
    // Item didn't match, let's check its children
    const QList<TreeItem*> children = pItem->children();
    if (children.isEmpty()) {
        // qWarning() << "  ! no match, no children";
        return {};
    }

    for (const auto* pChItem : std::as_const(children)) {
        const QModelIndex match = getBookmarkIndex(pChItem, parentIndex, bookmark);
        if (match.isValid()) {
            // qWarning() << "   xxx MATCH  :" << match << pItem->parentRow();
            return match;
        }
    }
    return {};
}

SidebarModel::Bookmark SidebarModel::createBookmarkFromIndex(const QModelIndex& index) {
    if (!index.isValid()) {
        return {};
    }

    // qWarning() << " >> getBookmarkFromIndex" << index;
    if (index.internalPointer() == this) {
        // LibraryFeature* pFeature = m_sFeatures[index.row()];
        // qWarning() << " >> found root" << index.row() << pFeature->title().toString();
        return Bookmark(index.row(), QVariant(), QPoint(0, 0));
    } else {
        TreeItem* pTreeItem = static_cast<TreeItem*>(index.internalPointer());
        if (!pTreeItem) {
            // qWarning() << " >> ! found child, no TreeItem";
            return {};
        }
        int featureRow = m_sFeatures.indexOf(pTreeItem->feature());
        VERIFY_OR_DEBUG_ASSERT(featureRow != -1) {
            return {};
        }

        QVariant data = pTreeItem->getData();
        // Store child level + child row for sorting.
        int level = pTreeItem->childLevel();
        int childRow = pTreeItem->parentRow();
        // int level = 1;
        // TreeItem* pTempItem = pTreeItem->parent();
        // while (!pTempItem->isRoot()) {
        //     level++;
        //     pTempItem = pTempItem->parent();
        // }

        // qWarning() << " >> found" << pTreeItem->feature()->title().toString() << "child";
        // qWarning() << "    level:" << level << "child#" << childRow;
        return Bookmark(featureRow, data, QPoint(level, childRow));
    }
}

bool SidebarModel::treeItemIsBookmark(const TreeItem* pTreeItem) const {
    // qWarning() << " ?? treeItemIsBookmark" << pTreeItem->getData();
    for (auto& bm : std::as_const(m_bookmarks)) {
        if (pTreeItem->getData() == bm.data) {
            qWarning() << " treeItemIsBookmark row:" << bm.featureRow
                       << "data:" << pTreeItem->getData();
            return true;
        }
    }
    return false;
}

bool SidebarModel::featureRootIsBookmark(int featureRow) const {
    // qWarning() << " ?? featureRootIsBookmark" << featureRow;
    for (auto& bm : std::as_const(m_bookmarks)) {
        if (bm.data.isNull() &&
                // ensure it's a root item, i.e. exclude Missing/Hidden
                // which also have no data
                bm.levelAndRow.x() == 0 &&
                bm.featureRow == featureRow) {
            return true;
        }
    }
    return false;
}
