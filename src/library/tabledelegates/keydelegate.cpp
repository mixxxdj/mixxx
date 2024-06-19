#include "library/tabledelegates/keydelegate.h"

#include <qnamespace.h>

#include <QPainter>
#include <QStyle>
#include <QTableView>

#include "moc_keydelegate.cpp"
#include "track/keyutils.h"
#include "util/color/predefinedcolorpalettes.h"
#include "util/color/rgbcolor.h"

KeyDelegate::KeyDelegate(QTableView* pTableView)
        : TableItemDelegate(pTableView) {
}

void KeyDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    paintItemBackground(painter, option, index);

    QString keyText = index.data().toString();
    mixxx::track::io::key::ChromaticKey key = KeyUtils::guessKeyFromText(keyText);
    int openKeyNumber = KeyUtils::keyToOpenKeyNumber(key);

    if (openKeyNumber != 0) {
        const auto rgbColor =
                mixxx::PredefinedColorPalettes::kDefaultKeyColorPalette.at(
                        openKeyNumber - 1); // Open Key numbers start from 1
        const auto qcolor = mixxx::RgbColor::toQColor(rgbColor);

        // Draw the colored rectangle next to the key label
        painter->fillRect(
                option.rect.x() + 2,
                option.rect.y() + 2,
                4, // width
                option.rect.height() - 4,
                qcolor);
    }

    QString elidedText = option.fontMetrics.elidedText(
            index.data().toString(),
            Qt::ElideLeft,
            columnWidth(index) - 8); // Subtracting total width of the rectangle

    painter->drawText(option.rect.x() + 8,
            option.rect.y(),
            option.rect.width() - 8,
            option.rect.height(),
            Qt::AlignVCenter,
            elidedText);

    // Draw a border if the key cell has focus
    if (option.state & QStyle::State_HasFocus) {
        drawBorder(painter, m_focusBorderColor, option.rect);
    }
}
