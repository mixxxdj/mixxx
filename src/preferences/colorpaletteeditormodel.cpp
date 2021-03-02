#include "preferences/colorpaletteeditormodel.h"

#include "moc_colorpaletteeditormodel.cpp"

namespace {

QIcon toQIcon(const QColor& color) {
    QPixmap pixmap(50, 50);
    pixmap.fill(color);
    return QIcon(pixmap);
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
        bool ok;
        int hotcueIndex = value.toInt(&ok);

        // Make sure that the value is valid
        if (!ok || hotcueIndex <= 0 || hotcueIndex > rowCount()) {
            return QStandardItemModel::setData(modelIndex, QVariant(), role);
        }

        // Make sure there is no other row with the same hotcue index
        for (int i = 0; i < rowCount(); i++) {
            QModelIndex otherModelIndex = index(i, 1);
            QVariant otherValue = data(otherModelIndex);
            int otherHotcueIndex = otherValue.toInt(&ok);
            if (ok && otherHotcueIndex == hotcueIndex) {
                QStandardItemModel::setData(otherModelIndex, QVariant(), role);
            }
        }
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

void ColorPaletteEditorModel::appendRow(const QColor& color, int hotcueIndex) {
    QStandardItem* pColorItem = new QStandardItem(toQIcon(color), color.name());
    pColorItem->setEditable(false);
    pColorItem->setDropEnabled(false);

    QString hotcueIndexStr;
    if (hotcueIndex >= 0) {
        hotcueIndexStr = QString::number(hotcueIndex + 1);
    }

    QStandardItem* pHotcueIndexItem = new QStandardItem(hotcueIndexStr);
    pHotcueIndexItem->setEditable(true);
    pHotcueIndexItem->setDropEnabled(false);

    QStandardItemModel::appendRow(QList<QStandardItem*>{pColorItem, pHotcueIndexItem});
}

void ColorPaletteEditorModel::setColorPalette(const ColorPalette& palette) {
    // Remove all rows
    removeRows(0, rowCount());

    // Make a map of hotcue indices
    QMap<int, int> hotcueColorIndicesMap;
    QList<int> colorIndicesByHotcue = palette.getIndicesByHotcue();
    for (int i = 0; i < colorIndicesByHotcue.size(); i++) {
        int colorIndex = colorIndicesByHotcue.at(i);
        hotcueColorIndicesMap.insert(colorIndex, i);
    }

    for (int i = 0; i < palette.size(); i++) {
        QColor color = mixxx::RgbColor::toQColor(palette.at(i));
        int colorIndex = hotcueColorIndicesMap.value(i, kNoHotcueIndex);
        appendRow(color, colorIndex);
    }

    setDirty(false);
}

ColorPalette ColorPaletteEditorModel::getColorPalette(const QString& name) const {
    QList<mixxx::RgbColor> colors;
    QMap<int, int> hotcueColorIndices;
    for (int i = 0; i < rowCount(); i++) {
        QStandardItem* pColorItem = item(i, 0);
        QStandardItem* pHotcueIndexItem = item(i, 1);
        mixxx::RgbColor::optional_t color = mixxx::RgbColor::fromQString(pColorItem->text());
        if (color) {
            colors << *color;

            bool ok;
            int hotcueIndex = pHotcueIndexItem->text().toInt(&ok);
            if (ok) {
                hotcueColorIndices.insert(hotcueIndex - 1, colors.size() - 1);
            }
        }
    }
    // If we have a non consequitive list of hotcue indexes, indexes are shifted down
    // due to the sorting nature of QMap. This is intended, this way we have a color for every hotcue.
    return ColorPalette(name, colors, hotcueColorIndices.values());
}
