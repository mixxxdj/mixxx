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

QIcon toQIcon(const QColor& color) {
    QPixmap pixmap(50, 50);
    pixmap.fill(color);
    return QIcon(pixmap);
}

HotcueIndexListItem* toHotcueIndexListItem(QStandardItem* pFrom) {
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

bool ColorPaletteEditorModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
    // Always move the entire row, and don't allow column "shifting"
    Q_UNUSED(column);
    return QStandardItemModel::dropMimeData(data, action, row, 0, parent);
}

bool ColorPaletteEditorModel::setData(const QModelIndex& modelIndex, const QVariant& value, int role) {
    setDirty(true);
    if (modelIndex.isValid() && modelIndex.column() == 1) {
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
            auto* pHotcueIndexListItem = toHotcueIndexListItem(item(i, 1));

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
    QStandardItem* pItem = item(row, 0);
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
        QStandardItem* pColorItem = item(i, 0);

        const auto* pHotcueIndexItem = toHotcueIndexListItem(item(i, 1));
        if (!pHotcueIndexItem) {
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
