#include "library/tabledelegates/keydelegate.h"

#include <QPainter>
#include <QStyle>
#include <QTableView>

#include "moc_keydelegate.cpp"
#include "track/keyutils.h"

void KeyDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    paintItemBackground(painter, option, index);

    mixxx::track::io::key::ChromaticKey key =
            index.data().value<mixxx::track::io::key::ChromaticKey>();
    if (key != mixxx::track::io::key::INVALID) {
        // Display the key colors if enabled
        bool keyColorsEnabled = index.data(Qt::UserRole).value<bool>();
        if (keyColorsEnabled) {
            const QColor keyColor = KeyUtils::keyToColor(key);
            if (keyColor.isValid()) {
                // Draw the colored rectangle next to the key label
                painter->fillRect(
                        option.rect.x(),
                        option.rect.y() + 2,
                        4, // width
                        option.rect.height() - 4,
                        keyColor);
            }
        }

        // Display the key text with the user-provided notation
        int rectWidth = keyColorsEnabled ? 8 : 0; //  4px width + 4px right padding
        const QString keyText = KeyUtils::keyToString(key);
        QString elidedText = option.fontMetrics.elidedText(
                keyText,
                Qt::ElideLeft,
                columnWidth(index) - rectWidth);

        if (option.state & QStyle::State_Selected) {
            // This uses selection-color from stylesheet for the text pen:
            // #LibraryContainer QTableView {
            //   selection-color: #fff;
            // }
            painter->setPen(QPen(option.palette.highlightedText().color()));
        }

        painter->drawText(option.rect.x() + rectWidth,
                option.rect.y(),
                option.rect.width() - rectWidth,
                option.rect.height(),
                Qt::AlignVCenter,
                elidedText);
    }

    // Draw a border if the key cell has focus
    if (option.state & QStyle::State_HasFocus) {
        drawBorder(painter, m_focusBorderColor, option.rect);
    }
}
