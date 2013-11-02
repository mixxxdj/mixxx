/***************************************************************************
                          wwidget.cpp  -  description
                             -------------------
    begin                : Wed Jun 18 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
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


#include <QtGui>
#include <QtDebug>

#include "wwidget.h"
#include "controlobject.h"
#include "controlobjectthread.h"
#include "controlobjectthreadwidget.h"


// Static member variable definition
QString WWidget::m_qPath;

WWidget::WWidget(QWidget* parent, Qt::WindowFlags flags)
        : QWidget(parent, flags),
          m_fValue(0.0),
          m_bOff(false),
          m_activeTouchButton(Qt::NoButton) {
    m_pTouchShift = new ControlObjectThread("[Controls]", "touch_shift");
    connect(this, SIGNAL(valueChangedLeftDown(double)), this, SLOT(slotReEmitValueDown(double)));
    connect(this, SIGNAL(valueChangedRightDown(double)), this, SLOT(slotReEmitValueDown(double)));
    connect(this, SIGNAL(valueChangedLeftUp(double)), this, SLOT(slotReEmitValueUp(double)));
    connect(this, SIGNAL(valueChangedRightUp(double)), this, SLOT(slotReEmitValueUp(double)));

    setAttribute(Qt::WA_StaticContents);
    setAttribute(Qt::WA_AcceptTouchEvents);
    setFocusPolicy(Qt::ClickFocus);
    //setBackgroundMode(Qt::NoBackground); //this is deprecated, and commenting it out doesn't seem to change anything -kousu 2009/03
}

WWidget::~WWidget() {
    delete m_pTouchShift;
}

void WWidget::setValue(double fValue) {
    m_fValue = fValue;
    update();
}

void WWidget::setOnOff(double d) {
    if (d==0.)
        m_bOff = false;
    else
        m_bOff = true;

    repaint();
}

void WWidget::slotReEmitValueDown(double fValue) {
    emit(valueChangedDown(fValue));
}

void WWidget::slotReEmitValueUp(double fValue) {
    emit(valueChangedUp(fValue));
}

int WWidget::selectNodeInt(const QDomNode &nodeHeader, const QString sNode) {
    QString text = selectNode(nodeHeader, sNode).toElement().text();
    bool ok;
    int conv = text.toInt(&ok, 0);
    if (ok) {
        return conv;
    } else {
        return 0;
    }
}

float WWidget::selectNodeFloat(const QDomNode &nodeHeader, const QString sNode) {
    return selectNode(nodeHeader, sNode).toElement().text().toFloat();
}

QString WWidget::selectNodeQString(const QDomNode &nodeHeader, const QString sNode) {
    QString ret;
    QDomNode node = selectNode(nodeHeader, sNode);
    if (!node.isNull())
        ret = node.toElement().text();
    else
        ret = "";
    return ret;
}

QDomNode WWidget::selectNode(const QDomNode &nodeHeader, const QString sNode) {
    QDomNode node = nodeHeader.firstChild();
    while (!node.isNull())
    {
        if (node.nodeName() == sNode)
            return node;
        node = node.nextSibling();
    }
    return node;
}

const QString WWidget::getPath(QString location) {
    QString l(location);
    return l.prepend(m_qPath);
}

void WWidget::setPixmapPath(QString qPath) {
    m_qPath = qPath;
}

double WWidget::getValue() {
   return m_fValue;
}

void WWidget::updateValue(double fValue) {
    setValue(fValue);
    emit(valueChangedUp(fValue));
    emit(valueChangedDown(fValue));
}

bool WWidget::touchIsRightButton() {
    return (m_pTouchShift->get() != 0.0);
}

bool WWidget::event(QEvent* e) {
    if (isEnabled()) {
        switch(e->type()) {
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
        case QEvent::TouchEnd:
        {
            QTouchEvent* touchEvent = static_cast<QTouchEvent*>(e);
            if (touchEvent->deviceType() !=  QTouchEvent::TouchScreen) {
                break;
            }

            // fake a mouse event!
            QEvent::Type eventType = QEvent::None;
            switch (touchEvent->type()) {
            case QEvent::TouchBegin:
                eventType = QEvent::MouseButtonPress;
                if (touchIsRightButton()) {
                    // touch is right click
                    m_activeTouchButton = Qt::RightButton;
                } else {
                    m_activeTouchButton = Qt::LeftButton;
                }
                break;
            case QEvent::TouchUpdate:
                eventType = QEvent::MouseMove;
                break;
            case QEvent::TouchEnd:
                eventType = QEvent::MouseButtonRelease;
                break;
            default:
                Q_ASSERT(!true);
                break;
            }

            const QTouchEvent::TouchPoint &touchPoint =
                    touchEvent->touchPoints().first();
            QMouseEvent mouseEvent(eventType,
                    touchPoint.pos().toPoint(),
                    touchPoint.screenPos().toPoint(),
                    m_activeTouchButton, // Button that causes the event
                    Qt::NoButton, // Not used, so no need to fake a proper value.
                    touchEvent->modifiers());

            return QWidget::event(&mouseEvent);
        }
        default:
            break;
        }
    }

    return QWidget::event(e);
}