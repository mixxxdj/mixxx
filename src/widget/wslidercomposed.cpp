/***************************************************************************
                          wslidercomposed.cpp  -  description
                             -------------------
    begin                : Tue Jun 25 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "widget/wslidercomposed.h"

#include <QtDebug>
#include <QStylePainter>
#include <QStyleOption>

#include "widget/wpixmapstore.h"
#include "widget/controlwidgetconnection.h"
#include "util/debug.h"
#include "util/math.h"

WSliderComposed::WSliderComposed(QWidget * parent)
    : WWidget(parent),
      m_dOldValue(-1.0), // virgin
      m_bRightButtonPressed(false),
      m_dPos(0),
      m_dStartHandlePos(0),
      m_dStartMousePos(0),
      m_dHandleLength(0),
      m_bHorizontal(false),
      m_bEventWhileDrag(true),
      m_bDrag(false),
      m_pSlider(NULL),
      m_pHandle(NULL) {
}

WSliderComposed::~WSliderComposed() {
    unsetPixmaps();
}

void WSliderComposed::setup(QDomNode node, const SkinContext& context) {
    // Setup pixmaps
    unsetPixmaps();

    if (context.hasNode(node, "Slider")) {
        PixmapSource sourceSlider = context.getPixmapSource(context.selectNode(node, "Slider"));
        setSliderPixmap(sourceSlider);
    }

    PixmapSource sourceHandle = context.getPixmapSource(context.selectNode(node, "Handle"));
    bool h = context.selectBool(node, "Horizontal", false);
    setHandlePixmap(h, sourceHandle);

    if (context.hasNode(node, "EventWhileDrag")) {
        if (context.selectString(node, "EventWhileDrag").contains("no")) {
            m_bEventWhileDrag = false;
        }
    }
    if (!m_connections.isEmpty()) {
        ControlParameterWidgetConnection* defaultConnection = m_connections.at(0);
        if (defaultConnection) {
            if (defaultConnection->getEmitOption() &
                    ControlParameterWidgetConnection::EMIT_DEFAULT) {
                // ON_PRESS means here value change on mouse move during press
                defaultConnection->setEmitOption(
                        ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE);
            }
        }
    }
}

void WSliderComposed::setSliderPixmap(PixmapSource sourceSlider) {
    m_pSlider = WPixmapStore::getPaintable(sourceSlider,
                                           Paintable::STRETCH);
    if (!m_pSlider) {
        qDebug() << "WSliderComposed: Error loading slider pixmap:" << sourceSlider.getPath();
    } else {
        // Set size of widget, using size of slider pixmap
        setFixedSize(m_pSlider->size());
    }
}

void WSliderComposed::setHandlePixmap(bool bHorizontal, PixmapSource sourceHandle) {
    m_bHorizontal = bHorizontal;
    m_pHandle = WPixmapStore::getPaintable(sourceHandle,
                                           Paintable::STRETCH);
    if (!m_pHandle) {
        qDebug() << "WSliderComposed: Error loading handle pixmap:" << sourceHandle.getPath();
    } else {
        if (m_bHorizontal) {
            // Stretch the pixmap to be the height of the widget.
            if (m_pHandle->height() != 0.0) {
                const qreal aspect = static_cast<double>(m_pHandle->width()) /
                        static_cast<double>(m_pHandle->height());
                m_dHandleLength = aspect * height();
            } else {
                m_dHandleLength = m_pHandle->width();
            }
        } else {
            // Stretch the pixmap to be the width of the widget.
            if (m_pHandle->width() != 0.0) {
                const qreal aspect = static_cast<double>(m_pHandle->height()) /
                        static_cast<double>(m_pHandle->width());
                m_dHandleLength = aspect * width();
            } else {
                m_dHandleLength = m_pHandle->height();
            }
        }

        // Value is unused in WSliderComposed.
        onConnectedControlChanged(getControlParameter(), 0);
        update();
    }
}

void WSliderComposed::unsetPixmaps() {
    m_pSlider.clear();
    m_pHandle.clear();
}

void WSliderComposed::mouseMoveEvent(QMouseEvent * e) {
    if (!m_bRightButtonPressed) {
        if (m_bHorizontal) {
            m_dPos = e->x() - m_dHandleLength / 2;
        } else {
            m_dPos = e->y() - m_dHandleLength / 2;
        }

        //qDebug() << "start " << m_dStartPos << ", pos " << m_dPos;
        m_dPos = m_dStartHandlePos + (m_dPos - m_dStartMousePos);

        double sliderLength = m_bHorizontal ? width() : height();

        // Clamp to the range [0, sliderLength - m_dHandleLength].
        m_dPos = math_clamp_unsafe(m_dPos, 0.0, sliderLength - m_dHandleLength);

        // Divide by (sliderLength - m_dHandleLength) to produce a normalized
        // value in the range of [0.0, 1.0].
        double newValue = m_dPos / (sliderLength - m_dHandleLength);
        if (!m_bHorizontal) {
            newValue = 1.0 - newValue;
        }

        // If we don't change this, then updates might be rejected in
        // onConnectedControlChanged.
        m_dOldValue = newValue;

        // Emit valueChanged signal
        if (m_bEventWhileDrag) {
            setControlParameter(newValue);
        }

        // Update display
        update();
    }
}

void WSliderComposed::wheelEvent(QWheelEvent *e) {
    // For legacy (MIDI) reasons this is tuned to 127.
    double wheelDirection = ((QWheelEvent *)e)->delta() / (120.0 * 127.0);
    double newValue = m_dOldValue + wheelDirection;

    // Clamp to [0.0, 1.0]
    newValue = math_clamp_unsafe(newValue, 0.0, 1.0);

    setControlParameter(newValue);
    // Value is unused in WSliderComposed.
    onConnectedControlChanged(newValue, 0);
    update();

    e->accept();

    //e->ignore();
}

void WSliderComposed::mouseReleaseEvent(QMouseEvent * e) {
    if (!m_bEventWhileDrag) {
        mouseMoveEvent(e);
        m_bDrag = false;
    }
    if (e->button() == Qt::RightButton) {
        m_bRightButtonPressed = false;
    } else {
        setControlParameter(m_dOldValue);
    }
}

void WSliderComposed::mousePressEvent(QMouseEvent * e) {
    if (!m_bEventWhileDrag) {
        m_dStartMousePos = 0;
        m_dStartHandlePos = 0;
        mouseMoveEvent(e);
        m_bDrag = true;
    } else {
        if (e->button() == Qt::RightButton) {
            resetControlParameter();
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

void WSliderComposed::paintEvent(QPaintEvent *) {
    QStyleOption option;
    option.initFrom(this);
    QStylePainter p(this);
    p.drawPrimitive(QStyle::PE_Widget, option);

    if (!m_pSlider.isNull() && !m_pSlider->isNull()) {
        m_pSlider->draw(0, 0, &p);
    }

    if (!m_pHandle.isNull() && !m_pHandle->isNull()) {
        if (m_bHorizontal) {
            // Stretch the pixmap to be the height of the widget.
            QRectF targetRect(m_dPos, 0, m_dHandleLength, height());
            m_pHandle->draw(targetRect, &p);
        } else {
            // Stretch the pixmap to be the width of the widget.
            QRectF targetRect(0, m_dPos, width(), m_dHandleLength);
            m_pHandle->draw(targetRect, &p);
        }
    }
}

void WSliderComposed::resizeEvent(QResizeEvent* pEvent) {
    Q_UNUSED(pEvent);
    m_dOldValue = -1;
    m_dPos = -1;

    if (m_bHorizontal) {
        // Stretch the pixmap to be the height of the widget.
        if (m_pHandle->height() != 0.0) {
            const qreal aspect = static_cast<double>(m_pHandle->width()) /
                    static_cast<double>(m_pHandle->height());
            m_dHandleLength = aspect * height();
        } else {
            m_dHandleLength = m_pHandle->width();
        }
    } else {
        // Stretch the pixmap to be the width of the widget.
        if (m_pHandle->width() != 0.0) {
            const qreal aspect = static_cast<double>(m_pHandle->height()) /
                    static_cast<double>(m_pHandle->width());
            m_dHandleLength = aspect * width();
        } else {
            m_dHandleLength = m_pHandle->height();
        }
    }

    // Re-calculate m_dPos based on our new width/height.
    onConnectedControlChanged(getControlParameter(), 0);
}

void WSliderComposed::onConnectedControlChanged(double dParameter, double) {
    // WARNING: The second parameter to this method is unused and called with
    // invalid values in parts of WSliderComposed. Do not use it unless you fix
    // this.

    // We don't update slider values while you're dragging them. This way you
    // don't have to "fight" with a controller that is also changing the
    // control.
    if (m_bDrag) {
        return;
    }

    if (m_dOldValue != dParameter) {
        m_dOldValue = dParameter;

        // Calculate handle position
        if (!m_bHorizontal) {
            dParameter = 1.0 - dParameter;
        }
        double sliderLength = m_bHorizontal ? width() : height();

        double newPos = dParameter * (sliderLength - m_dHandleLength);

        // Clamp to [0.0, sliderLength - m_dHandleLength].
        newPos = math_clamp_unsafe(newPos, 0.0, sliderLength - m_dHandleLength);

        // Check a second time for no-ops. It's possible the parameter changed
        // but the visible pixmap didn't. Only update() the widget if we're
        // really sure we need to since this involves painting ALL of its
        // parents.
        if (newPos != m_dPos) {
            m_dPos = newPos;
            update();
        }
    }
}

void WSliderComposed::fillDebugTooltip(QStringList* debug) {
    WWidget::fillDebugTooltip(debug);
    int sliderLength = m_bHorizontal ? width() : height();
    *debug << QString("Horizontal: %1").arg(toDebugString(m_bHorizontal))
           << QString("SliderPosition: %1").arg(m_dPos)
           << QString("SliderLength: %1").arg(sliderLength)
           << QString("HandleLength: %1").arg(m_dHandleLength);
}
