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
    QString keyText = index.data().toString();
    mixxx::track::io::key::ChromaticKey key = KeyUtils::guessKeyFromText(keyText);
    int openKeyNumber = KeyUtils::keyToOpenKeyNumber(key);

    if (openKeyNumber != 0) {
        // Open Key numbers start from 1
        const auto color =
                mixxx::PredefinedColorPalettes::kDefaultKeyColorPalette.at(
                        openKeyNumber - 1);

        // this code is from the ColorDelegate, using it temporarily for testing.
        if (color) {
            painter->fillRect(option.rect, mixxx::RgbColor::toQColor(color));
        } else {
            // Filter out track color that is hidden
            if (option.state & QStyle::State_Selected) {
                painter->fillRect(option.rect, option.palette.highlight());
            }
        }

        // Draw a border if the color cell has focus
        if (option.state & QStyle::State_HasFocus) {
            drawBorder(painter, m_focusBorderColor, option.rect);
        }
    }

    // QStyledItemDelegate::paint(painter, option, index);
}
