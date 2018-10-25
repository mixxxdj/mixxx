/***************************************************************************
                          wknob.cpp  -  description
                             -------------------
    begin                : Fri Jun 21 2002
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

#include <QtDebug>
#include <QMouseEvent>
#include <QWheelEvent>

#include "util/time.h"
#include "widget/wknob.h"

WKnob::WKnob(QWidget* pParent)
        : WDisplay(pParent),
          m_guiTickTimer(this) {
    connect(&m_guiTickTimer, SIGNAL(timeout()),
            this, SLOT(guiTick()));
}

void WKnob::mouseMoveEvent(QMouseEvent* e) {
    m_handler.mouseMoveEvent(this, e);
}

void WKnob::mousePressEvent(QMouseEvent* e) {
    m_handler.mousePressEvent(this, e);
}

void WKnob::mouseReleaseEvent(QMouseEvent* e) {
    m_handler.mouseReleaseEvent(this, e);
}

void WKnob::wheelEvent(QWheelEvent* e) {
    m_handler.wheelEvent(this, e);
}

void WKnob::guiTick() {
    mixxx::Duration now = mixxx::Time::elapsed();
    if (now - m_lastActivity > mixxx::Duration::fromSeconds(1)) {
        m_guiTickTimer.stop();
    }
    if (m_lastActivity > m_lastRender) {
        update();
        m_lastRender = m_lastActivity;
    }
}

void WKnob::inputActivity() {
    // Bug #1793015: Previously we would simply call QWidget::update in response
    // to input events that required re-rendering widgets, relying on Qt to
    // batch them together and deliver them at a reasonable frequency. On macOS,
    // the behavior of QWidget::update seems to have changed such that render
    // events happen much more frequently than they used to. To address this, we
    // instead use a downsampling timer attached to the VSyncThread's render
    // ticks for the waveform renderers. The timer invokes guiTick(), which is
    // responsible for actually calling QWidget::update(). When input arrives,
    // we call inputActivity to attach the timer. After 1 second of inactivity,
    // we disconnect the timer.
    m_lastActivity = mixxx::Time::elapsed();
    if (!m_guiTickTimer.isActive()) {
        m_guiTickTimer.start(mixxx::Duration::fromMillis(20));
    }
}
