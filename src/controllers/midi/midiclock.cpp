#include "controllers/midi/midiclock.h"
#include "controllers/midi/midimessage.h"
#include "util/math.h"

bool MidiClock::handleMessage(unsigned char status) {
    switch (status) {
    case MIDI_START:
        start();
        return true;
    case MIDI_STOP:
        stop();
        return true;
    case MIDI_TIMING_CLK:
        tick();
        return true;
    default:
        return false;
    }
}

void MidiClock::start() {
    m_bRunning = true;
    m_iFilled = 0;
    m_iRingBufferPos = 0;
}

void MidiClock::stop() {
    m_bRunning = false;
}

void MidiClock::tick() {
    // Update the ring buffer and calculate new bpm.  Update the last beat time
    // if we are on a beat.

    if (!m_bRunning) {
        qDebug() << "MidiClock: Got clock tick but not started, starting now.";
        start();
    }

    // Ringbuffer filling.
    const qint64 lastTickTime = m_pClock->now();
    m_iTickRingBuffer[m_iRingBufferPos] = lastTickTime;
    m_iRingBufferPos = (m_iRingBufferPos + 1) % kRingBufferSize;
    if (m_iFilled < kRingBufferSize) {
        ++m_iFilled;
    }

    // If this tick is a beat mark, record it, even if we have very few samples.
    if (m_iRingBufferPos % kPulsesPerQuarter == 0) {
        m_iLastBeatTime = lastTickTime;
    }

    // Figure out the bpm if we have enough samples.
    if (m_iFilled > 2) {
        qint64 earlyTickTime = 0;
        if (m_iFilled < kRingBufferSize) {
            earlyTickTime = m_iTickRingBuffer[0];
        } else {
            // In a filled ring buffer, the earliest tick is the next one that
            // will get filled.
            earlyTickTime = m_iTickRingBuffer[m_iRingBufferPos];
        }
        m_dBpm = calcBpm(earlyTickTime, lastTickTime, m_iFilled);
    }
}

// static
double MidiClock::calcBpm(
        qint64 early_tick, qint64 late_tick, int tick_count) {
    // Get the elapsed time between the latest tick and the earliest tick
    // and divide by the number of ticks in the buffer to get bpm.

    // If we have too few samples, we can't calculate a bpm, so return the
    // last-known value.
    DEBUG_ASSERT_AND_HANDLE(tick_count >= 2) {
        qWarning() << "MidiClock::calcBpm called with too few ticks";
        return 0.0;
    }

    DEBUG_ASSERT_AND_HANDLE(late_tick >= early_tick) {
        qWarning() << "MidiClock asked to calculate beat percentage but "
                   << "late_tick < early_tick:"
                   << late_tick << early_tick;
        return 0.0;
    }

    const double elapsed_mins =
            static_cast<double>(late_tick - early_tick) / (60.0 * 1e9);

    // We subtract one since two time values denote a single span of time --
    // so a filled value of 3 indicates 2 tick periods, etc.
    return static_cast<double>(tick_count - 1) / kPulsesPerQuarter / elapsed_mins;
}

// static
double MidiClock::beatPercentage(const qint64 last_beat, const qint64 now,
                                 const double bpm) {
    DEBUG_ASSERT_AND_HANDLE(now >= last_beat) {
        qWarning() << "MidiClock asked to calculate beat percentage but "
                << "now < last_beat:"
                << now << last_beat;
        return 0.0;
    }
    // Get seconds per beat.
    const double beat_length = 60.0 / bpm;
    // seconds / secondsperbeat = percentage of beat.
    const double beat_percent =
            static_cast<double>(now - last_beat) / 1e9 / beat_length;
    // Ensure values are < 1.0.
    return beat_percent - floor(beat_percent);
}

double MidiClock::beatPercentage() const {
    return beatPercentage(m_iLastBeatTime, m_pClock->now(), m_dBpm);
}

