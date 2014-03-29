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

MidiOutputHandler::MidiOutputHandler(MidiController* controller,
                                     const MidiOutputMapping& mapping)
        : m_pController(controller),
          m_mapping(mapping),
          m_cot(mapping.control),
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

    unsigned char byte3 = m_mapping.output.off;
    if (value >= m_mapping.output.min && value <= m_mapping.output.max) {
        byte3 = m_mapping.output.on;
    }

    if (!m_pController->isOpen()) {
        qWarning() << "MIDI device" << m_pController->getName() << "not open for output!";
    } else if (byte3 != 0xFF) {
        if (m_pController->debugging()) {
            qDebug() << "sending MIDI bytes:" << m_mapping.output.status
                     << ", " << m_mapping.output.control << ", "
                     << byte3 ;
        }
        m_pController->sendShortMsg(m_mapping.output.status,
                                    m_mapping.output.control, byte3);
    }
}
