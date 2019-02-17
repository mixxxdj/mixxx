
#include <QTableView>
#include <QPainter>

#include "library/tableitemdelegate.h"


TableItemDelegate::TableItemDelegate(QTableView* pTableView)
        : QStyledItemDelegate(pTableView),
          m_pTableView(pTableView) {
}

TableItemDelegate::~TableItemDelegate() {
}

void TableItemDelegate::paint(QPainter* painter,const QStyleOptionViewItem& option,
                              const QModelIndex& index) const {

    painter->save();

    painter->setClipRect(option.rect);

    // Set the palette appropriately based on whether the row is selected or
    // not. We also have to check if it is inactive or not and use the
    // appropriate ColorGroup.
    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
            ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
        cg = QPalette::Inactive;

    if (option.state & QStyle::State_Selected) {
        painter->setBrush(option.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setBrush(option.palette.color(cg, QPalette::Text));
    }

    if (m_pTableView) {
        QStyle* style = m_pTableView->style();
        if (style) {
            style->drawControl(QStyle::CE_ItemViewItem, &option, painter,
                    m_pTableView);
        }
    }

    paintItem(painter, option, index);

    painter->restore();
}
