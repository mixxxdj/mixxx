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

    QStyledItemDelegate::paint(painter, opt, index);
}

void DefaultDelegate::setHighlightedTextColor(
        QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    // Get the palette's text color
    QColor hlcol = option.palette.highlightedText().color();
    //  Get the 'played' or 'missing' color from the model.
    auto colorData = index.data(Qt::ForegroundRole);
    if (colorData.canConvert<QColor>()) {
        const QColor fgCol = colorData.value<QColor>();
        if (fgCol == hlcol) {
            return;
        }
        // Blend the colors 80/20
        hlcol = QColor(
                static_cast<int>(fgCol.red() * 0.8 + hlcol.red() * 0.2),
                static_cast<int>(fgCol.green() * 0.8 + hlcol.green() * 0.2),
                static_cast<int>(fgCol.blue() * 0.8 + hlcol.blue() * 0.2));
        option.palette.setColor(QPalette::Normal, QPalette::HighlightedText, hlcol);
    }
}
