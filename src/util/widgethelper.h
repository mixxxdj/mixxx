#pragma once

#include <QWidget>
#include <QPoint>
#include <QSize>

namespace mixxx {

namespace widgethelper {

/// Returns an adjusted upper left point for displaying the popup
/// with the given size on the screen, shifting the popup if it would go off
/// the right or bottom edges of the screen.
QPoint mapPopupToScreen(
        const QWidget& widget,
        const QPoint& popupUpperLeft,
        const QSize& popupSize);

} // namespace widgethelper

} // namespace mixxx
