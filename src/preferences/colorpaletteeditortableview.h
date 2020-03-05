#pragma once
#include <QStandardItemModel>
#include <QTableView>

#include "util/color/rgbcolor.h"

class ColorPaletteEditorModel : public QStandardItemModel {
    Q_OBJECT
  public:
    ColorPaletteEditorModel(int rows, int columns, QObject* parent = nullptr)
            : QStandardItemModel(rows, columns, parent) {
    }
    ColorPaletteEditorModel(QObject* parent = nullptr)
            : QStandardItemModel(parent) {
    }

    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override {
        // Always move the entire row, and don't allow column "shifting"
        Q_UNUSED(column);
        return QStandardItemModel::dropMimeData(data, action, row, 0, parent);
    }
};

class ColorPaletteEditorTableView : public QTableView {
    Q_OBJECT
  public:
    ColorPaletteEditorTableView(QWidget* parent = nullptr);

    ~ColorPaletteEditorTableView() {
        delete m_model;
    }

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

    QList<mixxx::RgbColor> getColors() const;
    void setColors(const QList<mixxx::RgbColor>& colors);

  signals:
    void dirtyChanged(bool bDirty);

  private:
    bool m_bDirty;
    ColorPaletteEditorModel* m_model;
};
