

#include <QtDebug>
#include <QTouchEvent>

#include "mixxxapplication.h"
#include "controlobjectslave.h"
#include "mixxx.h"

extern void qt_translateRawTouchEvent(QWidget *window,
        QTouchEvent::DeviceType deviceType,
        const QList<QTouchEvent::TouchPoint> &touchPoints);

MixxxApplication::MixxxApplication(int& argc, char** argv)
        : QApplication(argc, argv),
          m_fakeMouseSourcePointId(0),
          m_fakeMouseWidget(NULL),
          m_activeTouchButton(Qt::NoButton),
          m_pTouchShift(NULL) {
}

MixxxApplication::~MixxxApplication() {
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
bool MixxxApplication::notify(QObject* target, QEvent* event) {
    switch (event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    {
        QTouchEvent* touchEvent = static_cast<QTouchEvent*>(event);
        QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
        QEvent::Type eventType = QEvent::None;
        Qt::MouseButtons buttons = Qt::NoButton;
        QWidget* fakeMouseWidget = NULL;
        bool baseReturn;

        //qDebug() << "&" << touchEvent->type() << target;

        if (touchEvent->deviceType() !=  QTouchEvent::TouchScreen) {
            break;
        }

        switch (event->type()) {
        case QEvent::TouchBegin:
            // try to deliver as touch event
            baseReturn = QApplication::notify(target, event);
            if (dynamic_cast<MixxxMainWindow*>(touchEvent->widget())) {
                // the touchEvent has fallen trough to the MixxxMainWindow, because there
                // was no touch enabled widget found.
                // Now we resent this event and all following events for this touch point
                // as Mouse events.
                eventType = QEvent::MouseButtonPress;
                if (touchIsRightButton()) {
                    // touch is right click
                    m_activeTouchButton = Qt::RightButton;
                    buttons = Qt::RightButton;
                } else {
                    m_activeTouchButton = Qt::LeftButton;
                    buttons = Qt::LeftButton;
                }
                m_fakeMouseSourcePointId = touchPoints.first().id();
                m_fakeMouseWidget = dynamic_cast<QWidget*>(target);
                fakeMouseWidget = m_fakeMouseWidget;
                break;
            }
            return baseReturn;
        case QEvent::TouchUpdate:
            if (m_fakeMouseWidget) {
                eventType = QEvent::MouseMove;
                buttons = m_activeTouchButton;
                fakeMouseWidget = m_fakeMouseWidget;
                break;
            }
            return QApplication::notify(target, event);
        case QEvent::TouchEnd:
            if (m_fakeMouseWidget) {
                eventType = QEvent::MouseButtonRelease;
                m_fakeMouseSourcePointId = touchPoints.first().id();
                fakeMouseWidget = m_fakeMouseWidget;
                m_fakeMouseWidget = NULL;
                break;
            }
            return QApplication::notify(target, event);
        default:
            return QApplication::notify(target, event);
        }

        for (int i = 0; i < touchPoints.count(); ++i) {
             const QTouchEvent::TouchPoint& touchPoint = touchPoints.at(i);
             if (touchPoint.id() == m_fakeMouseSourcePointId) {
                 QMouseEvent mouseEvent(eventType,
                                        fakeMouseWidget->mapFromGlobal(touchPoint.screenPos().toPoint()),
                                        touchPoint.screenPos().toPoint(),
                                        m_activeTouchButton, // Button that causes the event
                                        buttons,
                                        touchEvent->modifiers());

                 //qDebug() << "#" << mouseEvent.type() << mouseEvent.button() << mouseEvent.buttons() << mouseEvent.pos() << mouseEvent.globalPos();

                 //if (m_fakeMouseWidget->focusPolicy() & Qt::ClickFocus) {
                 //    fakeMouseWidget->setFocus();
                 //}
                 QApplication::notify(fakeMouseWidget, &mouseEvent);
                 return true;
             }
        }
        //qDebug() << "return false";
        return false;
        break;
    }
    case QEvent::MouseButtonRelease:
    {
        bool ret = QApplication::notify(target, event);
        if (m_fakeMouseWidget) {
            // It may happen the faked mouse event was grabbed by a non touch window.
            // eg.: if we have started to drag by touch.
            // In this case X11 generates a MouseButtonRelease instead of a TouchPointReleased Event.
            // QApplication still tracks the Touch point and prevent touch to other widgets
            // So we need to fake the Touch release event as well to clean up
            // QApplicationPrivate::widgetForTouchPointId and QApplicationPrivate::appCurrentTouchPoints;
            m_fakeMouseWidget = NULL; // Disable MouseButtonRelease fake
            QList<QTouchEvent::TouchPoint> touchPoints;
            QTouchEvent::TouchPoint tp;
            tp.setId(m_fakeMouseSourcePointId);
            tp.setState(Qt::TouchPointReleased);
            touchPoints.append(tp);
            qt_translateRawTouchEvent(NULL, QTouchEvent::TouchScreen, touchPoints);
        }
        return ret;
    }
    default:
        break;
    }
    // No touch event
    bool ret = QApplication::notify(target, event);
    return ret;
}
#endif // QT_VERSION < QT_VERSION_CHECK(5, 0, 0)

bool MixxxApplication::touchIsRightButton() {
    if (!m_pTouchShift) {
        m_pTouchShift = new ControlObjectSlave(
                "[Controls]", "touch_shift", this);
    }
    return (m_pTouchShift->get() != 0.0);
}
