#pragma once

#include <QCursor>
#include <QMouseEvent>
#include <QPixmap>
#include <QPoint>
#include <QTimer>
#include <QWheelEvent>

#include "util/math.h"

// duration (ms) the cursor is blanked after a mouse wheel event
// 800 ms is the duration the parameter value is shown in the parameter name widget
// src/widget/weffectparameternamebase.cpp
constexpr int wheelEventCursorTimeout = 800;

template <class T>
class KnobEventHandler {
  public:
    KnobEventHandler()
            : m_bRightButtonPressed(false),
              m_pWheelCursorTimer(nullptr) {
        QPixmap blankPixmap(32, 32);
        blankPixmap.fill(Qt::transparent);
        m_blankCursor = QCursor(blankPixmap);
    }

    double valueFromMouseEvent(T* pWidget, QMouseEvent* e) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QPoint cur = e->globalPosition().toPoint();
#else
        QPoint cur = e->globalPos();
#endif
        QPoint diff = cur - m_prevPos;
        m_prevPos = cur;
        double dist = sqrt(static_cast<double>(diff.x() * diff.x() + diff.y() * diff.y()));
        bool y_dominant = abs(diff.y()) > abs(diff.x());

        // if y is dominant, then treat an increase in dy as negative (y is
        // pointed downward). Otherwise, if y is not dominant and x has
        // decreased, then treat it as negative.
        if ((y_dominant && diff.y() > 0) || (!y_dominant && diff.x() < 0)) {
            dist = -dist;
        }

        // For legacy (MIDI) reasons this is tuned to 127.
        double value = pWidget->getControlParameter() + dist / 127.0;

        // Clamp to [0.0, 1.0]
        value = math_clamp(value, 0.0, 1.0);

        return value;
    }

    void mouseMoveEvent(T* pWidget, QMouseEvent* e) {
        if (!m_bRightButtonPressed) {
            double value = valueFromMouseEvent(pWidget, e);
            pWidget->setControlParameterDown(value);
            pWidget->inputActivity();
        }
    }

    void mousePressEvent(T* pWidget, QMouseEvent* e) {
        switch (e->button()) {
            case Qt::RightButton:
                pWidget->resetControlParameter();
                m_bRightButtonPressed = true;
                break;
            case Qt::LeftButton:
            case Qt::MiddleButton:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                m_startPos = e->globalPosition().toPoint();
#else
                m_startPos = e->globalPos();
#endif
                m_prevPos = m_startPos;
                // Somehow using Qt::BlankCursor does not work on Windows
                // https://mixxx.org/forums/viewtopic.php?p=40298#p40298
                pWidget->setCursor(m_blankCursor);
                break;
            default:
                break;
        }
    }

    void mouseDoubleClickEvent(T* pWidget, QMouseEvent* e) {
        if (e->button() == Qt::LeftButton) {
            pWidget->resetControlParameter();
        }
    }

    void mouseReleaseEvent(T* pWidget, QMouseEvent* e) {
        double value = 0.0;
        switch (e->button()) {
            case Qt::LeftButton:
            case Qt::MiddleButton:
                QCursor::setPos(m_startPos);
                pWidget->unsetCursor();
                value = valueFromMouseEvent(pWidget, e);
                pWidget->setControlParameterUp(value);
                pWidget->inputActivity();
                break;
            case Qt::RightButton:
                m_bRightButtonPressed = false;
                break;
            default:
                break;
        }
    }

    void wheelEvent(T* pWidget, QWheelEvent* e) {
        // Hide/blank the cursor so the parameter value below the knob is not obscured.
        // Restore the cursor when the timer runs out, or when the cursor leaves the widget.
        pWidget->setCursor(m_blankCursor);
        // For legacy (MIDI) reasons this is tuned to 127.
        double wheelDirection = e->angleDelta().y() / (120.0 * 127.0);
        double newValue = pWidget->getControlParameter() + wheelDirection;

        // Clamp to [0.0, 1.0]
        newValue = math_clamp(newValue, 0.0, 1.0);

        pWidget->setControlParameter(newValue);
        pWidget->inputActivity();
        e->accept();
        if (!m_pWheelCursorTimer) {
            m_pWheelCursorTimer = new QTimer(pWidget);
            m_pWheelCursorTimer->setSingleShot(true);
            m_pWheelCursorTimer->setInterval(wheelEventCursorTimeout);
        }
        m_pWheelCursorTimer->start();
        m_pWheelCursorTimer->callOnTimeout(
                [pWidget]() {
                    if (pWidget) {
                        pWidget->unsetCursor();
                    }
                });
    }

    void leaveEvent(T* pWidget, QEvent* e) {
        if (m_pWheelCursorTimer && m_pWheelCursorTimer->isActive()) {
            m_pWheelCursorTimer->stop();
            pWidget->unsetCursor();
        }
        e->accept();
    }

  private:
    // True if right mouse button is pressed.
    bool m_bRightButtonPressed;

    // Starting point when left mouse button is pressed
    QPoint m_startPos;
    QPoint m_prevPos;
    QCursor m_blankCursor;
    QTimer* m_pWheelCursorTimer;
};
