#include "util/widgethelper.h"

#include <QScreen>
#include <QWindow>

#include "util/math.h"

namespace mixxx {

namespace widgethelper {

QPoint mapPopupToScreen(
        const QWidget& widget,
        const QPoint& popupUpperLeft,
        const QSize& popupSize) {
    const auto screenSize = widget.windowHandle()->screen()->size();
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

} // namespace widgethelper

} // namespace mixxx
