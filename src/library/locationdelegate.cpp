#include "library/locationdelegate.h"

#include <QPainter>


LocationDelegate::LocationDelegate(QTableView* pTableView)
        : TableItemDelegate(pTableView) {
}

void LocationDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    paintItemBackground(painter, option, index);

    QString elidedText = option.fontMetrics.elidedText(
            index.data().toString(),
            Qt::ElideLeft,
            columnWidth(index));
    painter->drawText(option.rect, Qt::AlignVCenter, elidedText);
}
