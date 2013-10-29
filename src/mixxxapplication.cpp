
#include <QtDebug>
#include <QTouchEvent>


#include "mixxxapplication.h"
#include "controlobjectthread.h"
#include "mixxx.h"


MixxxApplication::MixxxApplication(int& argc, char** argv)
        : QApplication(argc, argv),
          m_fakeMouseSourcePointId(0),
          m_fakeMouseWidget(NULL),
          m_activeTouchButton(Qt::NoButton),
          m_pTouchShift(NULL) {
}

MixxxApplication::~MixxxApplication() {
    delete m_pTouchShift;
}

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

        qDebug() << "&" << touchEvent->type() << target;

       // return QApplication::notify(target, event);



        if (touchEvent->deviceType() !=  QTouchEvent::TouchScreen) {
            break;
        }

        switch (event->type()) {
        case QEvent::TouchBegin:
            // try to deliver as touch event
            (void)QApplication::notify(target, event);
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
            }
            break;
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

                  qDebug() << "#" << mouseEvent.type() << mouseEvent.button() << mouseEvent.buttons() << mouseEvent.pos() << mouseEvent.globalPos();

                  // touch event was not accepted by any widget in the Main window
                  // send as the previously prepared faked Mouse event now.
                  //if (m_fakeMouseWidget->focusPolicy() & Qt::ClickFocus) {
                  //    fakeMouseWidget->setFocus();
                  //}
                  QApplication::notify(fakeMouseWidget, &mouseEvent);
                  return true;
            }
        }
        qDebug() << "return false";
        return false;
        break;
    }
    case QEvent::MouseButtonPress:
    case QEvent::MouseMove:
        qDebug() << "+" << event->type() << target;
        break;
    case QEvent::MouseButtonRelease:
        qDebug() << "+" << event->type() << target;
        QApplication::notify(target, event);
        if (m_fakeMouseWidget) {
            QApplication::notify(m_fakeMouseWidget, event);
            m_fakeMouseWidget->setAttribute(Qt::WA_WState_AcceptedTouchBeginEvent, false);
            m_fakeMouseWidget = NULL;
        }
        return true;
    default:
        break;
    }
    // No touch event
    return QApplication::notify(target, event);
}

bool MixxxApplication::touchIsRightButton() {
    if (!m_pTouchShift) {
        m_pTouchShift = new ControlObjectThread("[Controls]", "touch_shift");
    }
    return (m_pTouchShift->get() != 0.0);
}
