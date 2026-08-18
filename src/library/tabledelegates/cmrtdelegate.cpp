#include "library/tabledelegates/cmrtdelegate.h"

#include <QPainter>
#include <QStyle>
#include <QTableView>

#include "library/trackmodel.h"
#include "moc_cmrtdelegate.cpp"

namespace {
// Matches the existing Mixxx hotcue "Green" (0x32BE44, see schema.xml
// revision 32 / colorpalette.h) so the canonical-track swatch reads as
// consistent with the rest of the app's color language instead of
// introducing a brand-new green.
const QColor kCanonicalColor(0x32, 0xBE, 0x44);
// A clearly-distinct, non-alarming color for "this is a member, not the
// canonical file" — not red, since being a member isn't an error state.
const QColor kMemberColor(0xE6, 0x96, 0x28);
constexpr int kSwatchWidth = 4;
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

    if (cmrtName.isEmpty()) {
        // No CMRT group, or group has exactly one member — library_view
        // returns NULL for cmrt_track_name in both cases (see the
        // "cg.track_count > 1" join condition), so there's nothing to draw
        // beyond the placeholder.
        painter->drawText(option.rect, Qt::AlignVCenter, QStringLiteral("--"));
        if (option.state & QStyle::State_HasFocus) {
            drawBorder(painter, m_focusBorderColor, option.rect);
        }
        return;
    }

    const bool isCanonical = index.data(Qt::DecorationRole)
                                     .value<QVariantMap>()
                                     .value("isCanonical")
                                     .toBool();

    const int x = option.rect.x();
    constexpr int yPad = 2;
    painter->fillRect(x,
            option.rect.y() + yPad,
            kSwatchWidth,
            option.rect.height() - 2 * yPad,
            isCanonical ? kCanonicalColor : kMemberColor);

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
