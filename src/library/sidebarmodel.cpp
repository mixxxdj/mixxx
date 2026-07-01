#include "library/sidebarmodel.h"

#include <QTimer>
#include <QUrl>

#include "library/libraryfeature.h"
#include "library/treeitem.h"
#include "moc_sidebarmodel.cpp"
#include "util/assert.h"
#include "util/cmdlineargs.h"
#include <QMimeData>
#include "library/dao/playlistdao.h"
#include "library/library.h"
#include "library/trackset/playlistfeature.h"

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
          m_iDefaultSelectedIndex(0),
          m_pressedUntilClickedTimer(new QTimer(this)) {
    m_pressedUntilClickedTimer->setSingleShot(true);
    connect(m_pressedUntilClickedTimer,
            &QTimer::timeout,
            this,
            &SidebarModel::slotPressedUntilClickedTimeout);

    // Queue the move operation so drag-and-drop stays thread-safe.
    connect(this, &SidebarModel::requestPlaylistMove,
            this, &SidebarModel::slotExecutePlaylistMove,
            Qt::QueuedConnection);
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

QModelIndex SidebarModel::getFeatureRootIndex(LibraryFeature* pFeature) {
    if constexpr (kDebug) {
        qDebug() << "SidebarModel::getFeatureRootIndex for" << pFeature->title().toString();
    }
    QModelIndex ind;
    for (int i = 0; i < m_sFeatures.size(); ++i) {
        if (m_sFeatures[i] == pFeature) {
            ind = index(i, 0);
            break;
        }
    }
    return ind;
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
    if (!index.isValid()) {
        return QModelIndex();
    }

    if (index.internalPointer() == this) {
        return QModelIndex();
    }

    TreeItem* pChildItem = static_cast<TreeItem*>(index.internalPointer());
    if (!pChildItem) {
        return QModelIndex();
    }

    TreeItem* pParentItem = pChildItem->parent();

    if (!pParentItem || pParentItem->isRoot()) {
        LibraryFeature* pFeature = pChildItem->feature();
        if (!pFeature) {
            return QModelIndex();
        }
        
        int featureRow = m_sFeatures.indexOf(pFeature);
        if (featureRow != -1) {
            return createIndex(featureRow, 0, const_cast<SidebarModel*>(this));
        }
        return QModelIndex();
    }

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
    if (!parent.isValid()) {
        return true;
    }

    if (parent.internalPointer() == this) {
        return rowCount(parent) > 0;
    }

    TreeItem* pItem = static_cast<TreeItem*>(parent.internalPointer());
    return pItem ? (pItem->childRows() > 0) : false;
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
    if (!index.isValid()) {
        return false;
    }

    if (index.internalPointer() == this) {
        return m_sFeatures[index.row()]->dropAccept(urls, pSource);
    } else {
        TreeItem* pTreeItem = static_cast<TreeItem*>(index.internalPointer());
        if (!pTreeItem) {
            return false;
        }
        LibraryFeature* pFeature = pTreeItem->feature();
        return pFeature->dropAcceptChild(index, urls, pSource);
    }
}

bool SidebarModel::hasTrackTable(const QModelIndex& index) const {
    if (index.internalPointer() == this) {
        return m_sFeatures[index.row()]->hasTrackTable();
    }
    return false;
}

bool SidebarModel::dragMoveAccept(const QModelIndex& index, const QList<QUrl>& urls) const {
    if constexpr (kDebug) {
        qDebug() << "SidebarModel::dragMoveAccept() index=" << index << urls;
    }
    
    // If there is no URL, we are dragging an internal item within Playlists,
    // so we must allow the drop.
    if (urls.isEmpty() && index.isValid() && index.internalPointer() != this) {
        TreeItem* pItem = static_cast<TreeItem*>(index.internalPointer());
        if (pItem && pItem->feature() && pItem->feature()->title().toString() == QStringLiteral("Playlists")) {
            return true;
        }
    }

    // Otherwise keep the default Mixxx logic for external files and similar drops.
    if (!index.isValid()) {
        return false;
    }
    if (index.internalPointer() == this) {
        return m_sFeatures[index.row()]->dragMoveAccept(urls);
    } else {
        TreeItem* pTreeItem = static_cast<TreeItem*>(index.internalPointer());
        if (!pTreeItem) {
            return false;
        }
        LibraryFeature* pFeature = pTreeItem->feature();
        VERIFY_OR_DEBUG_ASSERT(pFeature) {
            return false;
        }
        return pFeature->dragMoveAcceptChild(index, urls);
    }
}

/// Translates an index from the child models to an index of the sidebar models
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
        for (int i = 0; i < m_sFeatures.size(); ++i) {
            if (m_sFeatures[i]->sidebarModel() == pModel) {
                translatedIndex = createIndex(i, 0, this);
            }
        }
    }
    return translatedIndex;
}

