#include "preferences/colorpaletteeditormodel.h"

#include <QList>
#include <QMap>
#include <QMultiMap>
#include <algorithm>

#include "moc_colorpaletteeditormodel.cpp"
#include "util/defs.h"
#include "util/make_const_iterator.h"
#include "util/rangelist.h"

namespace {

/// Column of the color swatch, holding a plain QStandardItem.
constexpr int kColorColumn = 0;
/// Column of the hotcue number assignment, holding a HotcueIndexListItem.
constexpr int kHotcueIndexColumn = 1;

/// Mime type used to remember the row a drag operation was started at.
/// QStandardItemModel's own mime data only encodes the item contents, not
/// their position in the model.
const QString kDragSourceRowMimeType =
        QStringLiteral("application/x-mixxx-color-palette-editor-drag-row");

QIcon toQIcon(const QColor& color) {
    QPixmap pixmap(50, 50);
    pixmap.fill(color);
    return QIcon(pixmap);
}

HotcueIndexListItem* toHotcueIndexListItem(QStandardItem* pFrom) {
    // QStandardItem::item() returns nullptr for empty cells and
    // QStandardItem::type() is a virtual function, i.e. it dereferences the
    // (null) pointer before the assertion could be evaluated.
    if (!pFrom) {
        return nullptr;
    }
    VERIFY_OR_DEBUG_ASSERT(pFrom->type() == QStandardItem::UserType) {
        return nullptr;
    }
    return static_cast<HotcueIndexListItem*>(pFrom);
}

} // namespace

ColorPaletteEditorModel::ColorPaletteEditorModel(QObject* parent)
        : QStandardItemModel(parent),
          m_bEmpty(true),
          m_bDirty(false) {
    connect(this,
            &ColorPaletteEditorModel::rowsRemoved,
            this,
            [this] {
                if (rowCount() == 0) {
                    m_bEmpty = true;
                    emit emptyChanged(true);
                }
                setDirty(true);
            });
    connect(this,
            &ColorPaletteEditorModel::rowsInserted,
            this,
            [this] {
                if (m_bEmpty && rowCount() != 0) {
                    m_bEmpty = false;
                    emit emptyChanged(true);
                }
                setDirty(true);
            });
    connect(this,
            &ColorPaletteEditorModel::rowsMoved,
            this,
            [this] {
                setDirty(true);
            });
}

QMimeData* ColorPaletteEditorModel::mimeData(const QModelIndexList& indexes) const {
    QMimeData* pMimeData = QStandardItemModel::mimeData(indexes);
    if (pMimeData && !indexes.isEmpty()) {
        // Rows are always dragged as a whole (see selectionBehavior() of the
        // view), so all indexes belong to the same row.
        pMimeData->setData(
                kDragSourceRowMimeType,
                QByteArray::number(indexes.first().row()));
    }
    return pMimeData;
}

bool ColorPaletteEditorModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
    // Always move the entire row, and don't allow column "shifting"
    Q_UNUSED(column);

    // Rows can only be reordered, never copied or dropped into another item.
    if (action != Qt::MoveAction || !data ||
            !data->hasFormat(kDragSourceRowMimeType)) {
        return false;
    }

    bool bSourceRowValid = false;
    const int sourceRow = data->data(kDragSourceRowMimeType).toInt(&bSourceRowValid);
    if (!bSourceRowValid || sourceRow < 0 || sourceRow >= rowCount()) {
        return false;
    }

    // The row is inserted above `destinationRow`; the source row is removed
    // afterwards by QAbstractItemViewPrivate::clearOrRemove(), which is why the
    // row is inserted here and not moved.
    int destinationRow = row;
    if (destinationRow < 0) {
        // The view passes row == -1 if the row was dropped onto another item
        // or onto the empty area below the last row. Note that a valid parent
        // index must not be forwarded to QStandardItemModel::insertRows(),
        // otherwise the row would be inserted below the item instead of being
        // added to the list of colors.
        destinationRow = parent.isValid() ? parent.row() : rowCount();
    }
    destinationRow = std::clamp(destinationRow, 0, rowCount());

    // QStandardItemModel::dropMimeData() must not be used here: it recreates
    // the dragged items via QStandardItemModelPrivate::createItem(), which
    // returns plain QStandardItem objects because no item prototype is set.
    // The inserted row would therefore lose its HotcueIndexListItem including
    // the hotcue index list. Moreover insertRows() pre-fills the new row with
    // null items and dropMimeData() only replaces those cells for which it can
    // compute a valid index, which may leave a row with a missing hotcue index
    // item behind. Essentially, when saving the palette  toHotcueIndexListItem()
    // would return early due to a null item and the color row would be lost.
    insertRow(destinationRow, cloneRow(sourceRow));
    return true;
}

