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

    // Workaround for a Qt6 bug occurring on Wayland (maybe also with other OS or
    // compositors):
    // Paint background color from model if available and not selected to
    // ensure the alpha channel is respected.
    paintItemBackground(painter, opt, index);
    // Clear the background brush so QStyledItemDelegate doesn't paint it again
    opt.backgroundBrush = QBrush();

    if (opt.state & QStyle::State_Selected) {
        setHighlightedTextColor(opt, index);
    }
    // TODO Guarantee font/bg contrast with ALL track colors

    QStyledItemDelegate::paint(painter, opt, index);
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
