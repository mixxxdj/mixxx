#include <QtDebug>
#include <QMouseEvent>
#include <QKeyEvent>

#include "widget/wwidget.h"
#include "controllers/controllerlearningeventfilter.h"

ControllerLearningEventFilter::ControllerLearningEventFilter(QObject* pParent)
        : QObject(pParent) {
}

ControllerLearningEventFilter::~ControllerLearningEventFilter() {
}

bool ControllerLearningEventFilter::eventFilter(QObject* pObject, QEvent* pEvent) {
    //qDebug() << "ControllerLearningEventFilter::eventFilter" << pObject << pEvent;

    WWidget* pWidget = dynamic_cast<WWidget*>(pObject);
    if (!pWidget) {
        return false;
    }

    if (pEvent->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = reinterpret_cast<QKeyEvent*>(pEvent);
    } else if (pEvent->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = reinterpret_cast<QMouseEvent*>(pEvent);
        qDebug() << "MouseButtonPress" << pWidget;

        const ControlInfo& info = m_widgetControlInfo[pWidget];
        if (mouseEvent->button() & Qt::LeftButton) {
            if (info.leftClickControl) {
                ConfigKey key = info.leftClickControl->getKey();
                qDebug() << "Left-click maps MIDI to:" << key.group << key.item;
            } else if (info.clickControl) {
                ConfigKey key = info.clickControl->getKey();
                qDebug() << "Default-click maps MIDI to:" << key.group << key.item;
            } else {
                qDebug() << "No control bound to left-click for" << pWidget;
            }

        }
        if (mouseEvent->button() & Qt::RightButton) {
            if (info.rightClickControl) {
                ConfigKey key = info.rightClickControl->getKey();
                qDebug() << "Right-click maps MIDI to:" << key.group << key.item;
            } else if (info.clickControl) {
                ConfigKey key = info.clickControl->getKey();
                qDebug() << "Default-click maps MIDI to:" << key.group << key.item;
            } else {
                qDebug() << "No control bound to right-click for" << pWidget;
            }
        }
    } else if (pEvent->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = reinterpret_cast<QMouseEvent*>(pEvent);
        qDebug() << "MouseButtonRelease" << pWidget;
    } else if (pEvent->type() == QEvent::MouseMove) {
        QMouseEvent* mouseEvent = reinterpret_cast<QMouseEvent*>(pEvent);
        qDebug() << "MouseMoveEvent" << pWidget;
    }
    return false;
}

void ControllerLearningEventFilter::addWidgetClickInfo(
    QWidget* pWidget, Qt::MouseButton buttonState,
    ControlObject* pControl,
    ControlObjectThreadWidget::EmitOption emitOption) {
    ControlInfo& info = m_widgetControlInfo[pWidget];

    if (buttonState == Qt::LeftButton) {
        info.leftClickControl = pControl;
        info.leftEmitOption = emitOption;
    } else if (buttonState == Qt::RightButton) {
        info.rightClickControl = pControl;
        info.rightEmitOption = emitOption;
    } else {
        info.clickControl = pControl;
        info.emitOption = emitOption;
    }
}
