#include "library/tableitemdelegate.h"

#include <QPainter>
#include <QTableView>

#include "moc_tableitemdelegate.cpp"
#include "util/painterscope.h"
#include "widget/wtracktableview.h"

TableItemDelegate::TableItemDelegate(QTableView* pTableView)
        : QStyledItemDelegate(pTableView),
          m_pTableView(pTableView) {
    DEBUG_ASSERT(m_pTableView);
    auto* pTrackTableView = qobject_cast<WTrackTableView*>(m_pTableView);
    if (pTrackTableView) {
        m_pFocusBorderColor = pTrackTableView->getFocusBorderColor();
    }
}

void TableItemDelegate::paint(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    PainterScope painterScope(painter);
    painter->setClipRect(option.rect);

    // Set the palette appropriately based on whether the row is selected or
    // not. We also have to check if it is inactive or not and use the
    // appropriate ColorGroup.
    QPalette::ColorGroup cg = QPalette::Disabled;
    if ((option.state & QStyle::State_Enabled) &&
        (option.state & QStyle::State_Active)) {
        cg = QPalette::Normal;
    }

    if (option.state & QStyle::State_Selected) {
        painter->setBrush(option.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setBrush(option.palette.color(cg, QPalette::Text));
    }

    QStyle* style = m_pTableView->style();
    if (style) {
        style->drawControl(
                QStyle::CE_ItemViewItem,
                &option,
                painter,
                m_pTableView);
    }

    paintItem(painter, option, index);
}

int TableItemDelegate::columnWidth(const QModelIndex &index) const {
    return m_pTableView->columnWidth(index.column());
}

void TableItemDelegate::paintItemBackground(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) {
    // If the row is not selected, paint the desired background color before
    // painting the delegate item
    if (option.showDecorationSelected &&
            (option.state & QStyle::State_Selected)) {
        return;
    }
    QVariant bgValue = index.data(Qt::BackgroundRole);
    if (!bgValue.isValid()) {
        return;
    }
    DEBUG_ASSERT(bgValue.canConvert<QBrush>());
    const auto bgBrush = qvariant_cast<QBrush>(bgValue);
    painter->fillRect(option.rect, bgBrush);
}
