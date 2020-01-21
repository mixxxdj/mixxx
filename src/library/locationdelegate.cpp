#include "library/locationdelegate.h"

#include <QPainter>

#include "moc_locationdelegate.cpp"

LocationDelegate::LocationDelegate(QTableView* pTableView)
        : TableItemDelegate(pTableView) {
}

void LocationDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    paintItemBackground(painter, option, index);

    if (option.state & QStyle::State_Selected) {
        // This uses selection-color from stylesheet for the text pen:
        // #LibraryContainer QTableView {
        //   selection-color: #fff;
        // }
        painter->setPen(QPen(option.palette.highlightedText().color()));
    }
    QString elidedText = option.fontMetrics.elidedText(
            index.data().toString(),
            Qt::ElideLeft,
            columnWidth(index));
    painter->drawText(option.rect, Qt::AlignVCenter, elidedText);
}
