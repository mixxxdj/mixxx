#include "library/tabledelegates/keydelegate.h"

#include <QPainter>
#include <QStyle>
#include <QTableView>

#include "library/trackmodel.h"
#include "moc_keydelegate.cpp"

namespace {
// Unicode symbol for 432Hz indicator
constexpr const char* k432HzSymbol = "\xE2\x9C\xA7"; // ✧ (sparkle symbol)
constexpr int k432HzSymbolWidth = 14;
} // namespace

void KeyDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    paintItemBackground(painter, option, index);

    const QString keyText = index.data().value<QString>();
    const QColor keyColor = index.data(Qt::DecorationRole).value<QColor>();
    const bool is432Hz = index.data(TrackModel::k432HzRole).toBool();
    int rectWidth = 0;

    if (keyColor.isValid()) {
        // Draw the colored rectangle next to the key label
        rectWidth = 8; //  4px width + 4px right padding
        painter->fillRect(
                option.rect.x(),
                option.rect.y() + 2,
                4, // width
                option.rect.height() - 4,
                keyColor);
    }

    // Reserve space for 432Hz symbol if needed
    int rightMargin = is432Hz ? k432HzSymbolWidth : 0;

    // Display the key text with the user-provided notation
    QString elidedText = option.fontMetrics.elidedText(
            keyText,
            Qt::ElideRight,
            columnWidth(index) - rectWidth - rightMargin);

    if (option.state & QStyle::State_Selected) {
        // This uses selection-color from stylesheet for the text pen:
        // #LibraryContainer QTableView {
        //   selection-color: #fff;
        // }
        painter->setPen(QPen(option.palette.highlightedText().color()));
    }

    painter->drawText(option.rect.x() + rectWidth,
            option.rect.y(),
            option.rect.width() - rectWidth - rightMargin,
            option.rect.height(),
            Qt::AlignVCenter,
            elidedText);

    // Draw 432Hz indicator symbol if the track is tuned to 432Hz
    if (is432Hz) {
        painter->save();
        // Use a golden/warm color for the 432Hz symbol
        QColor symbolColor(218, 165, 32); // Golden color
        if (option.state & QStyle::State_Selected) {
            // Use a brighter color when selected
            symbolColor = QColor(255, 215, 0); // Brighter gold
        }
        painter->setPen(symbolColor);
        QFont symbolFont = option.font;
        symbolFont.setBold(true);
        painter->setFont(symbolFont);
        painter->drawText(
                option.rect.x() + option.rect.width() - k432HzSymbolWidth,
                option.rect.y(),
                k432HzSymbolWidth,
                option.rect.height(),
                Qt::AlignVCenter | Qt::AlignRight,
                QString::fromUtf8(k432HzSymbol));
        painter->restore();
    }

    // Draw a border if the key cell has focus
    if (option.state & QStyle::State_HasFocus) {
        drawBorder(painter, m_focusBorderColor, option.rect);
    }
}
