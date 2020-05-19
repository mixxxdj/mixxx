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
    const auto adjustedX = math_clamp(
            popupUpperLeft.x(),
            /*min*/ 0,
            /*max*/ screenSize.width() - popupSize.width());
    const auto adjustedY = math_clamp(
            popupUpperLeft.y(),
            /*min*/ 0,
            /*max*/ screenSize.height() - popupSize.height());
    return QPoint(adjustedX, adjustedY);
}

} // namespace widgethelper

} // namespace mixxx
