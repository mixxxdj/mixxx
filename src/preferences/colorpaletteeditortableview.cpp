#include "preferences/colorpaletteeditortableview.h"

#include <QColorDialog>
#include <QHeaderView>
#include <QMenu>

namespace {
const QColor kDefaultPaletteColor(0, 0, 0);

QIcon toQIcon(const QColor& color) {
    QPixmap pixmap(50, 50);
    pixmap.fill(color);
    return QIcon(pixmap);
}
} // namespace

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
    horizontalHeader()->setVisible(false);
    // Set our custom model - this prevents row "shifting"
    setModel(m_model);

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
                QStandardItem* item = m_model->itemFromIndex(index);
                if (!item) {
                    return;
                }
                QColor color = QColorDialog::getColor();
                item->setIcon(toQIcon(color));
                item->setText(color.name());
                setDirty(true);
            });
    connect(this, &ColorPaletteEditorTableView::customContextMenuRequested, [this](const QPoint& pos) {
        QMenu menu(this);

        QAction* pAddAction = menu.addAction("Add");
        QAction* pRemoveAction = menu.addAction("Remove");
        QAction* pAction = menu.exec(viewport()->mapToGlobal(pos));
        if (pAction == pAddAction) {
            QStandardItem* item = new QStandardItem(
                    toQIcon(kDefaultPaletteColor), kDefaultPaletteColor.name());
            item->setEditable(false);
            item->setDropEnabled(false);
            m_model->appendRow(item);
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

void ColorPaletteEditorTableView::setColors(const QList<mixxx::RgbColor>& colors) {
    m_model->clear();
    foreach (const mixxx::RgbColor rgbColor, colors) {
        QColor color = mixxx::RgbColor::toQColor(rgbColor);
        QIcon icon = toQIcon(color);
        QString colorName = color.name();

        QStandardItem* item = new QStandardItem(icon, colorName);
        item->setEditable(false);
        item->setDropEnabled(false);
        m_model->appendRow(item);
    }
    setDirty(false);
}

QList<mixxx::RgbColor> ColorPaletteEditorTableView::getColors() const {
    QList<mixxx::RgbColor> colors;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QStandardItem* item = m_model->item(i, 0);
        mixxx::RgbColor::optional_t color = mixxx::RgbColor::fromQString(item->text());
        if (color) {
            colors << *color;
        }
    }
    return colors;
}
