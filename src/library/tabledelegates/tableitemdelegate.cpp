#include "library/tabledelegates/tableitemdelegate.h"

#include <QPainter>

#include "moc_tableitemdelegate.cpp"
#include "util/painterscope.h"
#include "widget/wtracktableview.h"

TableItemDelegate::TableItemDelegate(QTableView* pTableView)
        : DefaultDelegate(pTableView),
          m_pTableView(pTableView) {
    DEBUG_ASSERT(m_pTableView);
    auto* pTrackTableView = qobject_cast<WTrackTableView*>(m_pTableView);
    if (pTrackTableView) {
        // This gets a color from the stylesheet:
        // WTrackTableView {
        //   qproperty-focusBorderColor: red;
        // }
        m_focusBorderColor = pTrackTableView->getFocusBorderColor();
        // For some reason the color is not initialized from the stylesheet for
        // some WTrackTableViews (in DlgAutoDJ, DlgAnalysis, ...)
        // Listen to the property changed signal.
        connect(pTrackTableView,
                &WTrackTableView::focusBorderColorChanged,
                this,
                [this](QColor col) {
                    m_focusBorderColor = col;
                });
    }
}

void TableItemDelegate::paint(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    PainterScope painterScope(painter);
    painter->setClipRect(option.rect);

    // Clone the const option so we can change palette colors.
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    if (opt.state & QStyle::State_Selected) {
        setHighlightedTextColor(opt, index);
    }

    QBrush brush;
    if (opt.state & QStyle::State_Selected) {
        brush = opt.palette.highlightedText();
    } else {
        brush = opt.palette.text();
    }
    // Brush color for the star rating polygons:
    painter->setBrush(brush);
    // Pen for the 'location' text
    // Note: seems not to be required anymore (Qt 6.2.3)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    painter->setPen(brush.color());
#endif

    QStyle* style = m_pTableView->style();
    if (style) {
        style->drawControl(
                QStyle::CE_ItemViewItem,
                // Use the original option here to not screw up
                // the Key and Preview delegates.
                &option,
                painter,
                m_pTableView);
    }

    paintItem(painter, opt, index);
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

// static
void TableItemDelegate::drawBorder(
        QPainter* painter,
        const QColor borderColor,
        const QRect& rect) {
    // Draws a border around the cell with official skin's default focus border width
    QPen borderPen(
            borderColor,
            1,
            Qt::SolidLine,
            Qt::SquareCap);
    painter->setPen(borderPen);
    painter->setBrush(QBrush(Qt::transparent));
    painter->drawRect(
            rect.left(),
            rect.top(),
            rect.width() - 1,
            rect.height() - 1);
}

void TableItemDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    // We don't want to call DefaultDelegate::paint() because that would again
    // try to set the 'selected' color (which we already did in paint() above).
    // Relevant only for derived classes that don't implement paintItem()
    QStyledItemDelegate::paint(painter, option, index); // clazy:exclude=skipped-base-method
}
