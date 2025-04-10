#include "library/tabledelegates/tableitemdelegate.h"

#include <QPainter>

#include "moc_tableitemdelegate.cpp"
#include "util/painterscope.h"
#include "widget/wtracktableview.h"

TableItemDelegate::TableItemDelegate(QTableView* pTableView)
        : QStyledItemDelegate(pTableView),
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
        // This gets the custom 'missing' or played text color from BaseTrackTableModel
        // depending on check state of the (hidden) 'missing' (fs_deleted)
        // or 'played' columns.
        // Note that we need to do this again in BPMDelegate which uses the
        // style of the TableView.
        auto customColorData = index.data(Qt::ForegroundRole);
        if (customColorData.canConvert<QColor>()) {
            QColor customColor = customColorData.value<QColor>();
            // for the star rating polygons
            painter->setBrush(customColor);
            // for the 'location' text
            painter->setPen(customColor);
        } else {
            painter->setBrush(option.palette.color(cg, QPalette::Text));
        }
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
    QStyledItemDelegate::paint(painter, option, index);
}
