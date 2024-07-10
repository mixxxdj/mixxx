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
    // Get the key text with the user-provided notation
    bool keyColorsEnabled = index.data(Qt::UserRole).value<bool>();
    int rectWidth = 0; // only leave space for rect if it is rendered
    if (keyColorsEnabled) {
        const QColor keyColor = KeyUtils::keyToColor(key);
        rectWidth = 8; // 4px width + 4px right padding
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

    // Draw a border if the key cell has focus
    if (option.state & QStyle::State_HasFocus) {
        drawBorder(painter, m_focusBorderColor, option.rect);
    }
}
