#include "library/tabledelegates/keydelegate.h"

#include <QFontMetrics>
#include <QPainter>
#include <QStyle>
#include <QTableView>
#include <algorithm>

#include "library/keyhighlightmanager.h"
#include "library/trackmodel.h"
#include "moc_keydelegate.cpp"

namespace {
// Unicode symbols for tuning indicators
const QString kTuningSymbol432Hz = QStringLiteral("\u2727"); // ✧ (sparkle) for 432Hz
const QString kTuningSymbolLow = QStringLiteral("\u2193");   // ↓ (arrow down) for <440Hz
const QString kTuningSymbolHigh = QStringLiteral("\u2191");  // ↑ (arrow up) for >440Hz
constexpr int kTuningSymbolWidth = 14;
constexpr double kStandardTuningHz = 440.0;
constexpr double k432Hz = 432.0;
constexpr double kTuningToleranceHz = 2.5; // 2.5 Hz equals roughly 10 cents for these frequencies
constexpr double kStandardTuningLowHz = kStandardTuningHz - kTuningToleranceHz;
constexpr double kStandardTuningHighHz = kStandardTuningHz + kTuningToleranceHz;
constexpr double k432LowHz = k432Hz - kTuningToleranceHz;
constexpr double k432HighHz = k432Hz + kTuningToleranceHz;

// The key highlighter's hint how many semitones to pitch the track to make it
// compatible with the highlighting deck. Drawn left of any tuning symbol.
const QString kKeyShiftUp = QStringLiteral("+1");
// \u takes exactly four hex digits: U+2212 MINUS SIGN and U+00B1 PLUS-MINUS SIGN.
const QString kKeyShiftDown = QStringLiteral("\u22121");   // −1
const QString kKeyShiftEither = QStringLiteral("\u00B11"); // ±1
constexpr int kKeyShiftPadding = 4;
} // namespace

void KeyDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    paintItemBackground(painter, option, index);
    // The harmonic key highlighter's tint. Besides surviving row selection, it
    // signals that the text colour must come from the model's contrasting
    // ForegroundRole rather than the skin default (further down).
    const QColor highlightBg = paintHighlightOverSelection(painter, option, index);

    QString keyText = index.data().value<QString>();
    // On a pitched reference deck's track, e.g. "8A (9A)".
    const QString playingKeyText = index.data(TrackModel::kPlayingKeyRole).toString();
    if (!playingKeyText.isEmpty()) {
        keyText = QStringLiteral("%1 (%2)").arg(keyText, playingKeyText);
    }
    const QVariantMap colorRect = index.data(Qt::DecorationRole).value<QVariantMap>();
    const double tuningFrequencyHz = index.data(TrackModel::kTuningFrequencyRole).toDouble();
    const auto keyMatch = static_cast<mixxx::KeyHighlightManager::KeyMatch>(
            index.data(TrackModel::kKeyMatchRole).toInt());
    int leftMargin = 0;

    const QColor colorTop = colorRect["top"].value<QColor>();
    const double splitPoint = colorRect["splitPoint"].value<double>();

    if (colorTop.isValid()) {
        // Draw the colored rectangle next to the key label
        constexpr int width = 4;
        leftMargin = width + 4; // 4px right padding

        const int x = option.rect.x();
        constexpr int yPad = 2;
        const int padTop = option.rect.y() + yPad;
        const int padHeight = option.rect.height() - 2 * yPad;
        // adding 0.5 to get the round int instead of floor int
        const int splitHeight = static_cast<int>(padHeight * splitPoint + 0.5);

        painter->fillRect(
                x,
                padTop,
                width,
                splitHeight,
                colorTop);

        // if this track has a tuning, draw the second color
        if (splitPoint < 1) {
            const QColor colorBottom = colorRect["bottom"].value<QColor>();
            painter->fillRect(
                    x,
                    padTop + splitHeight,
                    width,
                    padHeight - splitHeight,
                    colorBottom);
        }
    }

    // Determine which tuning symbol to show (if any)
    QString tuningSymbol;
    QColor symbolColor;
    if (tuningFrequencyHz >= k432LowHz && tuningFrequencyHz <= k432HighHz) {
        // 432Hz (with tolerance) gets the sparkle symbol
        tuningSymbol = kTuningSymbol432Hz;
        symbolColor = QColor(218, 165, 32); // Golden color
    } else if (tuningFrequencyHz > 0.0 && tuningFrequencyHz < kStandardTuningLowHz) {
        // Lower than 440Hz gets arrow down
        tuningSymbol = kTuningSymbolLow;
        symbolColor = QColor(100, 149, 237); // Cornflower blue
    } else if (tuningFrequencyHz > kStandardTuningHighHz) {
        // Higher than 440Hz gets arrow up
        tuningSymbol = kTuningSymbolHigh;
        symbolColor = QColor(255, 99, 71); // Tomato red
    }

    QString keyShift;
    switch (keyMatch) {
    case mixxx::KeyHighlightManager::KeyMatch::ShiftUp:
        keyShift = kKeyShiftUp;
        break;
    case mixxx::KeyHighlightManager::KeyMatch::ShiftDown:
        keyShift = kKeyShiftDown;
        break;
    case mixxx::KeyHighlightManager::KeyMatch::ShiftEither:
        keyShift = kKeyShiftEither;
        break;
    case mixxx::KeyHighlightManager::KeyMatch::None:
    case mixxx::KeyHighlightManager::KeyMatch::Perfect:
    case mixxx::KeyHighlightManager::KeyMatch::Neighbour:
        break;
    }
    QFont keyShiftFont = option.font;
    keyShiftFont.setBold(true);

    // Reserve space for the tuning symbol (far right) and, left of it, the
    // shift hint if present.
    const int tuningMargin = !tuningSymbol.isEmpty() ? kTuningSymbolWidth : 0;
    const int keyShiftWidth = !keyShift.isEmpty()
            ? QFontMetrics(keyShiftFont).horizontalAdvance(keyShift) +
                    kKeyShiftPadding
            : 0;
    const int rightMargin = tuningMargin + keyShiftWidth;

    // Display the key text with the user-provided notation. Clamp the available
    // width to >= 0: with both a swatch and two right-edge glyphs a narrow Key
    // column could otherwise pass a negative width to elidedText.
    const int textWidth =
            std::max(0, columnWidth(index) - leftMargin - rightMargin);
    QString elidedText = option.fontMetrics.elidedText(
            keyText,
            Qt::ElideRight,
            textWidth);

    // Set the palette colours manually and select the appropriate one.
    QStyleOptionViewItem opt = option;
    setTextColor(opt, index);
    // The colour the key text is drawn in. The shift hint below reuses it, so
    // it is as legible as the key on any highlight background.
    QColor textColor = (opt.state & QStyle::State_Selected)
            ? opt.palette.highlightedText().color()
            : opt.palette.text().color();
    // On a cell the highlighter coloured, draw the key in the model's
    // contrasting ForegroundRole, selected or not: setTextColor() blends it
    // 50/50 with the skin's selected text colour, which can wash it out
    // against the highlight that stays under selection. Other cells keep the
    // colours chosen above.
    if (highlightBg.isValid()) {
        const auto fgData = index.data(Qt::ForegroundRole);
        if (fgData.canConvert<QColor>()) {
            const QColor fgColor = fgData.value<QColor>();
            if (fgColor.isValid()) {
                textColor = fgColor;
            }
        }
    }
    painter->setPen(QPen(textColor));

    painter->drawText(option.rect.x() + leftMargin,
            option.rect.y(),
            textWidth,
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

    // Draw the shift hint left of the tuning symbol, which stays anchored at
    // the far-right edge.
    if (!keyShift.isEmpty()) {
        painter->save();
        painter->setPen(textColor);
        painter->setFont(keyShiftFont);
        painter->drawText(
                option.rect.x() + option.rect.width() - rightMargin,
                option.rect.y(),
                keyShiftWidth,
                option.rect.height(),
                Qt::AlignVCenter | Qt::AlignRight,
                keyShift);
        painter->restore();
    }

    // Draw a border if the key cell has focus
    if (option.state & QStyle::State_HasFocus) {
        drawBorder(painter, m_focusBorderColor, option.rect);
    }
}
