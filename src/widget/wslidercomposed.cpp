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

#include "defs.h"
#include "widget/wpixmapstore.h"

WSliderComposed::WSliderComposed(QWidget * parent)
    : WWidget(parent),
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
        QString pathSlider = context.getSkinPath(context.selectString(node, "Slider"));
        setSliderPixmap(pathSlider);
    }

    QString pathHandle = context.getSkinPath(context.selectString(node, "Handle"));
    bool h = context.selectBool(node, "Horizontal", false);
    setHandlePixmap(h, pathHandle);

    if (context.hasNode(node, "EventWhileDrag")) {
        if (context.selectString(node, "EventWhileDrag").contains("no")) {
            m_bEventWhileDrag = false;
        }
    }
}

void WSliderComposed::setSliderPixmap(const QString& filenameSlider) {
    m_pSlider = WPixmapStore::getPaintable(filenameSlider);
    if (!m_pSlider) {
        qDebug() << "WSliderComposed: Error loading slider pixmap:" << filenameSlider;
    } else {
        // Set size of widget, using size of slider pixmap
        setFixedSize(m_pSlider->size());
    }
}

void WSliderComposed::setHandlePixmap(bool bHorizontal, const QString& filenameHandle) {
    m_bHorizontal = bHorizontal;
    m_pHandle = WPixmapStore::getPaintable(filenameHandle);
    if (!m_pHandle) {
        qDebug() << "WSliderComposed: Error loading handle pixmap:" << filenameHandle;
    } else {
        m_iHandleLength = m_bHorizontal ?
                m_pHandle->width() : m_pHandle->height();

        slotConnectedValueChanged(getValue());
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
        setValue(newValue);

        // Emit valueChanged signal
        if (m_bEventWhileDrag) {
            if (e->button() == Qt::RightButton) {
                emit(valueChangedRightUp(newValue));
            } else {
                emit(valueChangedLeftUp(newValue));
            }
        }

        // Update display
        update();
    }
}

void WSliderComposed::wheelEvent(QWheelEvent *e) {
    // For legacy (MIDI) reasons this is tuned to 127.
    double wheelDirection = ((QWheelEvent *)e)->delta() / (120.0 * 127.0);
    double newValue = getValue() + wheelDirection;

    // Clamp to [0.0, 1.0]
    newValue = math_max(0.0, math_min(1.0, newValue));

    updateValue(newValue);

    e->accept();

    //e->ignore();
}

void WSliderComposed::mouseReleaseEvent(QMouseEvent * e) {
    if (!m_bEventWhileDrag) {
        mouseMoveEvent(e);

        if (e->button() == Qt::RightButton) {
            emit(valueChangedRightUp(getValue()));
        } else {
            emit(valueChangedLeftUp(getValue()));
        }

        m_bDrag = false;
    }
    if (e->button() == Qt::RightButton) {
        m_bRightButtonPressed = false;
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
            emit(valueReset());
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

void WSliderComposed::slotConnectedValueChanged(double dValue) {
    if (!m_bDrag && getValue() != dValue) {
        // Set value without emitting a valueChanged signal
        // and force display update
        setValue(dValue);

        // Calculate handle position
        if (!m_bHorizontal) {
            dValue = 1.0 - dValue;
        }
        int sliderLength = m_bHorizontal ? width() : height();
        m_iPos = static_cast<int>(dValue * (sliderLength - m_iHandleLength));

        if (m_iPos > (sliderLength - m_iHandleLength)) {
            m_iPos = sliderLength - m_iHandleLength;
        } else if (m_iPos < 0) {
            m_iPos = 0;
        }
        update();
    }
}
