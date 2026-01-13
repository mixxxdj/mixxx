#include "library/tabledelegates/keydelegate.h"

#include <QPainter>
#include <QStyle>
#include <QTableView>

#include "library/trackmodel.h"
#include "moc_keydelegate.cpp"

namespace {
// Unicode symbols for tuning indicators
constexpr const char* kTuningSymbol432Hz = "\xE2\x9C\xA7"; // ✧ (sparkle) for 432Hz
constexpr const char* kTuningSymbolLow = "\xE2\x86\x93";   // ↓ (arrow down) for <440Hz
constexpr const char* kTuningSymbolHigh = "\xE2\x86\x91";  // ↑ (arrow up) for >440Hz
constexpr int kTuningSymbolWidth = 14;
constexpr int kStandardTuningHz = 440;
} // namespace

void KeyDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    paintItemBackground(painter, option, index);

    const QString keyText = index.data().value<QString>();
    const QColor keyColor = index.data(Qt::DecorationRole).value<QColor>();
    const bool is432Hz = index.data(TrackModel::k432HzRole).toBool();
    const int tuningFrequencyHz = index.data(TrackModel::kTuningFrequencyRole).toInt();
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

    // Determine which tuning symbol to show (if any)
    const char* tuningSymbol = nullptr;
    QColor symbolColor;
    if (is432Hz || tuningFrequencyHz == 432) {
        // 432Hz gets the sparkle symbol
        tuningSymbol = kTuningSymbol432Hz;
        symbolColor = QColor(218, 165, 32); // Golden color
    } else if (tuningFrequencyHz > 0 && tuningFrequencyHz < kStandardTuningHz) {
        // Lower than 440Hz gets arrow down
        tuningSymbol = kTuningSymbolLow;
        symbolColor = QColor(100, 149, 237); // Cornflower blue
    } else if (tuningFrequencyHz > kStandardTuningHz) {
        // Higher than 440Hz gets arrow up
        tuningSymbol = kTuningSymbolHigh;
        symbolColor = QColor(255, 99, 71); // Tomato red
    }

    // Reserve space for tuning symbol if needed
    int rightMargin = tuningSymbol ? kTuningSymbolWidth : 0;

    // Display the key text with the user-provided notation
    QString elidedText = option.fontMetrics.elidedText(
            keyText,
            Qt::ElideRight,
            columnWidth(index) - rectWidth - rightMargin);

    // This is not picking up the 'missing' or 'played' text color via
    // ForegroundRole from BaseTrackTableModel::data().
    // Set the palette colors manually and select the appropriate one.
    QStyleOptionViewItem opt = option;
    setTextColor(opt, index);
    if (opt.state & QStyle::State_Selected) {
        painter->setPen(QPen(opt.palette.highlightedText().color()));
    } else {
        painter->setPen(QPen(opt.palette.text().color()));
    }

    painter->drawText(option.rect.x() + rectWidth,
            option.rect.y(),
            option.rect.width() - rectWidth - rightMargin,
            option.rect.height(),
            Qt::AlignVCenter,
            elidedText);

    // Draw tuning indicator symbol
    if (tuningSymbol) {
        painter->save();
        if (option.state & QStyle::State_Selected) {
            // Use a brighter color when selected
            symbolColor = symbolColor.lighter(130);
        }
        painter->setPen(symbolColor);
        QFont symbolFont = option.font;
        symbolFont.setBold(true);
        painter->setFont(symbolFont);
        painter->drawText(
                option.rect.x() + option.rect.width() - kTuningSymbolWidth,
                option.rect.y(),
                kTuningSymbolWidth,
                option.rect.height(),
                Qt::AlignVCenter | Qt::AlignRight,
                QString::fromUtf8(tuningSymbol));
        painter->restore();
    }

    // Draw a border if the key cell has focus
    if (option.state & QStyle::State_HasFocus) {
        drawBorder(painter, m_focusBorderColor, option.rect);
    }
}
