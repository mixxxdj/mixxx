/***************************************************************************
                          enginechannel.cpp  -  description
                             -------------------
    begin                : Sun Apr 28 2002
    copyright            : (C) 2002 by
    email                :
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "engine/enginechannel.h"

#include "controlobject.h"
#include "controlpushbutton.h"

EngineChannel::EngineChannel(const char* pGroup,
                             EngineChannel::ChannelOrientation defaultOrientation)
        : m_group(pGroup) {
    m_pPFL = new ControlPushButton(ConfigKey(m_group, "pfl"));
    m_pPFL->setButtonMode(ControlPushButton::TOGGLE);
    m_pMaster = new ControlPushButton(ConfigKey(m_group, "master"));
    m_pMaster->setButtonMode(ControlPushButton::TOGGLE);
    m_pOrientation = new ControlObject(ConfigKey(m_group, "orientation"));
    m_pOrientation->set(defaultOrientation);
    m_pOrientationLeft = new ControlPushButton(ConfigKey(m_group, "orientation_left"));
    connect(m_pOrientationLeft, SIGNAL(valueChanged(double)),
            this, SLOT(slotOrientationLeft(double)));
    m_pOrientationRight = new ControlPushButton(ConfigKey(m_group, "orientation_right"));
    connect(m_pOrientationRight, SIGNAL(valueChanged(double)),
            this, SLOT(slotOrientationRight(double)));
    m_pOrientationCenter = new ControlPushButton(ConfigKey(m_group, "orientation_center"));
    connect(m_pOrientationCenter, SIGNAL(valueChanged(double)),
            this, SLOT(slotOrientationCenter(double)));
}

EngineChannel::~EngineChannel() {
    delete m_pMaster;
    delete m_pPFL;
    delete m_pOrientation;
}

const QString& EngineChannel::getGroup() const {
    return m_group;
}

bool EngineChannel::isPFL() {
    return m_pPFL->get() > 0.0;
}

bool EngineChannel::isMaster() {
    return m_pMaster->get() > 0.0;
}

void EngineChannel::slotOrientationLeft(double v) {
    if (v > 0) {
        m_pOrientation->set(LEFT);
    }
}

void EngineChannel::slotOrientationRight(double v) {
    if (v > 0) {
        m_pOrientation->set(RIGHT);
    }
}

void EngineChannel::slotOrientationCenter(double v) {
    if (v > 0) {
        m_pOrientation->set(CENTER);
    }
}

EngineChannel::ChannelOrientation EngineChannel::getOrientation() {
    double dOrientation = m_pOrientation->get();
    if (dOrientation == LEFT) {
        return LEFT;
    } else if (dOrientation == CENTER) {
        return CENTER;
    } else if (dOrientation == RIGHT) {
        return RIGHT;
    }
    return CENTER;
}
