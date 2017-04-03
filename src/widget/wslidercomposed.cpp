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

#include "widget/controlwidgetconnection.h"
#include "widget/wpixmapstore.h"
#include "util/debug.h"
#include "util/math.h"

WSliderComposed::WSliderComposed(QWidget * parent)
    : WWidget(parent),
      m_bRightButtonPressed(false),
      m_dHandleLength(0.0),
      m_dSliderLength(0.0),
      m_bHorizontal(false),
      m_pSlider(nullptr),
      m_pHandle(nullptr) {
}

WSliderComposed::~WSliderComposed() {
    unsetPixmaps();
}

void WSliderComposed::setup(const QDomNode& node, const SkinContext& context) {
    // Setup pixmaps
    unsetPixmaps();

    QDomElement slider = context.selectElement(node, "Slider");
    if (!slider.isNull()) {
        // The implicit default in <1.12.0 was FIXED so we keep it for backwards
        // compatibility.
        PixmapSource sourceSlider = context.getPixmapSource(slider);
        setSliderPixmap(
                sourceSlider,
                context.selectScaleMode(slider, Paintable::FIXED),
                context.getScaleFactor());
    }

    m_dSliderLength = m_bHorizontal ? width() : height();
    m_handler.setSliderLength(m_dSliderLength);

    QDomElement handle = context.selectElement(node, "Handle");
    PixmapSource sourceHandle = context.getPixmapSource(handle);
    bool h = context.selectBool(node, "Horizontal", false);
    // The implicit default in <1.12.0 was FIXED so we keep it for backwards
    // compatibility.
    setHandlePixmap(h, sourceHandle,
                    context.selectScaleMode(handle, Paintable::FIXED),
                    context.getScaleFactor());

    QString eventWhileDrag;
    if (context.hasNodeSelectString(node, "EventWhileDrag", &eventWhileDrag)) {
        if (eventWhileDrag.contains("no")) {
            m_handler.setEventWhileDrag(false);
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

void WSliderComposed::setSliderPixmap(PixmapSource sourceSlider,
                                      Paintable::DrawMode drawMode,
                                      double scaleFactor) {
    m_pSlider = WPixmapStore::getPaintable(sourceSlider, drawMode, scaleFactor);
    if (!m_pSlider) {
        qDebug() << "WSliderComposed: Error loading slider pixmap:" << sourceSlider.getPath();
    } else if (drawMode == Paintable::FIXED) {
        // Set size of widget, using size of slider pixmap
        setFixedSize(m_pSlider->size());
    }
}

void WSliderComposed::setHandlePixmap(bool bHorizontal,
                                      PixmapSource sourceHandle,
                                      Paintable::DrawMode mode,
                                      double scaleFactor) {
    m_bHorizontal = bHorizontal;
    m_handler.setHorizontal(m_bHorizontal);
    m_pHandle = WPixmapStore::getPaintable(sourceHandle, mode, scaleFactor);
    m_dHandleLength = calculateHandleLength();
    m_handler.setHandleLength(m_dHandleLength);
    if (!m_pHandle) {
        qDebug() << "WSliderComposed: Error loading handle pixmap:" << sourceHandle.getPath();
    } else {
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
    m_handler.mouseMoveEvent(this, e);
}

void WSliderComposed::wheelEvent(QWheelEvent *e) {
    m_handler.wheelEvent(this, e);
}

void WSliderComposed::mouseReleaseEvent(QMouseEvent * e) {
    m_handler.mouseReleaseEvent(this, e);
}

void WSliderComposed::mousePressEvent(QMouseEvent * e) {
    m_handler.mousePressEvent(this, e);
}

void WSliderComposed::paintEvent(QPaintEvent * /*unused*/) {
    QStyleOption option;
    option.initFrom(this);
    QStylePainter p(this);
    p.drawPrimitive(QStyle::PE_Widget, option);

    if (!m_pSlider.isNull() && !m_pSlider->isNull()) {
        m_pSlider->draw(rect(), &p);
    }

    if (!m_pHandle.isNull() && !m_pHandle->isNull()) {
        // Slider position rounded, verify this for HiDPI : bug 1479037
        double drawPos = round(m_handler.parameterToPosition(getControlParameterDisplay()));
        if (m_bHorizontal) {
            // The handle's draw mode determines whether it is stretched.
            QRectF targetRect(drawPos, 0, m_dHandleLength, height());
            m_pHandle->draw(targetRect, &p);
        } else {
            // The handle's draw mode determines whether it is stretched.
            QRectF targetRect(0, drawPos, width(), m_dHandleLength);
            m_pHandle->draw(targetRect, &p);
        }
    }
}

void WSliderComposed::resizeEvent(QResizeEvent* pEvent) {
    Q_UNUSED(pEvent);

    m_dHandleLength = calculateHandleLength();
    m_handler.setHandleLength(m_dHandleLength);
    m_dSliderLength = m_bHorizontal ? width() : height();
    m_handler.setSliderLength(m_dSliderLength);
    m_handler.resizeEvent(this, pEvent);

    // Re-calculate state based on our new width/height.
    onConnectedControlChanged(getControlParameter(), 0);
}

void WSliderComposed::onConnectedControlChanged(double dParameter, double /*dValue*/) {
    m_handler.onConnectedControlChanged(this, dParameter);
}

void WSliderComposed::fillDebugTooltip(QStringList* debug) {
    WWidget::fillDebugTooltip(debug);
    int sliderLength = m_bHorizontal ? width() : height();
    *debug << QString("Horizontal: %1").arg(toDebugString(m_bHorizontal))
           << QString("SliderPosition: %1").arg(
                   m_handler.parameterToPosition(getControlParameterDisplay()))
           << QString("SliderLength: %1").arg(sliderLength)
           << QString("HandleLength: %1").arg(m_dHandleLength);
}

double WSliderComposed::calculateHandleLength() {
    if (m_pHandle) {
        Paintable::DrawMode mode = m_pHandle->drawMode();
        if (m_bHorizontal) {
            // Stretch the pixmap to be the height of the widget.
            if (mode == Paintable::FIXED || mode == Paintable::STRETCH ||
                    mode == Paintable::TILE || m_pHandle->height() == 0.0) {
                return m_pHandle->width();
            } else if (mode == Paintable::STRETCH_ASPECT) {
                const int iHeight = m_pHandle->height();
                if (iHeight == 0) {
                  qDebug() << "WSliderComposed: Invalid height.";
                  return 0.0;
                }
                const qreal aspect =
                  static_cast<qreal>(m_pHandle->width()) / iHeight;
                return aspect * height();
            }
        } else {
            // Stretch the pixmap to be the width of the widget.
            if (mode == Paintable::FIXED || mode == Paintable::STRETCH ||
                    mode == Paintable::TILE || m_pHandle->width() == 0.0) {
                return m_pHandle->height();
            } else if (mode == Paintable::STRETCH_ASPECT) {
                const int iWidth = m_pHandle->width();
                if (iWidth == 0) {
                  qDebug() << "WSliderComposed: Invalid width.";
                  return 0.0;
                }
                const qreal aspect =
                  static_cast<qreal>(m_pHandle->height()) / iWidth;
                return aspect * width();
            }
        }
    }
    return 0;
}
