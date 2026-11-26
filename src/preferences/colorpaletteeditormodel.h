#pragma once

#include <QMimeData>
#include <QModelIndex>
#include <QModelIndexList>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QVariant>

#include "util/color/colorpalette.h"

// Model that is used by the QTableView of the ColorPaletteEditor.
// Takes care of displaying palette colors and provides a getter/setter for
// ColorPalette instances.
class ColorPaletteEditorModel : public QStandardItemModel {
    Q_OBJECT
  public:
    ColorPaletteEditorModel(QObject* parent = nullptr);

    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    void setColor(int row, const QColor& color);
    void appendRow(const QColor& color, const QList<int>& hotcueIndicies);

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
    /// Returns a copy of all items of the given row. The hotcue index cell is
    /// copied into another HotcueIndexListItem, i.e. the row keeps its type
    /// information. Empty cells are replaced by a new item of the matching
    /// type, so that a copied row never contains null items.
    QList<QStandardItem*> cloneRow(int row) const;
    QStandardItem* cloneItem(QStandardItem* pSource, int column) const;

    bool m_bEmpty;
    bool m_bDirty;
};

class HotcueIndexListItem : public QStandardItem {
  public:
    HotcueIndexListItem(const QList<int>& hotcueList = {});

    HotcueIndexListItem* clone() const override;

    void setData(const QVariant& value, int role = Qt::UserRole + 1) override;
    QVariant data(int role = Qt::UserRole + 1) const override;

    int type() const override {
        return QStandardItem::UserType;
    };

    const QList<int>& getHotcueIndexList() const {
        return m_hotcueIndexList;
    }
    void setHotcueIndexList(const QList<int>& list) {
        m_hotcueIndexList = QList(list);
    }

    void removeIndicies(const QList<int>& otherIndicies);

  private:
    QList<int> m_hotcueIndexList;
};
