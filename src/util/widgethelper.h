#pragma once

#include <QPoint>
#include <QSize>
#include <QWidget>
#include <QWindow>

#include "util/assert.h"

namespace mixxx {

namespace widgethelper {

/// Returns an adjusted upper left point for displaying the popup
/// with the given size on the screen, shifting the popup if it would go off
/// the right or bottom edges of the screen.
QPoint mapPopupToScreen(
        const QWidget& widget,
        const QPoint& popupUpperLeft,
        const QSize& popupSize);

/// Obtains the corresponding window for the given widget.
///
/// Might return nullptr if no window could be determined.
///
/// Adopted from windowForWidget() in qtbase/src/widgets/kernel/qapplication_p.h
QWindow* getWindow(
        const QWidget& widget);

/// Obtains the corresponding screen for the given widget.
///
/// Might return nullptr if no screen could be determined.
inline QScreen* getScreen(
        const QWidget& widget) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return widget.screen();
#else
    const auto* pWindow = getWindow(widget);
    if (!pWindow && widget.parent()) {
        // The window might still be hidden and invisible. Then we
        // try to obtain the screen from its parent recursively.
        return getScreen(*static_cast<QWidget*>(widget.parent()));
    }
    if (!pWindow) {
        // This might happen if the widget is not yet displayed
        return nullptr;
    }
    return pWindow->screen();
#endif
}

} // namespace widgethelper

} // namespace mixxx
