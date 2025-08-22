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
QWindow* getWindow(
        const QWidget& widget);

/// QSize for stretching a list widget attempting to show entire column
void growListWidget(QListWidget& listWidget, const QWidget& parent);

// Get the base color of a widget, or recursively search the parent tree for one.
//
// Returns QColor(0,0,0) when none is found. As the recursion can go quite deep,
// avoid calling repeatedly, but only when needed, e.g in a showEvent.
QColor findBaseColor(QWidget* pWidget);

} // namespace widgethelper

} // namespace mixxx
