#include "library/tabledelegates/cmrtdelegate.h"

#include <QPainter>
#include <QStyle>
#include <QTableView>

#include "library/trackmodel.h"
#include "moc_cmrtdelegate.cpp"

namespace {
const QColor kCanonicalColor(0x1E, 0x88, 0xE5);
const QColor kUnscoredMemberColor(Qt::black);
constexpr int kSwatchWidth = 4;

QColor matchScoreColor(double matchScore) {
    const double clamped = qBound(0.0, matchScore, 1.0);

    // Colour components we'll interpolate
    int hue, value;
    constexpr int sat = 200;

    if (clamped <= 0.5) {
        // 0.0 → deep red (value=100), 0.5 → bright red (value=220)
        hue = 0;
        constexpr int vLow = 100, vHigh = 220;
        value = vLow + static_cast<int>((vHigh - vLow) * (clamped / 0.5));
    } else if (clamped <= 0.75) {
        // 0.5 → red (hue=0), 0.75 → yellow (hue=60)
        value = 220;
        double t = (clamped - 0.5) / 0.25;
        hue = static_cast<int>(t * 60.0);
    } else {
        // 0.75 → yellow (hue=60), 1.0 → green (hue=120)
        value = 220;
        double t = (clamped - 0.75) / 0.25;
        hue = 60 + static_cast<int>(t * 60.0);
    }

    return QColor::fromHsv(hue, sat, value);
}
} // namespace

void CmrtDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    paintItemBackground(painter, option, index);

    const QString cmrtName = index.data().toString();

    QStyleOptionViewItem opt = option;
    setTextColor(opt, index);
    if (opt.state & QStyle::State_Selected) {
        painter->setPen(QPen(opt.palette.highlightedText().color()));
    } else {
        painter->setPen(QPen(opt.palette.text().color()));
    }

    const QVariant decorationVariant = index.data(Qt::DecorationRole);
    if (!decorationVariant.isValid()) {
        // No CMRT group, or group has exactly one member — library_view
        // returns NULL for cmrt_track_name in both cases (see the
        // "cg.track_count > 1" join condition), so there's nothing to draw
        // beyond the placeholder.
        painter->drawText(option.rect,
                Qt::AlignVCenter,
                cmrtName.isEmpty() ? QStringLiteral("--") : cmrtName);
        if (option.state & QStyle::State_HasFocus) {
            drawBorder(painter, m_focusBorderColor, option.rect);
        }
        return;
    }

    const QVariantMap decoration = decorationVariant.value<QVariantMap>();
    const bool isCanonical = decoration.value("isCanonical").toBool();

    const int x = option.rect.x();
    constexpr int yPad = 2;
    QColor swatchColor;
    if (isCanonical) {
        swatchColor = kCanonicalColor;
    } else {
        const QVariant matchScoreValue = decoration.value("matchScore");
        swatchColor = matchScoreValue.isNull()
                ? kUnscoredMemberColor
                : matchScoreColor(matchScoreValue.toDouble());
    }
    painter->fillRect(x,
            option.rect.y() + yPad,
            kSwatchWidth,
            option.rect.height() - 2 * yPad,
            swatchColor);

    const QString elidedText = option.fontMetrics.elidedText(
            cmrtName,
            Qt::ElideRight,
            columnWidth(index) - kSwatchWidth - 4);

    painter->drawText(option.rect.x() + kSwatchWidth + 4,
            option.rect.y(),
            option.rect.width() - kSwatchWidth - 4,
            option.rect.height(),
            Qt::AlignVCenter,
            elidedText);

    if (option.state & QStyle::State_HasFocus) {
        drawBorder(painter, m_focusBorderColor, option.rect);
    }
}