void SidebarModel::slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight) {
    // qDebug() << "slotDataChanged topLeft:" << topLeft << "bottomRight:" << bottomRight;
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


Qt::ItemFlags SidebarModel::flags(const QModelIndex& index) const {
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    if (!index.isValid() || index.internalPointer() == this) {
        defaultFlags |= Qt::ItemIsDropEnabled;
        return defaultFlags;
    }

    TreeItem* pItem = static_cast<TreeItem*>(index.internalPointer());
    if (pItem) {
        LibraryFeature* pFeature = pItem->feature();
        // Allow drag and drop for items that belong to a feature.
        if (pFeature) {
            defaultFlags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        }
    }

    return defaultFlags;
}

Qt::DropActions SidebarModel::supportedDropActions() const {
    // Handle internal reparenting within the tree.
    return Qt::MoveAction;
}

QStringList SidebarModel::mimeTypes() const {
    QStringList types;
    types << QStringLiteral("application/x-mixxx-playlist-id");
    return types;
}

QMimeData* SidebarModel::mimeData(const QModelIndexList& indexes) const {
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const QModelIndex& index : indexes) {
        if (index.isValid() && index.internalPointer() != this) {
            TreeItem* pItem = static_cast<TreeItem*>(index.internalPointer());
            if (pItem) {
                int playlistId = pItem->getData().toInt();
                stream << playlistId;
            }
        }
    }

    mimeData->setData(QStringLiteral("application/x-mixxx-playlist-id"), encodedData);
    return mimeData;
}

bool SidebarModel::dropMimeData(const QMimeData* data, Qt::DropAction action,
                                int row, int column, const QModelIndex& parent) {
    Q_UNUSED(row);
    Q_UNUSED(column);

    if (action == Qt::IgnoreAction) {
        return true;
    }

    if (!data->hasFormat(QStringLiteral("application/x-mixxx-playlist-id"))) {
        return false;
    }

    QByteArray encodedData = data->data(QStringLiteral("application/x-mixxx-playlist-id"));
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    int movedPlaylistId = kInvalidPlaylistId;
    stream >> movedPlaylistId;

    if (movedPlaylistId == kInvalidPlaylistId) {
        return false;
    }

    int targetParentId = kInvalidPlaylistId;
    TreeItem* pTargetItem = nullptr;
    if (parent.isValid() && parent.internalPointer() != this) {
        pTargetItem = static_cast<TreeItem*>(parent.internalPointer());
    }

    if (pTargetItem) {
        const int targetItemId = pTargetItem->getData().toInt();
        if (targetItemId != kInvalidPlaylistId) {
            PlaylistFeature* pPlaylistFeature = nullptr;
            for (LibraryFeature* pFeature : m_sFeatures) {
                if (pFeature && pFeature->title().toString() == QStringLiteral("Playlists")) {
                    pPlaylistFeature = qobject_cast<PlaylistFeature*>(pFeature);
                    break;
                }
            }

            if (pPlaylistFeature && pPlaylistFeature->playlistDao().isFolder(targetItemId)) {
                targetParentId = targetItemId;
            } else {
                TreeItem* pAncestor = pTargetItem->parent();
                while (pAncestor) {
                    if (pAncestor->getData().toInt() == movedPlaylistId) {
                        return false;
                    }
                    pAncestor = pAncestor->parent();
                }

                if (pTargetItem->parent() && !pTargetItem->parent()->isRoot()) {
                    const int parentItemId = pTargetItem->parent()->getData().toInt();
                    if (parentItemId != kInvalidPlaylistId) {
                        targetParentId = parentItemId;
                    }
                }
            }
        }

        TreeItem* pAncestor = pTargetItem;
        while (pAncestor) {
            if (pAncestor->getData().toInt() == movedPlaylistId) {
                return false;
            }
            pAncestor = pAncestor->parent();
        }
    }

    if (movedPlaylistId == targetParentId) {
        return false;
    }

    emit requestPlaylistMove(movedPlaylistId, targetParentId);
    return true;
}

void SidebarModel::slotExecutePlaylistMove(int movedPlaylistId, int targetParentId) {
    PlaylistFeature* pPlaylistFeature = nullptr;
    for (LibraryFeature* pFeature : m_sFeatures) {
        if (pFeature && pFeature->title().toString() == QStringLiteral("Playlists")) {
            pPlaylistFeature = qobject_cast<PlaylistFeature*>(pFeature);
            break;
        }
    }

    if (!pPlaylistFeature) {
        return;
    }

    PlaylistDAO& liveDao = pPlaylistFeature->playlistDao();
    if (liveDao.movePlaylist(movedPlaylistId, targetParentId)) {
        qDebug() << "D&D move applied to playlist" << movedPlaylistId
                 << "parent" << targetParentId;
    } else {
        qDebug() << "D&D move failed for playlist" << movedPlaylistId;
    }
}
