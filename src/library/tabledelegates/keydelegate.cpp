#include "library/tabledelegates/keydelegate.h"

#include <cmath>
#include <QPainter>
#include <QStyle>
#include <QTableView>

#include "library/trackmodel.h"
#include "moc_keydelegate.cpp"

namespace {
// Unicode symbols for tuning indicators
const QString kTuningSymbol432Hz = QStringLiteral("✧"); // ✧ (sparkle) for 432Hz
const QString kTuningSymbolLow = QStringLiteral("↓");   // ↓ (arrow down) for <440Hz
const QString kTuningSymbolHigh = QStringLiteral("↑");  // ↑ (arrow up) for >440Hz
constexpr int kTuningSymbolWidth = 14;
constexpr double kStandardTuningHz = 440.0;
constexpr double k432Hz = 432.0;
constexpr double k432HzTolerance = 0.5; // Allow 0.5 Hz tolerance for 432Hz detection
} // namespace

void KeyDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    paintItemBackground(painter, option, index);

    const QString keyText = index.data().value<QString>();
    const QColor keyColor = index.data(Qt::DecorationRole).value<QColor>();
    const double tuningFrequencyHz = index.data(TrackModel::kTuningFrequencyRole).toDouble();
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
    QString tuningSymbol;
    QColor symbolColor;
    if (tuningFrequencyHz > 0.0 && std::abs(tuningFrequencyHz - k432Hz) < k432HzTolerance) {
        // 432Hz (with tolerance) gets the sparkle symbol
        tuningSymbol = kTuningSymbol432Hz;
        symbolColor = QColor(218, 165, 32); // Golden color
    } else if (tuningFrequencyHz > 0.0 && tuningFrequencyHz < kStandardTuningHz) {
        // Lower than 440Hz gets arrow down
        tuningSymbol = kTuningSymbolLow;
        symbolColor = QColor(100, 149, 237); // Cornflower blue
    } else if (tuningFrequencyHz > kStandardTuningHz) {
        // Higher than 440Hz gets arrow up
        tuningSymbol = kTuningSymbolHigh;
        symbolColor = QColor(255, 99, 71); // Tomato red
    }

    // Reserve space for tuning symbol if needed
    int rightMargin = !tuningSymbol.isEmpty() ? kTuningSymbolWidth : 0;

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

    // Draw tuning indicator symbol
    if (!tuningSymbol.isEmpty()) {
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
                tuningSymbol);
        painter->restore();
    }

    // Draw a border if the key cell has focus
    if (option.state & QStyle::State_HasFocus) {
        drawBorder(painter, m_focusBorderColor, option.rect);
    }
}
