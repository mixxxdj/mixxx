#pragma once

#include <QListWidget>
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
QWindow* getWindow(const QWidget& widget);

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
        return getScreen(*qobject_cast<QWidget*>(widget.parent()));
    }
    if (!pWindow) {
        // This might happen if the widget is not yet displayed
        return nullptr;
    }
    return pWindow->screen();
#endif
}

/// Name for the central skin widget. Set by LegacySkinparser.
inline QString skinWidgetName() {
    return QStringLiteral("Skin");
}

/// Try to get the QMainWindow's central 'Skin' widget.
/// This is a helper for various GUI or non-GUI classes to use 'Skin' as parent
/// for their dialogs so they inherit the skin's stylesheet.
QWidget* getSkinWidget();

/// QSize for stretching a list widget attempting to show entire column
void growListWidget(QListWidget& listWidget, const QWidget& parent);

// Get the base color of a widget, or recursively search the parent tree for one.
//
// Returns QColor(0,0,0) when none is found. As the recursion can go quite deep,
// avoid calling repeatedly, but only when needed, e.g in a showEvent.
QColor findBaseColor(QWidget* pWidget);

} // namespace widgethelper

} // namespace mixxx
