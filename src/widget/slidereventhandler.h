#ifndef SLIDEREVENTHANDLER_H
#define SLIDEREVENTHANDLER_H

#include <QMouseEvent>
#include <QWheelEvent>
#include <QCursor>
#include <QApplication>
#include <QPoint>

#include "util/math.h"

template <class T>
class SliderEventHandler {
  public:
    SliderEventHandler()
            : m_dStartHandlePos(0),
              m_dStartMousePos(0),
              m_bRightButtonPressed(false),
              m_dOldCOValue(-1.0), // virgin
              m_dPos(0.0),
              m_dHandleLength(0),
              m_dSliderLength(0),
              m_bHorizontal(false),
              m_bDrag(false),
              m_bEventWhileDrag(true) {
        qDebug() << "dpos now111 " << m_dPos;
    }

    void setHorizontal(bool horiz) {
        m_bHorizontal = horiz;
    }

    void setHandleLength(double len) {
        m_dHandleLength = len;
    }

    void setEventWhileDrag(bool eventwhile) {
        m_bEventWhileDrag = eventwhile;
    }

    void mouseMoveEvent(T* pWidget, QMouseEvent* e) {
        if (!m_bRightButtonPressed) {
            double abs_pos = 0.0;
            if (m_bHorizontal) {
                abs_pos = e->x() - m_dHandleLength / 2;
            } else {
                abs_pos = e->y() - m_dHandleLength / 2;
            }

            qDebug() << "start " << m_dStartHandlePos << ", abs pos " << abs_pos;
            m_dPos = m_dStartHandlePos + (abs_pos - m_dStartMousePos);
            qDebug() << "new dpos " << m_dPos;

            //double sliderLength = m_bHorizontal ? pWidget->width() : pWidget->height();

            // Clamp to the range [0, sliderLength - m_dHandleLength].
            m_dPos = math_clamp_unsafe(m_dPos, 0.0, m_dSliderLength - m_dHandleLength);
            qDebug() << "clamped dpos " << m_dPos;

            // Divide by (sliderLength - m_dHandleLength) to produce a normalized
            // value in the range of [0.0, 1.0].
            double newValue = positionToValue(pWidget);
            if (!m_bHorizontal) {
                newValue = 1.0 - newValue;
            }
            qDebug() << "new CO val " << newValue;

            // If we don't change this, then updates might be rejected in
            // onConnectedControlChanged.
            m_dOldCOValue = newValue;

            // Emit valueChanged signal
            if (m_bEventWhileDrag) {
                qDebug() << "EVENT WHILE DRAG " << newValue;
                pWidget->setControlParameter(newValue);
            }

            // Update display
            pWidget->update();
        }
    }

    void mousePressEvent(T* pWidget, QMouseEvent* e) {
        if (!m_bEventWhileDrag) {
            m_dStartMousePos = 0;
            m_dStartHandlePos = 0;
            pWidget->mouseMoveEvent(e);
            m_bDrag = true;
        } else {
            if (e->button() == Qt::RightButton) {
//                pWidget->setControlParameter(1.0);
//                onConnectedControlChanged(pWidget, 1.0, 0);
                pWidget->resetControlParameter();
                pWidget->update();
                m_bRightButtonPressed = true;
            } else {
                qDebug() << "get start pos " << e->y() << " " << m_dHandleLength / 2;
                if (m_bHorizontal) {
                    m_dStartMousePos = e->x() - m_dHandleLength / 2;
                } else {
                    m_dStartMousePos = e->y() - m_dHandleLength / 2;
                }
                m_dStartHandlePos = m_dPos;
                qDebug() << "start handle pos is " << m_dStartHandlePos;
            }
        }
    }

    void mouseReleaseEvent(T* pWidget, QMouseEvent* e) {
        if (!m_bEventWhileDrag) {
            pWidget->mouseMoveEvent(e);
            m_bDrag = false;
        }
        if (e->button() == Qt::RightButton) {
            m_bRightButtonPressed = false;
        } else {
            qDebug() << "restore old value " << m_dOldCOValue;
            pWidget->setControlParameter(m_dOldCOValue);
        }
    }

