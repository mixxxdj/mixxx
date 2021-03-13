#include "controllers/midi/midibeatclockreceiver.h"

#include "controllers/midi/midimessage.h"

namespace {
constexpr int kPulsesPerQuarterNote = 24;
}

namespace mixxx {

MidiBeatClockReceiver::MidiBeatClockReceiver()
        : m_bpm(Bpm::kValueUndefined),
          m_isPlaying(false),
          m_clockTickIndex(0) {
}

// static
bool MidiBeatClockReceiver::canReceiveMidiStatus(unsigned char status) {
    switch (status) {
    case MidiOpCode::MIDI_START:
    case MidiOpCode::MIDI_STOP:
    case MidiOpCode::MIDI_TIMING_CLK:
        return true;
    default:
        return false;
    }
}

void MidiBeatClockReceiver::receive(unsigned char status, Duration timestamp) {
    switch (status) {
    case MidiOpCode::MIDI_START:
        m_clockTickIndex = 0;
        m_isPlaying = true;
        break;
    case MidiOpCode::MIDI_STOP:
        m_isPlaying = false;
        break;
    case MidiOpCode::MIDI_TIMING_CLK:
        if (m_lastTimestamp != Duration::empty() && timestamp != Duration::empty()) {
            Duration interval = timestamp - m_lastTimestamp;
            m_bpm = Bpm(1000000000.0 / interval.toDoubleNanos() / kPulsesPerQuarterNote * 60.0);
        }
        m_clockTickIndex = (m_clockTickIndex + 1) % kPulsesPerQuarterNote;
        m_lastTimestamp = timestamp;
        break;
    default:
        DEBUG_ASSERT(!"Unhandled message type");
    }
};

double MidiBeatClockReceiver::beatDistance() const {
    return 1.0 - (static_cast<double>(m_clockTickIndex) / kPulsesPerQuarterNote);
}

} // namespace mixxx