QList<QStandardItem*> ColorPaletteEditorModel::cloneRow(int row) const {
    QList<QStandardItem*> items;
    items.reserve(columnCount());
    for (int column = 0; column < columnCount(); ++column) {
        items.append(cloneItem(item(row, column), column));
    }
    return items;
}

QStandardItem* ColorPaletteEditorModel::cloneItem(QStandardItem* pSource, int column) const {
    if (column == kHotcueIndexColumn) {
        // Copying a HotcueIndexListItem with QStandardItem::clone() would
        // return a plain QStandardItem, dropping both the item type and the
        // hotcue index list.
        HotcueIndexListItem* pHotcueIndexItem = new HotcueIndexListItem();
        if (const auto* pSourceHotcueIndexItem = toHotcueIndexListItem(pSource)) {
            pHotcueIndexItem->setHotcueIndexList(
                    pSourceHotcueIndexItem->getHotcueIndexList());
        }
        pHotcueIndexItem->setEditable(true);
        pHotcueIndexItem->setDropEnabled(false);
        return pHotcueIndexItem;
    }

    QStandardItem* pClone = pSource ? pSource->clone() : new QStandardItem();
    pClone->setEditable(false);
    pClone->setDropEnabled(false);
    return pClone;
}

bool ColorPaletteEditorModel::setData(const QModelIndex& modelIndex, const QVariant& value, int role) {
    setDirty(true);
    if (modelIndex.isValid() && modelIndex.column() == kHotcueIndexColumn) {
        const bool initialAttemptSuccessful = QStandardItemModel::setData(modelIndex, value, role);

        const auto* pHotcueIndexListItem = toHotcueIndexListItem(itemFromIndex(modelIndex));
        VERIFY_OR_DEBUG_ASSERT(pHotcueIndexListItem) {
            return false;
        }

        auto hotcueIndexList = pHotcueIndexListItem->getHotcueIndexList();

        // make sure no index is outside of range
        DEBUG_ASSERT(std::is_sorted(hotcueIndexList.constBegin(), hotcueIndexList.constEnd()));
        auto endUpper = std::upper_bound(
                hotcueIndexList.constBegin(), hotcueIndexList.constEnd(), kMaxNumberOfHotcues + 1);
        constErase(&hotcueIndexList, endUpper, hotcueIndexList.constEnd());
        auto endLower = std::upper_bound(
                hotcueIndexList.constBegin(), hotcueIndexList.constEnd(), 0);
        constErase(&hotcueIndexList, hotcueIndexList.constBegin(), endLower);

        for (int i = 0; i < rowCount(); ++i) {
            auto* pHotcueIndexListItem = toHotcueIndexListItem(item(i, kHotcueIndexColumn));

            if (pHotcueIndexListItem == nullptr) {
                continue;
            }

            if (i == modelIndex.row()) {
                pHotcueIndexListItem->setHotcueIndexList(hotcueIndexList);
            } else {
                pHotcueIndexListItem->removeIndicies(hotcueIndexList);
            }
        }

        return initialAttemptSuccessful;
    }
    return QStandardItemModel::setData(modelIndex, value, role);
}

void ColorPaletteEditorModel::setColor(int row, const QColor& color) {
    QStandardItem* pItem = item(row, kColorColumn);
    if (pItem) {
        pItem->setIcon(toQIcon(color));
        pItem->setText(color.name());
    }
    setDirty(true);
}

void ColorPaletteEditorModel::appendRow(
        const QColor& color, const QList<int>& hotcueIndicies) {
    QStandardItem* pColorItem = new QStandardItem(toQIcon(color), color.name());
    pColorItem->setEditable(false);
    pColorItem->setDropEnabled(false);

    QStandardItem* pHotcueIndexItem = new HotcueIndexListItem(hotcueIndicies);
    pHotcueIndexItem->setEditable(true);
    pHotcueIndexItem->setDropEnabled(false);

    QStandardItemModel::appendRow(
            QList<QStandardItem*>{pColorItem, pHotcueIndexItem});
}

