#include "preferences/colorpaletteeditortableview.h"

#include <QColorDialog>
#include <QHeaderView>
#include <QMenu>

namespace {
const QColor kDefaultPaletteColor(0, 0, 0);
}

ColorPaletteEditorTableView::ColorPaletteEditorTableView(QWidget* parent)
        : QTableView(parent),
          m_bDirty(false),
          m_model(new ColorPaletteEditorModel()) {
    setShowGrid(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDragDropOverwriteMode(false);
    setContextMenuPolicy(Qt::CustomContextMenu);

    // Set our custom model - this prevents row "shifting"
    setModel(m_model);
    m_model->setColumnCount(2);
    m_model->setHeaderData(0, Qt::Horizontal, tr("Color"), Qt::DisplayRole);
    m_model->setHeaderData(1, Qt::Horizontal, tr("Assign to Hotcue"), Qt::DisplayRole);

    connect(m_model,
            &ColorPaletteEditorModel::rowsMoved,
            [this] {
                setDirty(true);
            });
    connect(m_model,
            &ColorPaletteEditorModel::rowsInserted,
            [this] {
                setDirty(true);
            });
    connect(m_model,
            &ColorPaletteEditorModel::rowsRemoved,
            [this] {
                setDirty(true);
            });
    connect(this,
            &ColorPaletteEditorTableView::doubleClicked,
            [this](const QModelIndex& index) {
                if (index.isValid() && index.column() == 0) {
                    QColor color = QColorDialog::getColor();
                    if (color.isValid()) {
                        m_model->setColor(index.row(), color);
                        setDirty(true);
                    }
                }
            });
    connect(this, &ColorPaletteEditorTableView::customContextMenuRequested, [this](const QPoint& pos) {
        QMenu menu(this);

        QAction* pAddAction = menu.addAction("Add");
        QAction* pRemoveAction = menu.addAction("Remove");
        QAction* pAction = menu.exec(viewport()->mapToGlobal(pos));
        if (pAction == pAddAction) {
            m_model->appendRow(kDefaultPaletteColor);
        } else if (pAction == pRemoveAction) {
            QModelIndexList selection = selectionModel()->selectedRows();

            if (selection.count() > 0) {
                QModelIndex index = selection.at(0);

                //row selected
                int row = index.row();
                m_model->removeRow(row);
            }
        }
    });
}
void ColorPaletteEditorTableView::setColorPalette(const ColorPalette& palette) {
    m_model->setColorPalette(palette);
    setDirty(false);
}

ColorPalette ColorPaletteEditorTableView::getColorPalette(const QString& name) const {
    return m_model->getColorPalette(name);
}
