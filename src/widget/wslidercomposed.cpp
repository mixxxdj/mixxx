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
      m_iPos(0),
      m_iStartHandlePos(0),
      m_iStartMousePos(0),
      m_iHandleLength(0),
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
        QString pathSlider = context.getPixmapPath(context.selectNode(node, "Slider"));
        setSliderPixmap(pathSlider);
    }

    QString pathHandle = context.getPixmapPath(context.selectNode(node, "Handle"));
    bool h = context.selectBool(node, "Horizontal", false);
    setHandlePixmap(h, pathHandle);

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

void WSliderComposed::setSliderPixmap(const QString& filenameSlider) {
    m_pSlider = WPixmapStore::getPaintable(filenameSlider,
                                           Paintable::STRETCH);
    if (!m_pSlider) {
        qDebug() << "WSliderComposed: Error loading slider pixmap:" << filenameSlider;
    } else {
        // Set size of widget, using size of slider pixmap
        setFixedSize(m_pSlider->size());
    }
}

void WSliderComposed::setHandlePixmap(bool bHorizontal, const QString& filenameHandle) {
    m_bHorizontal = bHorizontal;
    m_pHandle = WPixmapStore::getPaintable(filenameHandle,
                                           Paintable::STRETCH);
    if (!m_pHandle) {
        qDebug() << "WSliderComposed: Error loading handle pixmap:" << filenameHandle;
    } else {
        m_iHandleLength = m_bHorizontal ?
                m_pHandle->width() : m_pHandle->height();

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
            m_iPos = e->x() - m_iHandleLength / 2;
        } else {
            m_iPos = e->y() - m_iHandleLength / 2;
        }

        //qDebug() << "start " << m_iStartPos << ", pos " << m_iPos;
        m_iPos = m_iStartHandlePos + (m_iPos - m_iStartMousePos);

        int sliderLength = m_bHorizontal ? width() : height();

        // Clamp to the range [0, sliderLength - m_iHandleLength].
        if (m_iPos > (sliderLength - m_iHandleLength)) {
            m_iPos = sliderLength - m_iHandleLength;
        } else if (m_iPos < 0) {
            m_iPos = 0;
        }

        // Divide by (sliderLength - m_iHandleLength) to produce a normalized
        // value in the range of [0.0, 1.0].  1.0
        double newValue = static_cast<double>(m_iPos) /
                static_cast<double>(sliderLength - m_iHandleLength);
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
    newValue = math_clamp(newValue, 0.0, 1.0);

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
        m_iStartMousePos = 0;
        m_iStartHandlePos = 0;
        mouseMoveEvent(e);
        m_bDrag = true;
    } else {
        if (e->button() == Qt::RightButton) {
            resetControlParameter();
            m_bRightButtonPressed = true;
        } else {
            if (m_bHorizontal) {
                m_iStartMousePos = e->x() - m_iHandleLength / 2;
            } else {
                m_iStartMousePos = e->y() - m_iHandleLength / 2;
            }
            m_iStartHandlePos = m_iPos;
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
        int posx = m_bHorizontal ? m_iPos : 0;
        int posy = m_bHorizontal ? 0 : m_iPos;
        m_pHandle->draw(posx, posy, &p);
    }
}

void WSliderComposed::resizeEvent(QResizeEvent* pEvent) {
    Q_UNUSED(pEvent);
    m_dOldValue = -1;
    m_iPos = -1;
    // Re-calculate m_iPos based on our new width/height.
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
        int sliderLength = m_bHorizontal ? width() : height();

        int newPos = static_cast<int>(dParameter * (sliderLength - m_iHandleLength));
        if (newPos > (sliderLength - m_iHandleLength)) {
            newPos = sliderLength - m_iHandleLength;
        } else if (newPos < 0) {
            newPos = 0;
        }

        // Check a second time for no-ops. It's possible the parameter changed
        // but the visible pixmap didn't. Only update() the widget if we're
        // really sure we need to since this involves painting ALL of its
        // parents.
        if (newPos != m_iPos) {
            m_iPos = newPos;
            update();
        }
    }
}

void WSliderComposed::fillDebugTooltip(QStringList* debug) {
    WWidget::fillDebugTooltip(debug);
    int sliderLength = m_bHorizontal ? width() : height();
    *debug << QString("Horizontal: %1").arg(toDebugString(m_bHorizontal))
           << QString("SliderPosition: %1").arg(m_iPos)
           << QString("SliderLength: %1").arg(sliderLength)
           << QString("HandleLength: %1").arg(m_iHandleLength);
}
