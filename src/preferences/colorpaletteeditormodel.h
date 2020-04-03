#pragma once
#include <QStandardItemModel>

#include "util/color/colorpalette.h"

// Model that is used by the QTableView of the ColorPaletteEditor.
// Takes of displaying palette colors and provides a getter/setter for
// ColorPalette instances.
class ColorPaletteEditorModel : public QStandardItemModel {
    Q_OBJECT
  public:
    static constexpr int kNoHotcueIndex = -1;

    ColorPaletteEditorModel(QObject* parent = nullptr);

    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    void setColor(int row, const QColor& color);
    void appendRow(const QColor& color, int hotcueIndex = kNoHotcueIndex);

    void setDirty(bool bDirty) {
        if (m_bDirty == bDirty) {
            return;
        }
        m_bDirty = bDirty;
        emit dirtyChanged(m_bDirty);
    }

    bool isDirty() const {
        return m_bDirty;
    }

    bool isEmpty() const {
        return m_bEmpty;
    }

    void setColorPalette(const ColorPalette& palette);
    ColorPalette getColorPalette(const QString& name) const;

  signals:
    void emptyChanged(bool bIsEmpty);
    void dirtyChanged(bool bIsDirty);

  private:
    bool m_bEmpty;
    bool m_bDirty;
};
