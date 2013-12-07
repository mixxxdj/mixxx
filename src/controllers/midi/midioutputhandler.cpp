/**
 * @file midioutputhandler.cpp
 * @author Sean Pappalardo spappalardo@mixxx.org
 * @date Tue 11 Feb 2012
 * @brief MIDI output mapping handler
 *
 */

#include <QtDebug>

#include "controllers/midi/midioutputhandler.h"
#include "controllers/midi/midicontroller.h"
#include "controlobject.h"

MidiOutputHandler::MidiOutputHandler(const QString& group, const QString& key,
                                     MidiController *controller,
                                     float min, float max,
                                     unsigned char status, unsigned char midino,
                                     unsigned char on, unsigned char off)
        : m_pController(controller),
          m_cot(group, key),
          m_min(min),
          m_max(max),
          m_status(status),
          m_midino(midino),
          m_on(on),
          m_off(off),
          m_lastVal(0) {
    connect(&m_cot, SIGNAL(valueChanged(double)),
            this, SLOT(controlChanged(double)));
}

MidiOutputHandler::~MidiOutputHandler() {
    ConfigKey cKey = m_cot.getKey();
    if (m_pController->debugging()) {
        qDebug() << QString("Destroying static MIDI output handler on %1 for %2,%3")
                .arg(m_pController->getName(), cKey.group, cKey.item);
    }
}

bool MidiOutputHandler::validate() {
    return m_cot.valid();
}

void MidiOutputHandler::update() {
    controlChanged(m_cot.get());
}

void MidiOutputHandler::controlChanged(double value) {
    // Don't send redundant messages.
    if (value == m_lastVal) {
        return;
    }

    // Don't update with out of date messages.
    value = m_cot.get();
    m_lastVal = value;

    unsigned char byte3 = m_off;
    if (value >= m_min && value <= m_max) { byte3 = m_on; }

    if (!m_pController->isOpen()) {
        qWarning() << "MIDI device" << m_pController->getName() << "not open for output!";
    } else if (byte3 != 0xFF) {
        if (m_pController->debugging()) {
            qDebug() << "sending MIDI bytes:" << m_status << ", " << m_midino << ", "
                     << byte3 ;
        }
        m_pController->sendShortMsg(m_status, m_midino, byte3);
    }
}
