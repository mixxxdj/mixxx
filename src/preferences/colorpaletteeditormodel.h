#pragma once
#include <QStandardItemModel>

#include "util/color/colorpalette.h"

class ColorPaletteEditorModel : public QStandardItemModel {
    Q_OBJECT
  public:
    static constexpr int kNoHotcueIndex = -1;

    ColorPaletteEditorModel(int rows, int columns, QObject* parent = nullptr)
            : QStandardItemModel(rows, columns, parent) {
    }
    ColorPaletteEditorModel(QObject* parent = nullptr)
            : QStandardItemModel(parent) {
    }

    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

    void setColor(int row, const QColor& color);
    void setHotcueIndex(int row, int hotcueIndex);
    void appendRow(const QColor& color, int hotcueIndex = kNoHotcueIndex);

    void setColorPalette(const ColorPalette& palette);
    ColorPalette getColorPalette(const QString& name) const;
};
