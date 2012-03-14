/**
* @file midioutputhandler.cpp
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Tue 11 Feb 2012
* @brief MIDI output mapping handler
*
*/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "midioutputhandler.h"
#include "midicontroller.h"

#include <QDebug>

MidiOutputHandler::MidiOutputHandler(QString group, QString key,
                                     MidiController *controller,
                                     float min, float max,
                                     unsigned char status, unsigned char midino,
                                     unsigned char on, unsigned char off)
    : m_min(min), m_max(max), m_status(status), m_on(on), m_off(off),
      m_pController(controller), m_controlno(midino) {

    m_cobj = ControlObject::getControl(ConfigKey(group, key));

    if (m_cobj == NULL) {
        QByteArray err_tmp = QString("Invalid config group: '%1', name: '%2'")
                                    .arg(group, key).toAscii();
        Q_ASSERT_X(m_cobj, "MidiOutputHandler", err_tmp);
        qWarning() << "MidiOutputHandler:" << err_tmp;
        return;
    }

    connect(m_cobj, SIGNAL(valueChangedFromEngine(double)), this, SLOT(controlChanged(double)));
    connect(m_cobj, SIGNAL(valueChanged(double)), this, SLOT(controlChanged(double)));
}

MidiOutputHandler::~MidiOutputHandler() {
    ConfigKey cKey = m_cobj->getKey();
    if (m_pController->debugging()) {
        qDebug() << QString("Destroying static MIDI output handler on %1 for %2,%3")
                            .arg(m_pController->getName(), cKey.group, cKey.item);
    }
}

void MidiOutputHandler::update() {
    controlChanged(m_cobj->get());
}

void MidiOutputHandler::controlChanged(double value) {
    unsigned char byte3 = m_off;
    if (value >= m_min && value <= m_max) { byte3 = m_on; }

    if (!m_pController->isOpen())
        qWarning() << "MIDI device" << m_pController->getName() << "not open for output!";
    else if (byte3 != 0xff) {
//         qDebug() << "MIDI bytes:" << m_status << ", " << m_controllerno << ", " << m_byte2 ;
        m_pController->sendShortMsg(m_status, m_controlno, byte3);
    }
}

