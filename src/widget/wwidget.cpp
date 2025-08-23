#include "widget/wwidget.h"

#include <QTouchEvent>

#include "control/controlproxy.h"
#include "moc_wwidget.cpp"
#include "util/assert.h"

WWidget::WWidget(QWidget* parent, Qt::WindowFlags flags)
        : QWidget(parent, flags),
          WBaseWidget(this),
          m_activeTouchButton(Qt::NoButton),
          m_scaleFactor(1.0) {
    m_pTouchShift = new ControlProxy("[Controls]", "touch_shift");
    setAttribute(Qt::WA_StaticContents);
    // Touch events are disabled on macOS to work around the issue that Mac
    // trackpad events are not processed correctly with Qt 6. Since these events
    // aren't needed anyway, we can safely disable them. Once upstream (Qt)
    // fixes the issue, the `#ifndef` can be removed to re-enable touch events.
    // For details on both the issue and the fix, see
    // - https://bugreports.qt.io/browse/QTBUG-103935?focusedId=739905#comment-739905
    // - https://github.com/mixxxdj/mixxx/issues/11869
    // - https://github.com/mixxxdj/mixxx/pull/11870
#ifndef __APPLE__
    setAttribute(Qt::WA_AcceptTouchEvents);
#endif
    setFocusPolicy(Qt::ClickFocus);
}

WWidget::~WWidget() {
    delete m_pTouchShift;
}

bool WWidget::touchIsRightButton() {
    return m_pTouchShift->toBool();
}

bool WWidget::event(QEvent* e) {
    if (e->type() == QEvent::ToolTip) {
        updateTooltip();
    } if (e->type() == QEvent::FontChange) {
        const QFont& fonti = font();
        // Change the new font on the fly by casting away its constancy
        // using setFont() here, would results into a recursive loop
        // resetting the font to the original css values.
        // Only scale pixel size fonts, point size fonts are scaled by the OS
        if (fonti.pixelSize() > 0) {
            const_cast<QFont&>(fonti).setPixelSize(
                    static_cast<int>(fonti.pixelSize() * m_scaleFactor));
        }
    } else if (isEnabled()) {
        switch(e->type()) {
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
        case QEvent::TouchEnd: {
            QTouchEvent* touchEvent = dynamic_cast<QTouchEvent*>(e);
            if (touchEvent == nullptr) {
                break;
            }

            // fake a mouse event!
            QEvent::Type eventType = QEvent::None;
            switch (touchEvent->type()) {
            case QEvent::TouchBegin:
                eventType = QEvent::MouseButtonPress;
                if (touchIsRightButton()) {
                    // touch is right click
                    m_activeTouchButton = Qt::RightButton;
                } else {
                    m_activeTouchButton = Qt::LeftButton;
                }
                break;
            case QEvent::TouchUpdate:
                eventType = QEvent::MouseMove;
                break;
            case QEvent::TouchEnd:
                eventType = QEvent::MouseButtonRelease;
                break;
            default:
                DEBUG_ASSERT(false);
                break;
            }

            const QTouchEvent::TouchPoint& touchPoint =
                    touchEvent->points()
                            .first();
            QMouseEvent mouseEvent(eventType,
                    touchPoint.position(),
                    touchPoint.position(),
                    touchPoint.globalPosition(),
                    m_activeTouchButton, // Button that causes the event
                    Qt::NoButton,        // Not used, so no need to fake a proper value.
                    touchEvent->modifiers(),
                    Qt::MouseEventSynthesizedByApplication);

            return QWidget::event(&mouseEvent);
        }
        default:
            break;
        }
    }

    return QWidget::event(e);
}
