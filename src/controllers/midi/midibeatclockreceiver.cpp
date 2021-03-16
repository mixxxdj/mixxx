#include "controllers/midi/midibeatclockreceiver.h"

#include "controllers/midi/midimessage.h"

namespace mixxx {

MidiBeatClockReceiver::MidiBeatClockReceiver()
        : m_bpm(Bpm(Bpm::kValueUndefined)),
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
        m_clockTickIndex.store(0);
        m_isPlaying.store(true);
        break;
    case MidiOpCode::MIDI_STOP:
        m_isPlaying.store(false);
        break;
    case MidiOpCode::MIDI_TIMING_CLK: {
        const int index = m_clockTickIndex.load();
        if (m_lastTimestamp != Duration::empty() && timestamp != Duration::empty()) {
            m_intervalRingBuffer[m_clockTickIndex] = timestamp - m_lastTimestamp;

            int numValues = 0;
            Duration sumIntervals;
            for (int i = 0; i < kPulsesPerQuarterNote; i++) {
                if (m_intervalRingBuffer[i] != Duration::empty()) {
                    sumIntervals += m_intervalRingBuffer[i];
                    numValues++;
                }
            }

            if (numValues > 0) {
                const Bpm bpm = Bpm(1000000000.0 /
                        (sumIntervals.toDoubleNanos() / numValues) /
                        kPulsesPerQuarterNote * 60.0);
                m_bpm.store(bpm);
            }
        }
        m_clockTickIndex.store((index + 1) % kPulsesPerQuarterNote);
        m_lastTimestamp = timestamp;
        break;
    }
    default:
        DEBUG_ASSERT(!"Unhandled message type");
    }
};

double MidiBeatClockReceiver::beatDistance() const {
    return 1.0 - (static_cast<double>(m_clockTickIndex) / kPulsesPerQuarterNote);
}

} // namespace mixxx
