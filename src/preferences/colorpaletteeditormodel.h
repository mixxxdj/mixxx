#pragma once
#include <QStandardItemModel>

#include "util/color/colorpalette.h"

class ColorPaletteEditorModel : public QStandardItemModel {
    Q_OBJECT
  public:
    static constexpr int kNoHotcueIndex = -1;

    ColorPaletteEditorModel(QObject* parent = nullptr);

    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    void setColor(int row, const QColor& color);
    void setHotcueIndex(int row, int hotcueIndex);
    void appendRow(const QColor& color, int hotcueIndex = kNoHotcueIndex);

    void setDirty(bool bDirty) {
        if (m_bDirty == bDirty) {
            return;
        }
        m_bDirty = bDirty;
        emit dirtyChanged(m_bDirty);
    }

    bool isDirty() {
        return m_bDirty;
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