    void wheelEvent(T* pWidget, QWheelEvent* e) {
        // For legacy (MIDI) reasons this is tuned to 127.
        double wheelAdjustment = ((QWheelEvent *)e)->delta() / (120.0 * 127.0);
        double newValue = pWidget->getControlParameter() + wheelAdjustment;
        qDebug() << "wheel: " << pWidget->getControlParameter() << "--> " << newValue;

        // Clamp to [0.0, 1.0]
        newValue = math_clamp_unsafe(newValue, 0.0, 1.0);

        qDebug() << "CO value is now " << newValue;
        pWidget->setControlParameter(newValue);
        onConnectedControlChanged(pWidget, newValue, 0);
        pWidget->update();
        e->accept();
    }

    void onConnectedControlChanged(T* pWidget, double dParameter, double) {
        // WARNING: The second parameter to this method is unused and called with
        // invalid values in parts of WSliderComposed. Do not use it unless you fix
        // this.

        // We don't update slider values while you're dragging them. This way you
        // don't have to "fight" with a controller that is also changing the
        // control.
        if (m_bDrag) {
            return;
        }

        qDebug() << "got an update " << dParameter;

        if (m_dOldCOValue != dParameter) {
            qDebug() << "UPDATING SLIDER " << dParameter;
            m_dOldCOValue = dParameter;

            qDebug() << "parameter now " << dParameter;
            //double sliderLength = m_bHorizontal ? pWidget->width() : pWidget->height();

            double newPos = valueToPosition(pWidget, dParameter);

            // Clamp to [0.0, sliderLength - m_dHandleLength].
            newPos = math_clamp_unsafe(newPos, 0.0, m_dSliderLength - m_dHandleLength);

            // Check a second time for no-ops. It's possible the parameter changed
            // but the visible pixmap didn't. Only update() the widget if we're
            // really sure we need to since this involves painting ALL of its
            // parents.
            if (newPos != m_dPos) {
                m_dPos = newPos;
                qDebug() << "dpos now " << m_dPos;
                pWidget->setControlParameter(dParameter);
                pWidget->update();
            }
        }
    }

    void resizeEvent(T* pWidget, QResizeEvent* pEvent) {
        Q_UNUSED(pEvent);
        qDebug () << "RESIZE";
        m_dSliderLength = m_bHorizontal ? pWidget->width() : pWidget->height();
        m_dPos = valueToPosition(pWidget, pWidget->getControlParameter());
        m_dOldCOValue = -1;
        qDebug() << "dpos now " << m_dPos;
    }

  private:
    double valueToPosition(T* pWidget, double value) {
        if (m_dSliderLength <= 0) {
            m_dSliderLength = m_bHorizontal ? pWidget->width() : pWidget->height();
        }
        if (!m_bHorizontal) {
            value = 1.0 - value;
        }
        qDebug() << "eventhandler denormalized " << value << " " << m_dSliderLength << " "
                << m_dHandleLength << " " << (value * (m_dSliderLength - m_dHandleLength));
        return value * (m_dSliderLength - m_dHandleLength);
    }

    double positionToValue(T* pWidget) {
        if (m_dSliderLength <= 0) {
            m_dSliderLength = m_bHorizontal ? pWidget->width() : pWidget->height();
        }
        qDebug() << "convert pos to CO val " << m_dPos << m_dSliderLength << " "
                << m_dHandleLength << " to " << m_dPos / (m_dSliderLength - m_dHandleLength);
        return m_dPos / (m_dSliderLength - m_dHandleLength);
    }


    // Internal storage of slider position in pixels
    double m_dStartHandlePos, m_dStartMousePos;
    // True if right mouse button is pressed.
    bool m_bRightButtonPressed;
    // Previous value of the control object, 0 to 1
    double m_dOldCOValue;
    // Internal storage of slider position in pixels
    double m_dPos;
    // Length of handle in pixels
    double m_dHandleLength;
    double m_dSliderLength;
    // True if it's a horizontal slider
    bool m_bHorizontal;
    // True if slider is dragged. Only used when m_bEventWhileDrag is false
    bool m_bDrag;
    // Is true if events is emitted while the slider is dragged
    bool m_bEventWhileDrag;

    // Starting point when left mouse button is pressed
    QPoint m_startPos;
};

#endif /* SLIDEREVENTHANDLER_H */
