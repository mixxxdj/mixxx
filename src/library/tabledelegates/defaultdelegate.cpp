#include "library/tabledelegates/defaultdelegate.h"

#include <QPainter>

#include "moc_defaultdelegate.cpp"
#include "util/assert.h"

DefaultDelegate::DefaultDelegate(QTableView* pTableView)
        : QStyledItemDelegate(pTableView) {
}

void DefaultDelegate::paint(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // Workaround for a Qt6 bug occurring on Wayland with Breeze and Adwaita
    // theme (maybe also with other OS or compositors),
    // see https://github.com/mixxxdj/mixxx/issues/17037:
    // Paint background color from model if available and not selected to
    // ensure the alpha channel is respected.
    paintItemBackground(painter, opt, index);
    // Clear the background brush so the style doesn't paint it a second time.
    opt.backgroundBrush = QBrush();
    // KDE's Breeze (and possibly similar styles) paints the alternate row
    // background inside CE_ItemViewItem. That opaque fill would cover the
    // translucent track color we just painted. It has already been painted by
    // PE_PanelItemViewRow before the delegate ran (QTableViewPrivate::drawCell),
    // so drop the flag to prevent repainting it.
    opt.features &= ~QStyleOptionViewItem::Alternate;

    if (opt.state & QStyle::State_Selected) {
        setHighlightedTextColor(opt, index);
    }
    // TODO Guarantee font/bg contrast with ALL track colors

    // Draw the item ourselves instead of calling QStyledItemDelegate::paint():
    // that would call initStyleOption() a second time, re-applying
    // Qt::BackgroundRole to backgroundBrush, and the style would paint the
    // background again on top of our manual fill (= double opacity).
    // Note: the view always sets option.widget in initViewItemOption(), so we
    // don't need a parent() fallback for paint() calls without a view option.
    QStyle* pStyle = opt.widget ? opt.widget->style() : nullptr;
    if (pStyle) {
        pStyle->drawControl(
                QStyle::CE_ItemViewItem,
                &opt,
                painter,
                opt.widget);
    }
}

void DefaultDelegate::paintItemBackground(
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

void DefaultDelegate::setHighlightedTextColor(
        QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    // Get the palette's selected text color
    QColor hlColor = option.palette.highlightedText().color();
    //  Get the 'played' or 'missing' color from the model.
    auto colorData = index.data(Qt::ForegroundRole);
    if (colorData.canConvert<QColor>()) {
        const QColor fgColor = colorData.value<QColor>();
        if (fgColor == hlColor) {
            return;
        }
        // Blend the colors 50/50
        hlColor = QColor(
                static_cast<int>((fgColor.red() + hlColor.red()) / 2),
                static_cast<int>((fgColor.green() + hlColor.green()) / 2),
                static_cast<int>((fgColor.blue() + hlColor.blue()) / 2));
        option.palette.setColor(QPalette::Normal, QPalette::HighlightedText, hlColor);
        option.palette.setColor(QPalette::Inactive, QPalette::HighlightedText, hlColor);
    }
}
