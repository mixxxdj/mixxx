#include <QtDebug>

#include <QTableView>
#include <QPainter>

#include "library/locationdelegate.h"

LocationDelegate::LocationDelegate(QTableView* pTableView)
        : QStyledItemDelegate(pTableView),
          m_pTableView(pTableView) {
}

void LocationDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                         const QModelIndex& index) const {
    QString elidedLocation =
            option.fontMetrics.elidedText(index.data().toString(),
                                          Qt::ElideLeft,
                                          m_pTableView->columnWidth(index.column()));
    painter->drawText(option.rect, Qt::AlignVCenter, elidedLocation);
}
