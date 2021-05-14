#pragma once

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
              m_dOldParameter(-1.0), // virgin
              m_dPos(0.0),
              m_dHandleLength(0),
              m_dSliderLength(0),
              m_bHorizontal(false),
              m_bDrag(false),
              m_bEventWhileDrag(true) { }

    void setHorizontal(bool horiz) {
        m_bHorizontal = horiz;
    }

    void setHandleLength(double len) {
        m_dHandleLength = len;
    }

    void setSliderLength(double len) {
        m_dSliderLength = len;
    }

    void setEventWhileDrag(bool eventwhile) {
        m_bEventWhileDrag = eventwhile;
    }

    void mouseMoveEvent(T* pWidget, QMouseEvent* e) {
        if (!m_bRightButtonPressed) {
            if (m_bHorizontal) {
                m_dPos = e->x() - m_dHandleLength / 2;
            } else {
                m_dPos = e->y() - m_dHandleLength / 2;
            }

            m_dPos = m_dStartHandlePos + (m_dPos - m_dStartMousePos);

            // Clamp to the range [0, sliderLength - m_dHandleLength].
            if (m_dSliderLength - m_dHandleLength > 0.0) {
                m_dPos = math_clamp(m_dPos, 0.0, m_dSliderLength - m_dHandleLength);
            }
            double newParameter = positionToParameter(m_dPos);

            // If we don't change this, then updates might be rejected in
            // onConnectedControlChanged.
            m_dOldParameter = newParameter;

            // Emit valueChanged signal
            if (m_bEventWhileDrag) {
                pWidget->setControlParameter(newParameter);
            }

            // Update display
            pWidget->inputActivity();
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
                pWidget->resetControlParameter();
                m_bRightButtonPressed = true;
            } else {
                if (m_bHorizontal) {
                    m_dStartMousePos = e->x() - m_dHandleLength / 2;
                } else {
                    m_dStartMousePos = e->y() - m_dHandleLength / 2;
                }
                m_dStartHandlePos = m_dPos;
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
            pWidget->setControlParameter(m_dOldParameter);
        }
    }

    void wheelEvent(T* pWidget, QWheelEvent* e) {
        // For legacy (MIDI) reasons this is tuned to 127.
        double wheelAdjustment = (e)->angleDelta().y() / (120.0 * 127.0);
        double newParameter = pWidget->getControlParameter() + wheelAdjustment;

        // Clamp to [0.0, 1.0]
        newParameter = math_clamp(newParameter, 0.0, 1.0);

        pWidget->setControlParameter(newParameter);
        onConnectedControlChanged(pWidget, newParameter);
        pWidget->inputActivity();
        e->accept();
    }

    void onConnectedControlChanged(T* pWidget, double dParameter) {
        // WARNING: The second parameter to this method is unused and called with
        // invalid values in parts of WSliderComposed. Do not use it unless you fix
        // this.

        // We don't update slider values while you're dragging them. This way you
        // don't have to "fight" with a controller that is also changing the
        // control.
        if (m_bDrag) {
            return;
        }

        if (m_dOldParameter != dParameter) {
            m_dOldParameter = dParameter;

            double newPos = parameterToPosition(dParameter);

            // Clamp to [0.0, sliderLength - m_dHandleLength].
            if (m_dSliderLength - m_dHandleLength > 0.0) {
                newPos = math_clamp(newPos, 0.0, m_dSliderLength - m_dHandleLength);
            }

            // Check a second time for no-ops. It's possible the parameter changed
            // but the visible pixmap didn't. Only update() the widget if we're
            // really sure we need to since this involves painting ALL of its
            // parents.
            if (newPos != m_dPos) {
                m_dPos = newPos;
                pWidget->update();
            }
        }
    }

    void resizeEvent(T* pWidget, QResizeEvent* pEvent) {
        Q_UNUSED(pEvent);
        // m_dSliderLength and m_dHandleLength are explicitly updated.
        m_dPos = parameterToPosition(pWidget->getControlParameter());
        m_dOldParameter = -1;
    }

    // Convert CO parameter value to a handle pixel position.
    double parameterToPosition(double parameter) const {
        if (m_dSliderLength - m_dHandleLength <= 0.0) {
            return 0.0;
        }
        if (!m_bHorizontal) {
            parameter = 1.0 - parameter;
        }
        return parameter * (m_dSliderLength - m_dHandleLength);
    }

    // Convert handle pixel position to a CO parameter value.
    double positionToParameter(double pos) const {
        if (m_dSliderLength - m_dHandleLength <= 0.0) {
            return 0.0;
        }
        double val = pos / (m_dSliderLength - m_dHandleLength);
        if (!m_bHorizontal) {
            return 1.0 - val;
        }
        return val;
    }

  private:
    // This is the position the handle was when a drag started.
    double m_dStartHandlePos;
    // We record where the mouse was when the user started clicking so they
    // don't need to perfectly grab the slider handle.
    double m_dStartMousePos;
    // True while right mouse button is pressed.
    bool m_bRightButtonPressed;
    // Previous parameter value of the control object, 0 to 1
    double m_dOldParameter;
    // Internal storage of slider position in pixels
    double m_dPos;
    // Length of handle in pixels
    double m_dHandleLength;
    // Length of the slider in pixels
    double m_dSliderLength;
    // True if it's a horizontal slider
    bool m_bHorizontal;
    // True if slider is being dragged. Only used when m_bEventWhileDrag is false
    bool m_bDrag;
    // Is true if events is emitted while the slider is dragged
    bool m_bEventWhileDrag;
};
