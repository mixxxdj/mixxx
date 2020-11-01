#include "util/widgethelper.h"

#include <QScreen>

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
    const auto screenSize = pScreen->size();
    // math_clamp() cannot be used, because if the dimensions of
    // the popup menu are greater than the screen size a debug
    // assertion would be triggered!
    const auto adjustedX = math_max(0,
            math_min(
                    popupUpperLeft.x(),
                    screenSize.width() - popupSize.width()));
    const auto adjustedY = math_max(0,
            math_min(
                    popupUpperLeft.y(),
                    screenSize.height() - popupSize.height()));
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

} // namespace widgethelper

} // namespace mixxx
