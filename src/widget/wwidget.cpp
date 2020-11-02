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

#include <QtDebug>
#include <QTouchEvent>

#include "widget/wwidget.h"
#include "control/controlproxy.h"
#include "util/assert.h"

WWidget::WWidget(QWidget* parent, Qt::WindowFlags flags)
        : QWidget(parent, flags),
          WBaseWidget(this),
          m_activeTouchButton(Qt::NoButton),
          m_scaleFactor(1.0) {
    m_pTouchShift = new ControlProxy("[Controls]", "touch_shift");
    setAttribute(Qt::WA_StaticContents);
    setAttribute(Qt::WA_AcceptTouchEvents);
    setFocusPolicy(Qt::ClickFocus);
}

WWidget::~WWidget() {
    delete m_pTouchShift;
}

bool WWidget::touchIsRightButton() {
    return m_pTouchShift->toBool();
}

bool WWidget::event(QEvent* e) {
    if (e->type() == QEvent::ToolTip) {
        updateTooltip();
    } if (e->type() == QEvent::FontChange) {
        const QFont& fonti = font();
        // Change the new font on the fly by casting away its constancy
        // using setFont() here, would results into a recursive loop
        // resetting the font to the original css values.
        // Only scale pixel size fonts, point size fonts are scaled by the OS
        if (fonti.pixelSize() > 0) {
            const_cast<QFont&>(fonti).setPixelSize(
                    static_cast<int>(fonti.pixelSize() * m_scaleFactor));
        }
    } else if (isEnabled()) {
        switch(e->type()) {
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
        case QEvent::TouchEnd:
        {
            QTouchEvent* touchEvent = dynamic_cast<QTouchEvent*>(e);
            if (touchEvent == nullptr || touchEvent->device() == nullptr ||
                    touchEvent->device()->type() !=  QTouchDevice::TouchScreen) {
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
                DEBUG_ASSERT(false);
                break;
            }

            const QTouchEvent::TouchPoint& touchPoint =
                    touchEvent->touchPoints().first();
            QMouseEvent mouseEvent(eventType,
                    touchPoint.pos(),
                    touchPoint.pos(),
                    touchPoint.screenPos(),
                    m_activeTouchButton, // Button that causes the event
                    Qt::NoButton,        // Not used, so no need to fake a proper value.
                    touchEvent->modifiers(),
                    Qt::MouseEventSynthesizedByApplication);

            return QWidget::event(&mouseEvent);
        }
        default:
            break;
        }
    }

    return QWidget::event(e);
}
