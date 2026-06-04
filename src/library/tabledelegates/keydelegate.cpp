#include "library/tabledelegates/keydelegate.h"

#include <QPainter>
#include <QStyle>
#include <QTableView>
#include <algorithm>

#include "library/trackmodel.h"
#include "moc_keydelegate.cpp"
#include "track/keyutils.h"

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

// Harmonic key-highlight transpose-direction arrows, shown on Yellow Key cells:
// the track must be pitched up (+1), down (-1), or either way by one semitone to
// become mixable with the highlighted deck. Drawn just left of any tuning
// symbol. The glyphs mirror KeyUtils::YellowShift (None/Up/Down/Both).
const QString kShiftArrowUp = QStringLiteral("↑");   // up arrow, pitch +1
const QString kShiftArrowDown = QStringLiteral("↓"); // down arrow, pitch -1
const QString kShiftArrowBoth = QStringLiteral("↕"); // up-down arrow, either
constexpr int kShiftArrowWidth = 14;
} // namespace

void KeyDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    paintItemBackground(painter, option, index);

    const QString keyText = index.data().value<QString>();
    const QVariantMap colorRect = index.data(Qt::DecorationRole).value<QVariantMap>();
    const double tuningFrequencyHz = index.data(TrackModel::kTuningFrequencyRole).toDouble();
    const auto shiftDirection = static_cast<KeyUtils::YellowShift>(
            index.data(TrackModel::kKeyShiftDirectionRole).toInt());
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

    // Pick the harmonic transpose-direction arrow (Yellow cells only). It sits
    // just left of the tuning symbol, so reserve space for both independently.
    QString shiftArrow;
    switch (shiftDirection) {
    case KeyUtils::YellowShift::Up:
        shiftArrow = kShiftArrowUp;
        break;
    case KeyUtils::YellowShift::Down:
        shiftArrow = kShiftArrowDown;
        break;
    case KeyUtils::YellowShift::Both:
        shiftArrow = kShiftArrowBoth;
        break;
    case KeyUtils::YellowShift::None:
        break;
    }

    // Reserve space for the tuning symbol (far right) and, inside it, the
    // harmonic arrow if present.
    const int tuningMargin = !tuningSymbol.isEmpty() ? kTuningSymbolWidth : 0;
    const int shiftMargin = !shiftArrow.isEmpty() ? kShiftArrowWidth : 0;
    int rightMargin = tuningMargin + shiftMargin;

    // Display the key text with the user-provided notation. Clamp the available
    // width to >= 0: with both a swatch and two right-edge glyphs a narrow Key
    // column could otherwise pass a negative width to elidedText.
    const int textWidth =
            std::max(0, columnWidth(index) - leftMargin - rightMargin);
    QString elidedText = option.fontMetrics.elidedText(
            keyText,
            Qt::ElideRight,
            textWidth);

    // This is not picking up the 'missing' or 'played' text color via
    // ForegroundRole from BaseTrackTableModel::data().
    // Set the palette colors manually and select the appropriate one.
    QStyleOptionViewItem opt = option;
    setTextColor(opt, index);
    // The colour the key glyph is drawn in. Reused for the direction arrow below
    // so the arrow is always exactly as legible as the key text beside it, on
    // any background the highlighter produces - a fixed colour could otherwise
    // vanish (e.g. a dark arrow on the played dark-blue cell).
    const QColor textColor = (opt.state & QStyle::State_Selected)
            ? opt.palette.highlightedText().color()
            : opt.palette.text().color();
    painter->setPen(QPen(textColor));

    painter->drawText(option.rect.x() + leftMargin,
            option.rect.y(),
            option.rect.width() - leftMargin - rightMargin,
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

    // Draw the harmonic transpose-direction arrow just left of the tuning
    // symbol (the tuning symbol stays anchored at the far-right edge). It uses
    // the same colour as the key text, so it stays legible against whatever
    // background the highlighter painted for this cell.
    if (!shiftArrow.isEmpty()) {
        painter->save();
        painter->setPen(textColor);
        QFont arrowFont = option.font;
        arrowFont.setBold(true);
        painter->setFont(arrowFont);
        painter->drawText(
                option.rect.x() + option.rect.width() - tuningMargin -
                        kShiftArrowWidth,
                option.rect.y(),
                kShiftArrowWidth,
                option.rect.height(),
                Qt::AlignVCenter | Qt::AlignRight,
                shiftArrow);
        painter->restore();
    }

    // Draw a border if the key cell has focus
    if (option.state & QStyle::State_HasFocus) {
        drawBorder(painter, m_focusBorderColor, option.rect);
    }
}
