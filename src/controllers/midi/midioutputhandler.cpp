#include "controllers/midi/midioutputhandler.h"

#include <QtDebug>

#include "control/controlobject.h"
#include "controllers/controllerdebug.h"
#include "controllers/midi/midicontroller.h"
#include "moc_midioutputhandler.cpp"

MidiOutputHandler::MidiOutputHandler(MidiController* controller,
        const MidiOutputMapping& mapping)
        : m_pController(controller),
          m_mapping(mapping),
          m_cos(mapping.controlKey, this, ControlFlag::NoAssertIfMissing),
          m_lastVal(-1) { // -1 = virgin
    m_cos.connectValueChanged(this, &MidiOutputHandler::controlChanged);
}

MidiOutputHandler::~MidiOutputHandler() {
    ConfigKey cKey = m_cos.getKey();
    controllerDebug(QString("Destroying static MIDI output handler on %1 for %2,%3")
                .arg(m_pController->getName(), cKey.group, cKey.item));
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
        qWarning() << "MIDI device" << m_pController->getName() << "not open for output!";
    } else if (byte3 != 0xFF) {
        controllerDebug("sending MIDI bytes:" << m_mapping.output.status
                     << "," << m_mapping.output.control << ","
                     << byte3);
        m_pController->sendShortMsg(m_mapping.output.status,
                                    m_mapping.output.control, byte3);
        m_lastVal = static_cast<int>(byte3);
    }
}
