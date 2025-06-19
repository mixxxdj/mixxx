#include "library/tabledelegates/defaultdelegate.h"

#include <QPainter>

#include "moc_defaultdelegate.cpp"

DefaultDelegate::DefaultDelegate(QTableView* pTableView)
        : QStyledItemDelegate(pTableView) {
}

void DefaultDelegate::paint(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    if (opt.state & QStyle::State_Selected) {
        setHighlightedTextColor(opt, index);
    }
    // TODO Guarantee font/bg contrast with ALL track colors

    QStyledItemDelegate::paint(painter, opt, index);
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
