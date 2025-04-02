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

} // anonymous namespace

SidebarModel::SidebarModel(
        QObject* parent)
        : QAbstractItemModel(parent),
          m_iDefaultSelectedIndex(0), // Tracks / MixxxLibraryFeature
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
        emit selectIndex(getDefaultSelection(), true /* scrollTo */);
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
        // If we have selected the root of a library feature at position 'row',
        // its internal pointer is a pointer to the current sidebarmodel object.
        // We return its associated childmodel index.
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
            // We have selected an item within the childmodel.
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
    if (!index.isValid()) {
        return {};
    }

    // If we have selected the root of a library feature,
    // its internal pointer is the current sidebar object model.
    // A root library feature has no parent and thus we return
    // an invalid QModelIndex.
    if (index.internalPointer() == this) {
        return {};
    }

    TreeItem* pTreeItem = static_cast<TreeItem*>(index.internalPointer());
    VERIFY_OR_DEBUG_ASSERT(pTreeItem != nullptr) {
        return {};
    }
    TreeItem* pParentItem = pTreeItem->parent();
    VERIFY_OR_DEBUG_ASSERT(pParentItem != nullptr) {
        return {};
    }
    if (pParentItem->isRoot()) {
        // If we have selected an item at the first level of a childnode,
        // Create a ModelIndex for parent 'this' having a
        // library feature at position 'i'.
        // `this` is const, but the function expects a
        // non-const pointer.
        // TODO: Check if we can get rid of this const cast
        // somehow.
        LibraryFeature* pFeature = pTreeItem->feature();
        int featureRow = m_sFeatures.indexOf(pFeature);
        VERIFY_OR_DEBUG_ASSERT(featureRow != -1) {
            return {};
        }
        return createIndex(featureRow, 0, const_cast<SidebarModel*>(this));
    }
    // If we have selected an item at some deeper level of a childnode
    return createIndex(pParentItem->parentRow(), 0, pParentItem);
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
        // If it points to SidebarModel this is a root item.
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
        case SidebarModel::NeedsUpdateRole:
            // True only for BrowseFeature items that need an update
            return pTreeItem->needsUpdate();
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

/// Invoked by double click on child items, click on tree node expand icons and
/// when jumping to a child item in a collapsed branch.
void SidebarModel::doubleClicked(const QModelIndex& index) {
    stopPressedUntilClickedTimer();
    if (!index.isValid()) {
        return;
    }
    if (index.internalPointer() == this) {
        // Index is a root index and QTreeView already did expand it,
        // so there's nothing to do for us.
        return;
    } else {
        TreeItem* pTreeItem = static_cast<TreeItem*>(index.internalPointer());
        VERIFY_OR_DEBUG_ASSERT(pTreeItem) {
            return;
        }
        LibraryFeature* pFeature = pTreeItem->feature();
        pFeature->onLazyChildExpandation(index);
    }
}

/// Invoked by click on Refresh icon of BrowseFeature items whose child tree
/// is outdated. Triggers force-rebuild of the chidl tree.
void SidebarModel::updateItem(const QModelIndex& index) {
    stopPressedUntilClickedTimer();
    if (!index.isValid()) {
        return;
    }
    TreeItem* pTreeItem = static_cast<TreeItem*>(index.internalPointer());
    if (pTreeItem) {
        LibraryFeature* pFeature = pTreeItem->feature();
        pFeature->onLazyChildExpandation(index, true /* enforce rebuild */);
    }
}