void ColorPaletteEditorModel::setColorPalette(const ColorPalette& palette) {
    // Remove all rows
    removeRows(0, rowCount());

    // Make a map of hotcue indices
    QMultiMap<int, int> hotcueColorIndicesMap;
    QList<int> colorIndicesByHotcue = palette.getIndicesByHotcue();
    for (int i = 0; i < colorIndicesByHotcue.size(); i++) {
        int colorIndex = colorIndicesByHotcue.at(i);
        hotcueColorIndicesMap.insert(colorIndex, i + 1);
    }

    for (int i = 0; i < palette.size(); i++) {
        QColor color = mixxx::RgbColor::toQColor(palette.at(i));
        QList<int> colorIndex = hotcueColorIndicesMap.values(i);
        appendRow(color, colorIndex);
    }

    setDirty(false);
}

ColorPalette ColorPaletteEditorModel::getColorPalette(
        const QString& name) const {
    QList<mixxx::RgbColor> colors;
    QMap<int, int> hotcueColorIndices;
    for (int i = 0; i < rowCount(); i++) {
        QStandardItem* pColorItem = item(i, kColorColumn);
        const auto* pHotcueIndexItem = toHotcueIndexListItem(item(i, kHotcueIndexColumn));

        if (!pColorItem || !pHotcueIndexItem) {
            continue;
        }

        mixxx::RgbColor::optional_t color =
                mixxx::RgbColor::fromQString(pColorItem->text());

        if (color) {
            const QList<int> hotcueIndexes = pHotcueIndexItem->getHotcueIndexList();
            colors << *color;

            for (int index : hotcueIndexes) {
                hotcueColorIndices.insert(index - 1, colors.size() - 1);
            }
        }
    }
    // If we have a non consecutive list of hotcue indexes, indexes are shifted down
    // due to the sorting nature of QMap. This is intended, this way we have a color for every hotcue.
    return ColorPalette(name, colors, hotcueColorIndices.values());
}

HotcueIndexListItem::HotcueIndexListItem(const QList<int>& hotcueList)
        : QStandardItem(), m_hotcueIndexList(hotcueList) {
    std::sort(m_hotcueIndexList.begin(), m_hotcueIndexList.end());
}

HotcueIndexListItem* HotcueIndexListItem::clone() const {
    // QStandardItem::clone() would return a plain QStandardItem, losing both
    // the item type and the hotcue index list.
    return new HotcueIndexListItem(m_hotcueIndexList);
}

QVariant HotcueIndexListItem::data(int role) const {
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole: {
        return QVariant(mixxx::stringifyRangeList(m_hotcueIndexList));
    }
    default:
        return QStandardItem::data(role);
    }
}

void HotcueIndexListItem::setData(const QVariant& value, int role) {
    switch (role) {
    case Qt::EditRole: {
        const QList<int> newHotcueIndicies = mixxx::parseRangeList(value.toString());

        if (m_hotcueIndexList != newHotcueIndicies) {
            m_hotcueIndexList = newHotcueIndicies;
            emitDataChanged();
        }
        break;
    }
    default:
        QStandardItem::setData(value, role);
        break;
    }
}

void HotcueIndexListItem::removeIndicies(const QList<int>& otherIndicies) {
    DEBUG_ASSERT(std::is_sorted(otherIndicies.cbegin(), otherIndicies.cend()));
    DEBUG_ASSERT(std::is_sorted(m_hotcueIndexList.cbegin(), m_hotcueIndexList.cend()));

    QList<int> hotcueIndiciesWithOthersRemoved;
    hotcueIndiciesWithOthersRemoved.reserve(m_hotcueIndexList.size());

    std::set_difference(m_hotcueIndexList.cbegin(),
            m_hotcueIndexList.cend(),
            otherIndicies.cbegin(),
            otherIndicies.cend(),
            std::back_inserter(hotcueIndiciesWithOthersRemoved));

    if (m_hotcueIndexList != hotcueIndiciesWithOthersRemoved) {
        m_hotcueIndexList = hotcueIndiciesWithOthersRemoved;
        emitDataChanged();
    }
}
