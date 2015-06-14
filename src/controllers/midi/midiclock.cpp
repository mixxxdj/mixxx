#include "controllers/midi/midiclock.h"
#include "util/math.h"

void MidiClock::start() {
    m_bRunning = true;
    m_iFilled = 0;
    m_iRingBufferPos = 0;
}

void MidiClock::stop() {
    m_bRunning = false;
    // if we stop and don't clear the buffer, ticks will be way off.
    // but also, if we start over the bpm will be weird at the begging, right?
    // maybe that's a TODO for now
    // maybe use previous bpm to inform new bpm??
}

void MidiClock::tick() {
    // Update the ring buffer and calculate new bpm.  Update the last beat time
    // if we are on a beat.

    if (!m_bRunning) {
        qDebug() << "MidiClock: Got clock tick but not started, starting now.";
        start();
    }

    const qint64 lastTickTime = m_pClock->now();
    m_iTickRingBuffer[m_iRingBufferPos] = lastTickTime;
    m_iRingBufferPos = (m_iRingBufferPos + 1) % kRingBufferSize;
    // math_max doesn't work here because it can't see the constant.
    if (m_iFilled < kRingBufferSize) {
        ++m_iFilled;
    }

    // If this tick is a beat mark, record it, even if we have very few samples.
    if (m_iRingBufferPos % 24 == 0) {
        m_iLastBeatTime = lastTickTime;
    }

    if (m_iFilled < 2) {
        return;
    }
    qint64 earlyTickTime = 0;
    if (m_iFilled < kRingBufferSize) {
        earlyTickTime = m_iTickRingBuffer[0];
    } else {
        // In a filled ring buffer, the earliest tick is the next one that will
        // get filled.
        earlyTickTime = m_iTickRingBuffer[m_iRingBufferPos];
    }

    // Figure out the bpm
    m_aiBpm = static_cast<int>(
            calcBpm(earlyTickTime, lastTickTime) * kFixedPrecision);
}

double MidiClock::calcBpm(qint64 early_tick, qint64 late_tick) const {
    // Get the elapsed time between the latest tick and the earliest tick
    // and divide by the number of ticks in the buffer to get bpm.
    if (m_iFilled < 2) {
        return static_cast<double>(m_aiBpm) / kFixedPrecision;
    }
    const double elapsed_mins =
            static_cast<double>(late_tick - early_tick) / (60.0 * 1e9);

    return static_cast<double>(m_iFilled - 1) / 24.0 / elapsed_mins;
}

double MidiClock::bpm() const {
    return static_cast<double>(m_aiBpm) / kFixedPrecision;
}

double MidiClock::beatPercentage() const {
    if (!m_bRunning) {
        // If we aren't running we can't produce a reliable percentage.
        return 0.0;
    }

    // Due to threading, last beat time and bpm may be based on different
    // values, but that shouldn't cause large amounts of error.

    const qint64 last_beat = m_iLastBeatTime;
    const qint64 now = m_pClock->now();

    // 60 seconds per bpm = seconds per beat.
    const double beat_length =
            60.0 / (static_cast<double>(m_aiBpm) / kFixedPrecision);
    // seconds / secondsperbeat = percentage of beat.
    return static_cast<double>(now - last_beat) / 1e9 / beat_length;
}
