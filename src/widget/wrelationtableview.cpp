#include "widget/wrelationtableview.h"

#include <QPainter>

#include "moc_wrelationtableview.cpp"

WRelationTableView::WRelationTableView(
        QWidget* parent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        double trackTableBackgroundColorOpacity)
        : WTrackTableView(parent,
                  pConfig,
                  pLibrary,
                  trackTableBackgroundColorOpacity) {
    setDragDropMode(QAbstractItemView::DragOnly);
    setDragEnabled(true);
}

void WRelationTableView::paintEvent(QPaintEvent* event) {
    QTableView::paintEvent(event);

    QPainter painter(viewport());
    QPen pen(QColor(60, 60, 60), 1);
    painter.setPen(pen);

    int rowCount = model()->rowCount();
    for (int row = 1; row < rowCount; ++row) {
        if (row % 2 == 0) {
            int y = rowViewportPosition(row);
            painter.drawLine(0, y, viewport()->width(), y);
        }
    }
}