void SidebarModel::rightClicked(const QPoint& globalPos, const QModelIndex& index) {
    stopPressedUntilClickedTimer();
    if (!index.isValid()) {
        return;
    }
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
    QModelIndex newParent = translateSourceIndex(parent);
    endInsertRows();
    maybeUpdateBookmarkIndices(newParent);
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

void SidebarModel::slotFeatureSelect(LibraryFeature* pFeature,
        const QModelIndex& featureIndex,
        bool scrollTo) {
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
    emit selectIndex(ind, scrollTo);
}

void SidebarModel::toggleBookmarkByIndex(const QModelIndex& index) {
    qWarning() << "   toggleBookmarkByIndex" << index;
    if (!index.isValid()) {
        return;
    }

    SidebarBookmark bookmark = createBookmarkFromIndex(index);
    if (m_bookmarks.contains(bookmark)) {
        // Remove bookmark and index
        m_bookmarks.removeOne(bookmark);
        qWarning() << "--- removed" << bookmark;
    } else {
        m_bookmarks.append(bookmark);
        qWarning() << "+++ added" << bookmark;
    }
    m_bookmarkIndices = sortBookmarksUpdateIndices(&m_bookmarks);
}

QModelIndexList SidebarModel::sortBookmarksUpdateIndices(QList<SidebarBookmark>* pBookmarks) {
    // Sort by position in the tree so getNextPrevBookmarkIndex()
    // switches to bookmark below/above in a predictable manner:
    // feature row -> child level -> parent row
    std::sort(pBookmarks->begin(), pBookmarks->end());
    // Update indices. Only add valid indices -- invalid means the bookmark
    // wasn't found after last child model update.
    QModelIndexList newBookmarkIndices;
    for (const auto& bm : std::as_const(*pBookmarks)) {
        if (bm.index.isValid()) {
            newBookmarkIndices.append(bm.index);
        }
    }
    return newBookmarkIndices;
    qDebug() << "Sidebar bookmarks updated";
}

QModelIndex SidebarModel::getNextPrevBookmarkIndex(const QModelIndex& currIndex, int direction) {
    if (!currIndex.isValid() || direction == 0) {
        qWarning() << "SidebarModel::getNextPrevBookmarkIndex: invalid index "
                      "or dir == 0"
                   << currIndex;
        return {};
    }
    if (m_bookmarks.isEmpty()) {
        qWarning() << "SidebarModel::getNextPrevBookmarkIndex: no bookmarks stored";
        return {};
    }

    if (m_bookmarkIndices.size() == 1 && currIndex == m_bookmarkIndices[0]) {
        // We already have the only bookmark selected.
        // Return invalid index so the sidebar does not reload the current view.
        return {};
    }

    // Do single steps regardless the input.
    direction = direction > 0 ? 1 : -1;

    int currPos = 0;
    const SidebarBookmark tempBM = createBookmarkFromIndex(currIndex);
    if (m_bookmarks.contains(tempBM)) {
        currPos = m_bookmarks.indexOf(tempBM);
    } else {
        // We're not on a bookmark. To get true prev/next (up/down) behavior,
        // we add tempBM to a clone of m_bookmarks, sort, create index list and
        // get the position of currIndex.
        auto tempBookmarks = m_bookmarks;
        tempBookmarks.append(tempBM);
        sortBookmarksUpdateIndices(&tempBookmarks);
        currPos = tempBookmarks.indexOf(tempBM);
        if (direction > 0 && currPos <= m_bookmarks.size() - 1) {
            // Not first, subtract 1 so we're in the real m_bookmarks range again.
            currPos--;
            // qWarning() << " >> tempPos at end, --:" << bookmarkPos;
        }
    }
    // Now we have a valid start position in the bookmark list.
    int targetPos = currPos + direction;
    if (targetPos < 0) {
        // wrap-around, pick last
        targetPos = m_bookmarks.size() - 1;
    } else if (targetPos >= m_bookmarks.size()) {
        // pick first
        targetPos = 0;
    }

    SidebarBookmark targetBM;
    QModelIndex targetIndex;
    int maxAttempts = m_bookmarks.size();
    int attempt = 0;
    while (!targetIndex.isValid() && attempt < maxAttempts) {
        targetBM = m_bookmarks[targetPos];
        // If this is root item we can simply create an index.
        // Else we look insinde the child model
        if (targetBM.childLevel == 0) {
            targetIndex = index(targetBM.featureRow, 0);
        } else {
            targetIndex = translateChildIndex(findBookmarkIndex(targetBM));
        }
        attempt++;
        targetPos++;
        if (targetPos >= maxAttempts) {
            targetPos = 0; // wrap around
        }
    }

    if (targetIndex != targetBM.index) {
        // Lookup bookmark.
        // If found, replace with fresh bookmark, ie. update all properties at once.
        // (just in case the index changed and we didn't notice)
        // Else just invalidate it's index so we know that we should skip it when
        // updating the index list.
        if (targetIndex.isValid()) {
            targetBM = createBookmarkFromIndex(translateSourceIndex(targetIndex));
        } else {
            targetBM.index = QModelIndex();
        }
        sortBookmarksUpdateIndices(&m_bookmarks);
    }
    return targetIndex;
}

/// Invoked by rowsInserted(). A range of sidebar indices of a childmodel has been
/// rebuilt. Some stored indices may now be invalid so we need to reassociate
/// bookmarks with new indices and update the quick-lookup index list for
/// Happens when playlists or crates are added/removedm, when a History playlist
/// has been moved into a YEAR group or when the BrowseFeature tree is rebuilt.
// TODO(ronso0) Implement some lock/wait mechanism to avoid concurrent access to
// m_bookmarks/m_bookmarkIndices. Caller is in same thread so QMutex won't work.
// Reason: previously, deleting a playlist caused both PlaylistFeature and SetlogFeauture
// to rebuilt their child models, even though only one of them can be affected.
// This is now fixed, but I didn't check with other features, so can't rule out
// simultaneous invocations of rowsInserted().
void SidebarModel::maybeUpdateBookmarkIndices(const QModelIndex& parentIndex) {
    // qWarning() << "maybeUpdateBookmarkIndices" << parentIndex;
    if (m_bookmarkIndices.isEmpty()) {
        return;
    }
    // Collect the start parameters for findBookmarkIndex()
    int featureRow = -1;
    if (parentIndex.internalPointer() == this) {
        featureRow = parentIndex.row();
    } else {
        const auto* pTreeItem = static_cast<TreeItem*>(parentIndex.internalPointer());
        VERIFY_OR_DEBUG_ASSERT(pTreeItem) {
            return;
        }
        featureRow = m_sFeatures.indexOf(pTreeItem->feature());
    }

    bool needsUpdate = false;
    for (auto& bm : m_bookmarks) {
        if (bm.featureRow != featureRow) {
            continue;
        }
        needsUpdate = true;
        // Lookup bookmark.
        // If found, replace with fresh bookmark, ie. update all properties at once.
        // Else just invalidate it's index so we know that we should skip it when
        // updating the index list.
        const auto bmIndex = findBookmarkIndex(bm);
        if (bmIndex.isValid()) {
            bm = createBookmarkFromIndex(translateSourceIndex(bmIndex));
            // qWarning() << "   >> updated" << bm << "in" << pFeature->title().toString();
        } else {
            // qWarning() << "   >> no match for" << bm << "in" << pFeature->title().toString();
            bm.index = QModelIndex();
        }
    }

    if (!needsUpdate) {
        // no hit for affected feature, nothing to do
        return;
    }

    // Note: Don't remove missing bookmarks.
    // BrowseFeature bookmarks may be 'missing' after collapsing and
    // re-expanding a directory tree one or more levels above a bookmark
    // because expanding an item only rebuilds the next sublevel, so
    // bookmarked items on lower levels are simply not there, yet.

    m_bookmarkIndices = sortBookmarksUpdateIndices(&m_bookmarks);
}

/// Try to find the matching TreeItem in a feature's childmodel.
/// Return its index when found, else return invalid QModelIndex().
/// Scans the entire tree recursively, either by item data or label.
// TODO Do we need to store an index list at all when using match()?
// Compare performance of list vs. match() for each next/prev move.
QModelIndex SidebarModel::findBookmarkIndex(const SidebarBookmark& bookmark) {
    // qWarning() << " findBookmarkIndex" << bookmark;
    LibraryFeature* pFeature = m_sFeatures[bookmark.featureRow];
    TreeItemModel* pChildModel = pFeature->sidebarModel();
    const QModelIndex rootIndex = index(bookmark.featureRow, 0);
    DEBUG_ASSERT(pChildModel);
    QModelIndexList results;
    if (bookmark.data.isValid() && pFeature->isItemDataUnique(bookmark.data)) {
        // Search for matching data
        results = pChildModel->match(
                rootIndex,
                TreeItemModel::kDataRole,
                bookmark.data,
                1,
                Qt::MatchWrap | Qt::MatchExactly | Qt::MatchRecursive);
    } else {
        // Search for label match.
        // This covers root items, Tracks Missing/Hidden, AutoDJ Crates
        // and History's YEAR nodes.
        results = pChildModel->match(
                rootIndex,
                Qt::DisplayRole,
                bookmark.label,
                1,
                Qt::MatchWrap | Qt::MatchExactly | Qt::MatchRecursive);
    }

    if (!results.isEmpty()) {
        return results.front();
    }
    return {};
}

SidebarBookmark SidebarModel::createBookmarkFromIndex(const QModelIndex& index) {
    if (!index.isValid()) {
        return {};
    }

    // qWarning() << " >> getBookmarkFromIndex" << index;
    SidebarBookmark bookmark;
    if (index.internalPointer() == this) {
        LibraryFeature* pFeature = m_sFeatures[index.row()];
        bookmark = SidebarBookmark(
                index.row(),
                0,
                0,
                QVariant(),
                pFeature->title().toString(),
                index);
    } else {
        TreeItem* pTreeItem = static_cast<TreeItem*>(index.internalPointer());
        VERIFY_OR_DEBUG_ASSERT(pTreeItem) {
            return {};
        }
        bookmark.featureRow = m_sFeatures.indexOf(pTreeItem->feature());
        bookmark.childLevel = pTreeItem->childLevel();
        bookmark.parentRow = pTreeItem->parentRow();
        const auto& data = pTreeItem->getData();
        if (data.isValid() && pTreeItem->isDataUniqueInFeature()) {
            bookmark.data = data;
        } else {
            bookmark.label = pTreeItem->getLabel();
        }
        bookmark.index = index;
    }
    // qWarning() << " >> created" << bookmark;
    DEBUG_ASSERT(bookmark.isValid());
    return bookmark;
}

bool SidebarModel::indexIsBookmark(const QModelIndex& index) const {
    if (!index.isValid()) {
        return false;
    }
    return m_bookmarkIndices.contains(index);
}
