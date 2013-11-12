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

#include "wslidercomposed.h"
#include <qpixmap.h>
#include <QtDebug>
#include <qpainter.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include "defs.h"
#include "wpixmapstore.h"

WSliderComposed::WSliderComposed(QWidget * parent)
    : WAbstractControl(parent),
      m_iPos(0),
      m_iStartHandlePos(0),
      m_iStartMousePos(0),
      m_iSliderLength(0),
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

void WSliderComposed::setup(QDomNode node) {
    // Setup pixmaps
    QString pathSlider = getPath(selectNodeQString(node, "Slider"));
    QString pathHandle = getPath(selectNodeQString(node, "Handle"));
    QString pathHorizontal = selectNodeQString(node, "Horizontal");
    bool h = false;
    if (pathHorizontal.contains("true",Qt::CaseInsensitive)) {
        h = true;
    }
    setPixmaps(h, pathSlider, pathHandle);

    if (!selectNode(node, "EventWhileDrag").isNull()) {
        if (selectNodeQString(node, "EventWhileDrag").contains("no")) {
            m_bEventWhileDrag = false;
        }
    }
}

void WSliderComposed::setPixmaps(bool bHorizontal, const QString &filenameSlider, const QString &filenameHandle) {
    m_bHorizontal = bHorizontal;
    unsetPixmaps();
    m_pSlider = WPixmapStore::getPixmap(filenameSlider);
    if (!m_pSlider) {
        qDebug() << "WSliderComposed: Error loading slider pixmap:" << filenameSlider;
    }
    m_pHandle = WPixmapStore::getPixmap(filenameHandle);
    if (!m_pHandle) {
        qDebug() << "WSliderComposed: Error loading handle pixmap:" << filenameHandle;
    }

    if (m_pSlider && m_pHandle) {
        if (m_bHorizontal) {
            m_iSliderLength = m_pSlider->width();
            m_iHandleLength = m_pHandle->width();
        } else {
            m_iSliderLength = m_pSlider->height();
            m_iHandleLength = m_pHandle->height();
        }

        // Set size of widget, using size of slider pixmap
        if (m_pSlider) {
            setFixedSize(m_pSlider->size());
        }

        setValue(m_fValue);

        repaint();
    }
}

void WSliderComposed::unsetPixmaps() {
    if (m_pSlider) {
        WPixmapStore::deletePixmap(m_pSlider);
        m_pSlider = NULL;
    }
    if (m_pHandle) {
        WPixmapStore::deletePixmap(m_pHandle);
        m_pHandle = NULL;
    }
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

        if (m_iPos > (m_iSliderLength - m_iHandleLength)) {
            m_iPos = m_iSliderLength - m_iHandleLength;
        } else if (m_iPos < 0) {
            m_iPos = 0;
        }

        // value ranges from 0 to 127
        m_fValue = (double)m_iPos * (127. / (double)(m_iSliderLength - m_iHandleLength));
        if (!m_bHorizontal) {
            m_fValue = 127. - m_fValue;
        }

        // Emit valueChanged signal
        if (m_bEventWhileDrag) {
            if (e->button() == Qt::RightButton) {
                emit(valueChangedRightUp(m_fValue));
            } else {
                emit(valueChangedLeftUp(m_fValue));
            }
        }

        // Update display
        update();
    }
}

void WSliderComposed::wheelEvent(QWheelEvent *e) {
    double wheelDirection = ((QWheelEvent *)e)->delta() / 120.;
    double newValue = getValue() + (wheelDirection);
    this->updateValue(newValue);

    e->accept();

    //e->ignore();
}

void WSliderComposed::mouseReleaseEvent(QMouseEvent * e) {
    if (!m_bEventWhileDrag) {
        mouseMoveEvent(e);

        if (e->button() == Qt::RightButton) {
            emit(valueChangedRightUp(m_fValue));
        } else {
            emit(valueChangedLeftUp(m_fValue));
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
    if (m_pSlider && m_pHandle) {
        QPainter p(this);
        int posx;
        int posy;
        if (m_bHorizontal) {
            posx = m_iPos;
            posy = 0;
        } else {
            posx = 0;
            posy = m_iPos;
        }

        // Draw slider followed by handle
        p.drawPixmap(0, 0, *m_pSlider);
        p.drawPixmap(posx, posy, *m_pHandle);
    }
}

void WSliderComposed::setValue(double fValue) {
    if (!m_bDrag && m_fValue != fValue) {
        // Set value without emitting a valueChanged signal
        // and force display update
        m_fValue = fValue;

        // Calculate handle position
        if (!m_bHorizontal) {
            fValue = 127-fValue;
        }
        m_iPos = (int)((fValue / 127.) * (double)(m_iSliderLength - m_iHandleLength));

        if (m_iPos > (m_iSliderLength - m_iHandleLength)) {
            m_iPos = m_iSliderLength - m_iHandleLength;
        } else if (m_iPos < 0) {
            m_iPos = 0;
        }
        update();
    }
}
