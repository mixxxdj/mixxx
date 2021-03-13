#include "controllers/midi/midibeatclockreceiver.h"

#include "controllers/midi/midimessage.h"

namespace {
constexpr int kPulsesPerQuarterNote = 24;
}

namespace mixxx {

MidiBeatClockReceiver::MidiBeatClockReceiver()
        : m_bpm(Bpm::kValueUndefined),
          m_isPlaying(false) {
}

void MidiBeatClockReceiver::receive(unsigned char status, Duration timestamp) {
    switch (status) {
    case MidiOpCode::MIDI_START:
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
        m_lastTimestamp = timestamp;
        break;
    default:
        DEBUG_ASSERT(!"Unhandled message type");
    }
};

} // namespace mixxx
