#ifndef KNOBEVENTHANDLER_H
#define KNOBEVENTHANDLER_H

#include <QMouseEvent>
#include <QWheelEvent>
#include <QCursor>
#include <QApplication>
#include <QPoint>

#include "defs.h"

template <class T>
class KnobEventHandler {
  public:
    KnobEventHandler()
            : m_bRightButtonPressed(false) {
    }

    void mouseMoveEvent(T* pWidget, QMouseEvent* e) {
        if (!m_bRightButtonPressed) {
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

            double value = pWidget->getValue();
            value += dist;
            QCursor::setPos(m_startPos);

            if (value > 127.0)
                value = 127.0;
            else if (value < 0.0)
                value = 0.0;

            pWidget->setValue(value);
            emit(pWidget->valueChangedLeftDown(value));
            pWidget->update();
        }
    }

    void mousePressEvent(T* pWidget, QMouseEvent* e) {
        switch (e->button()) {
            case Qt::RightButton:
                emit(pWidget->valueReset());
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
        switch (e->button()) {
            case Qt::LeftButton:
            case Qt::MidButton:
                QCursor::setPos(m_startPos);
                QApplication::restoreOverrideCursor();
                emit(pWidget->valueChangedLeftUp(pWidget->getValue()));
                break;
            case Qt::RightButton:
                m_bRightButtonPressed = false;
                //emit(pWidget->valueChangedRightUp(m_fValue));
                break;
            default:
                break;
        }
        pWidget->update();
    }

    void wheelEvent(T* pWidget, QWheelEvent* e) {
        double wheelDirection = e->delta() / 120.;
        double newValue = pWidget->getValue() + wheelDirection;

        // Clamp to [0.0, 127.0]
        newValue = math_max(0.0, math_min(127.0, newValue));

        pWidget->updateValue(newValue);
        e->accept();
    }

  private:
    // True if right mouse button is pressed.
    bool m_bRightButtonPressed;

    // Starting point when left mouse button is pressed
    QPoint m_startPos;
};

#endif /* KNOBEVENTHANDLER_H */
