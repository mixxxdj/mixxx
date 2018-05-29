
#include <QTableView>

#include "library/tableitemdelegate.h"


TableItemDelegate::TableItemDelegate(QTableView* pTableView)
        : QStyledItemDelegate(pTableView),
          m_pTableView(pTableView) {
}

TableItemDelegate::~TableItemDelegate() {
}

void TableItemDelegate::paint(QPainter* painter,const QStyleOptionViewItem& option,
                        const QModelIndex& index) const {
    paintItem(painter, option, index);
}
