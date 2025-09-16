#include "util/widgethelper.h"

#include <QApplication>
#include <QMainWindow>
#include <QScreen>
#include <QStyle>

#include "util/math.h"

namespace mixxx {

namespace widgethelper {

QPoint mapPopupToScreen(
        const QWidget& widget,
        const QPoint& popupUpperLeft,
        const QSize& popupSize) {
    const auto* const pScreen = getScreen(widget);
    VERIFY_OR_DEBUG_ASSERT(pScreen) {
        // This should never fail
        return popupUpperLeft;
    }

    // the screen geometry is the physical screen of the virtual desktop
    // this will be offset by it's top and left starting points
    const auto screenSize = pScreen->geometry();

    // math_clamp() cannot be used, because if the dimensions of
    // the popup menu are greater than the screen size a debug
    // assertion would be triggered!
    const auto adjustedX = math_max(0,
            math_min(
                    popupUpperLeft.x(),
                    screenSize.right() - popupSize.width()));
    const auto adjustedY = math_max(0,
            math_min(
                    popupUpperLeft.y(),
                    screenSize.bottom() - popupSize.height()));
    return QPoint(adjustedX, adjustedY);
}

QWindow* getWindow(
        const QWidget& widget) {
    if (auto* window = widget.windowHandle()) {
        return window;
    }
    if (auto* nativeParent = widget.nativeParentWidget()) {
        return nativeParent->windowHandle();
    }
    return nullptr;
}

QScreen* getMainScreen() {
    QMainWindow* pMainWindow = nullptr;
    const QWidgetList pwidgets = QApplication::topLevelWidgets();
    for (auto* pWidget : pwidgets) {
        if ((pMainWindow = qobject_cast<QMainWindow*>(pWidget))) {
            return pMainWindow->screen();
        }
    }
    return nullptr;
}

void growListWidget(QListWidget& listWidget, const QWidget& parent) {
    // Try to display all files and the complete file locations to avoid
    // horizontal scrolling.
    // Get the screen dimensions
    QScreen* const pScreen = getScreen(parent);
    QSize screenSpace;
    VERIFY_OR_DEBUG_ASSERT(pScreen) {
        qWarning() << "Screen not detected. Assuming screen size of 800x600px.";
        screenSpace = QSize(800, 600);
    }
    else {
        screenSpace = pScreen->size();
    }
    // Calculate the dimensions of the file list to show all.
    int margin = 2 * listWidget.frameWidth() +
            listWidget.style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    int minW = listWidget.sizeHintForColumn(0) + margin;
    int minH = listWidget.sizeHintForRow(0) * listWidget.count() + margin;
    // The file list should fit into the window, but clamp to 75% of screen size
    // so (hoepfully) the entire containing dialog is visible even if there are
    // toolbars at the screen edges
    int newW = std::min(minW, static_cast<int>(screenSpace.width() * 0.75));
    int newH = std::min(minH, static_cast<int>(screenSpace.height() * 0.75));
    // Apply new size
    if (newW > 0 && newH > 0) {
        listWidget.setMinimumSize(newW, newH);
    }
}

// get the base color of a widget, or recursively search the parent tree for one
QColor findBaseColor(QWidget* pWidget) {
    while (pWidget) {
        if (pWidget->palette().isBrushSet(QPalette::Normal, QPalette::Base)) {
            return pWidget->palette().color(QPalette::Base);
        }
        pWidget = qobject_cast<QWidget*>(pWidget->parent());
    }
    return QColor(0, 0, 0);
}

} // namespace widgethelper

} // namespace mixxx
