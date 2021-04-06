#include "preferences/colorpaletteeditormodel.h"

#include <util/assert.h>

#include <QList>
#include <QMap>
#include <QMultiMap>
#include <QRegularExpression>
#include <algorithm>

#include "moc_colorpaletteeditormodel.cpp"

namespace {

QIcon toQIcon(const QColor& color) {
    QPixmap pixmap(50, 50);
    pixmap.fill(color);
    return QIcon(pixmap);
}

const QRegularExpression hotcueListMatchingRegex(QStringLiteral("(\\d+)[ ,]*"));

HotcueIndexListItem* toHotcueIndexListItem(QStandardItem* from) {
    VERIFY_OR_DEBUG_ASSERT(from->type() == QStandardItem::UserType) {
        // does std::optional make sense for pointers?
        return nullptr;
    }
    return static_cast<HotcueIndexListItem*>(from);
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
    if (modelIndex.isValid() && modelIndex.column() == 1) {
        const auto rollbackData = itemFromIndex(modelIndex)->data(role);

        // parse and validate in color-context
        const bool initialAttemptSuccessful = QStandardItemModel::setData(modelIndex, value, role);

        QList<int> allHotcueIndicies;
        allHotcueIndicies.reserve(rowCount());

        for (int i = 0; i < rowCount(); ++i) {
            auto* hotcueIndexListItem = toHotcueIndexListItem(item(i, 1));

            if (hotcueIndexListItem) {
                allHotcueIndicies.append(hotcueIndexListItem->getHotcueIndexList());
            }
        }

        // validate hotcueindicies in palette context
        // checks for duplicates and validates largest index

        const int preDedupLen = allHotcueIndicies.length();

        std::sort(allHotcueIndicies.begin(), allHotcueIndicies.end());
        const auto end = std::unique(allHotcueIndicies.begin(), allHotcueIndicies.end());
        allHotcueIndicies.erase(end, allHotcueIndicies.end());

        const bool isOutsidePalette = !allHotcueIndicies.empty() &&
                allHotcueIndicies.last() > rowCount();

        if (preDedupLen != allHotcueIndicies.length() || isOutsidePalette) {
            // checks failed!
            // rollback cell content to previous hotcue index list
            return QStandardItemModel::setData(modelIndex, rollbackData, role);
        } else {
            setDirty(true);
            return initialAttemptSuccessful;
        }
    }

    setDirty(true);
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

        auto* pHotcueIndexItem = toHotcueIndexListItem(item(i, 1));
        if (!pHotcueIndexItem)
            continue;

        mixxx::RgbColor::optional_t color =
                mixxx::RgbColor::fromQString(pColorItem->text());

        if (color) {
            QList<int> hotcueIndexes = pHotcueIndexItem->getHotcueIndexList();
            colors << *color;

            for (int index : qAsConst(hotcueIndexes)) {
                hotcueColorIndices.insert(index - 1, colors.size() - 1);
            }
        }
    }
    // If we have a non consequitive list of hotcue indexes, indexes are shifted down
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
        QString serializedIndexStrings;
        for (int i = 0; i < m_hotcueIndexList.size(); ++i) {
            serializedIndexStrings += QString::number(m_hotcueIndexList.at(i));
            if (i < m_hotcueIndexList.size() - 1) {
                serializedIndexStrings += QStringLiteral(", ");
            }
        }
        return QVariant(serializedIndexStrings);
        break;
    }
    default:
        return QStandardItem::data(role);
        break;
    }
}

void HotcueIndexListItem::setData(const QVariant& value, int role) {
    switch (role) {
    case Qt::EditRole: {
        m_hotcueIndexList.clear();
        QRegularExpressionMatchIterator regexResults =
                hotcueListMatchingRegex.globalMatch(value.toString());
        while (regexResults.hasNext()) {
            QRegularExpressionMatch match = regexResults.next();
            bool ok;
            const int hotcueIndex = match.captured(1).toInt(&ok);
            if (ok && !m_hotcueIndexList.contains(hotcueIndex)) {
                m_hotcueIndexList.append(hotcueIndex);
            }
        }
        std::sort(m_hotcueIndexList.begin(), m_hotcueIndexList.end());

        emitDataChanged();
        break;
    }
    default:
        QStandardItem::setData(value, role);
        break;
    }
}
