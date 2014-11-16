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

EngineChannel::EngineChannel(QString pGroup,
                             EngineChannel::ChannelOrientation defaultOrientation)
        : m_group(pGroup) {
    m_pPFL = new ControlPushButton(ConfigKey(m_group, "pfl"));
    m_pPFL->setButtonMode(ControlPushButton::TOGGLE);
    m_pMaster = new ControlPushButton(ConfigKey(m_group, "master"));
    m_pMaster->setButtonMode(ControlPushButton::TOGGLE);
    m_pOrientation = new ControlPushButton(ConfigKey(m_group, "orientation"));
    m_pOrientation->setButtonMode(ControlPushButton::TOGGLE);
    m_pOrientation->setStates(3);
    m_pOrientation->set(defaultOrientation);
    m_pOrientationLeft = new ControlPushButton(ConfigKey(m_group, "orientation_left"));
    connect(m_pOrientationLeft, SIGNAL(valueChanged(double)),
            this, SLOT(slotOrientationLeft(double)), Qt::DirectConnection);
    m_pOrientationRight = new ControlPushButton(ConfigKey(m_group, "orientation_right"));
    connect(m_pOrientationRight, SIGNAL(valueChanged(double)),
            this, SLOT(slotOrientationRight(double)), Qt::DirectConnection);
    m_pOrientationCenter = new ControlPushButton(ConfigKey(m_group, "orientation_center"));
    connect(m_pOrientationCenter, SIGNAL(valueChanged(double)),
            this, SLOT(slotOrientationCenter(double)), Qt::DirectConnection);
    m_pTalkover = new ControlPushButton(ConfigKey(pGroup, "talkover"));
    m_pTalkover->setButtonMode(ControlPushButton::POWERWINDOW);
}

EngineChannel::~EngineChannel() {
    delete m_pMaster;
    delete m_pPFL;
    delete m_pOrientation;
    delete m_pOrientationLeft;
    delete m_pOrientationRight;
    delete m_pOrientationCenter;
    delete m_pTalkover;
}

const QString& EngineChannel::getGroup() const {
    return m_group;
}

void EngineChannel::setPFL(bool enabled) {
    m_pPFL->set(enabled ? 1.0 : 0.0);
}

bool EngineChannel::isPFL() const {
    return m_pPFL->get() > 0.0;
}

void EngineChannel::setMaster(bool enabled) {
    m_pMaster->set(enabled ? 1.0 : 0.0);
}

bool EngineChannel::isMaster() const {
    return m_pMaster->get() > 0.0;
}

void EngineChannel::setTalkover(bool enabled) {
    m_pTalkover->set(enabled ? 1.0 : 0.0);
}

bool EngineChannel::isTalkover() const {
    return m_pTalkover->get() > 0.0;
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

EngineChannel::ChannelOrientation EngineChannel::getOrientation() const {
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
