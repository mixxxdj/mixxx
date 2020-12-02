#include "library/locationdelegate.h"

#include <QFlags>
#include <QFontMetrics>
#include <QModelIndex>
#include <QPainter>
#include <QPalette>
#include <QPen>
#include <QStyle>
#include <QVariant>
#include <Qt>
#include <QtGui>

class QTableView;

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
