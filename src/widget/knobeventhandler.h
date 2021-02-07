#pragma once

#include <QMouseEvent>
#include <QWheelEvent>
#include <QColor>
#include <QCursor>
#include <QApplication>
#include <QPoint>
#include <QPixmap>

#include "util/math.h"

template <class T>
class KnobEventHandler {
  public:
    KnobEventHandler()
            : m_bRightButtonPressed(false) {
            QPixmap blankPixmap(32, 32);
            blankPixmap.fill(QColor(0, 0, 0, 0));
            m_blankCursor = QCursor(blankPixmap);
    }

    double valueFromMouseEvent(T* pWidget, QMouseEvent* e) {
        QPoint cur(e->globalPos());
        QPoint diff(cur - m_prevPos);
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
            m_prevPos = e->globalPos();
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
                m_startPos = e->globalPos();
                m_prevPos = m_startPos;
                // Somehow using Qt::BlankCursor does not work on Windows
                // https://mixxx.org/forums/viewtopic.php?p=40298#p40298
                QApplication::setOverrideCursor(m_blankCursor);
                break;
            default:
                break;
        }
    }

    void mouseReleaseEvent(T* pWidget, QMouseEvent* e) {
        double value = 0.0;
        switch (e->button()) {
            case Qt::LeftButton:
            case Qt::MiddleButton:
                QCursor::setPos(m_startPos);
                QApplication::restoreOverrideCursor();
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
        // For legacy (MIDI) reasons this is tuned to 127.
        double wheelDirection = e->angleDelta().y() / (120.0 * 127.0);
        double newValue = pWidget->getControlParameter() + wheelDirection;

        // Clamp to [0.0, 1.0]
        newValue = math_clamp(newValue, 0.0, 1.0);

        pWidget->setControlParameter(newValue);
        pWidget->inputActivity();
        e->accept();
    }

  private:
    // True if right mouse button is pressed.
    bool m_bRightButtonPressed;

    // Starting point when left mouse button is pressed
    QPoint m_startPos;
    QPoint m_prevPos;
    QCursor m_blankCursor;
};
