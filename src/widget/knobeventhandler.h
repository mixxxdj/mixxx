#ifndef KNOBEVENTHANDLER_H
#define KNOBEVENTHANDLER_H

#include <QMouseEvent>
#include <QWheelEvent>
#include <QCursor>
#include <QApplication>
#include <QPoint>

#include "util/math.h"

template <class T>
class KnobEventHandler {
  public:
    KnobEventHandler()
            : m_bRightButtonPressed(false) {
    }

    double valueFromMouseEvent(T* pWidget, QMouseEvent* e) {
        QPoint cur(e->globalPos());
        QPoint diff(cur - m_startPos);
        double dist = sqrt(static_cast<double>(diff.x() * diff.x() + diff.y() * diff.y()));
        bool y_dominant = abs(diff.y()) > abs(diff.x());

        // if y is dominant, then thread an increase in dy as negative (y is
        // pointed downward). Otherwise, if y is not dominant and x has
        // decreased, then thread it as negative.
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
            QCursor::setPos(m_startPos);
            double value = valueFromMouseEvent(pWidget, e);
            pWidget->setControlParameterDown(value);
            pWidget->update();
        }
    }

    void mousePressEvent(T* pWidget, QMouseEvent* e) {
        switch (e->button()) {
            case Qt::RightButton:
                pWidget->resetControlParameter();
                m_bRightButtonPressed = true;
                break;
            case Qt::LeftButton:
            case Qt::MidButton:
                m_startPos = e->globalPos();
                QApplication::setOverrideCursor(Qt::BlankCursor);
                break;
            default:
                break;
        }
    }

    void mouseReleaseEvent(T* pWidget, QMouseEvent* e) {
        double value = 0.0;
        switch (e->button()) {
            case Qt::LeftButton:
            case Qt::MidButton:
                QCursor::setPos(m_startPos);
                QApplication::restoreOverrideCursor();
                value = valueFromMouseEvent(pWidget, e);
                pWidget->setControlParameterUp(value);
                pWidget->update();
                break;
            case Qt::RightButton:
                m_bRightButtonPressed = false;
                break;
            default:
                break;
        }
        pWidget->update();
    }

    void wheelEvent(T* pWidget, QWheelEvent* e) {
        // For legacy (MIDI) reasons this is tuned to 127.
        double wheelDirection = e->delta() / (120.0 * 127.0);
        double newValue = pWidget->getControlParameter() + wheelDirection;

        // Clamp to [0.0, 1.0]
        newValue = math_clamp(newValue, 0.0, 1.0);

        pWidget->setControlParameter(newValue);
        pWidget->update();
        e->accept();
    }

  private:
    // True if right mouse button is pressed.
    bool m_bRightButtonPressed;

    // Starting point when left mouse button is pressed
    QPoint m_startPos;
};

#endif /* KNOBEVENTHANDLER_H */
