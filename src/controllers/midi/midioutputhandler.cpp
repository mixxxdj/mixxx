#include "controllers/midi/midioutputhandler.h"

#include <QtDebug>

#include "controllers/midi/midicontroller.h"
#include "moc_midioutputhandler.cpp"

MidiOutputHandler::MidiOutputHandler(MidiController* controller,
        const MidiOutputMapping& mapping,
        const RuntimeLoggingCategory& logger)
        : m_pController(controller),
          m_mapping(mapping),
          m_cos(mapping.controlKey, this, ControlFlag::NoAssertIfMissing),
          m_lastVal(-1), // arbitrary invalid MIDI value
          m_logger(logger) {
    m_cos.connectValueChanged(this, &MidiOutputHandler::controlChanged);
}

MidiOutputHandler::~MidiOutputHandler() {
    ConfigKey cKey = m_cos.getKey();
    qCDebug(m_logger) << QString("Destroying static MIDI output handler on %1 for %2,%3")
                                 .arg(m_pController->getName(), cKey.group, cKey.item);
}

bool MidiOutputHandler::validate() {
    return m_cos.valid();
}

void MidiOutputHandler::update() {
    controlChanged(m_cos.get());
}

void MidiOutputHandler::controlChanged(double value) {
    // Don't update with out of date messages.
    value = m_cos.get();

    unsigned char byte3 = m_mapping.output.off;
    if (value >= m_mapping.output.min && value <= m_mapping.output.max) {
        byte3 = m_mapping.output.on;
    }

    if (static_cast<int>(byte3) == m_lastVal) {
        // Don't send redundant messages.
        return;
    }

    if (!m_pController->isOpen()) {
        qCWarning(m_logger) << "MIDI device" << m_pController->getName() << "not open for output!";
    } else if (byte3 != 0xFF) {
        qCDebug(m_logger) << "sending MIDI bytes:" << m_mapping.output.status
                          << "," << m_mapping.output.control << ","
                          << byte3;
        m_pController->sendShortMsg(m_mapping.output.status,
                                    m_mapping.output.control, byte3);
        m_lastVal = static_cast<int>(byte3);
    }
}
