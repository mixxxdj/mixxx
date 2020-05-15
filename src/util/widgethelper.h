#pragma once

#include <QScreen>

namespace mixxx {
namespace widgethelper {
/// Returns an adjusted upper left point for displaying the popup
/// with the given size on the screen, shifting the popup if it would go off
/// the right or bottom edges of the screen.
QPoint mapPopupToScreen(
        const QSize& screenSize,
        const QPoint& popupUpperLeft,
        const QSize& popupSize) {
    QPoint newTopLeft = popupUpperLeft;
    if (popupUpperLeft.x() + popupSize.width() > screenSize.width()) {
        newTopLeft.setX(screenSize.width() - popupSize.width());
    }
    if (popupUpperLeft.y() + popupSize.height() > screenSize.height()) {
        newTopLeft.setY(screenSize.height() - popupSize.height());
    }
    return newTopLeft;
}
} // namespace widgethelper
} // namespace mixxx
