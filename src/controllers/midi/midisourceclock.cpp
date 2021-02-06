#include "controllers/midi/midisourceclock.h"

#include "controllers/midi/midimessage.h"
#include "util/math.h"

bool MidiSourceClock::handleMessage(unsigned char status,
        const mixxx::Duration& timestamp) {
    // TODO(owen): We need to support MIDI_CONTINUE.
    switch (status) {
    case MIDI_START:
        start();
        return true;
    case MIDI_STOP:
        stop();
        return true;
    case MIDI_TIMING_CLK:
        pulse(timestamp);
        return true;
    default:
        return false;
    }
}

void MidiSourceClock::start() {
    // Treating MIDI_START as the first downbeat is standard practice:
    // http://www.blitter.com/%7Erusstopia/MIDI/%7Ejglatt/tech/midispec/seq.htm
    m_bRunning = true;
    m_iFilled = 0;
    m_iRingBufferPos = 0;
}

void MidiSourceClock::stop() {
    m_bRunning = false;
}

void MidiSourceClock::pulse(const mixxx::Duration& timestamp) {
    // Update the ring buffer and calculate new bpm.  Update the last beat time
    // if we are on a beat.

    if (!m_bRunning) {
        qDebug()
                << "MidiSourceClock: Got clock pulse but not started, starting now.";
        start();
    }

    // Ringbuffer filling.
    // TODO(owen): We should have a ringbuffer convenience class.
    m_pulseRingBuffer[m_iRingBufferPos] = timestamp;
    m_iRingBufferPos = (m_iRingBufferPos + 1) % kRingBufferSize;
    if (m_iFilled < kRingBufferSize) {
        ++m_iFilled;
    }

    // If this pulse is a beat mark, record it, even if we have very few samples.
    if (m_iRingBufferPos % kPulsesPerQuarter == 0) {
        QMutexLocker lock(&m_mutex);
        if (m_dBpm != 0.0 && m_lastBeatTime.toIntegerNanos() != 0) {
            // Calculate the smoothed last beat time from the current bpm
            // and the actual last beat time.  By not using the last smoothed
            // time we prevent drift.
            const double beat_length = 60.0 * 1e9 / m_dBpm;
            const auto beat_duration = mixxx::Duration::fromNanos(static_cast<qint64>(beat_length));
            m_smoothedBeatTime = m_lastBeatTime + beat_duration;
        } else {
            m_smoothedBeatTime = timestamp;
        }
        m_lastBeatTime = timestamp;
    }

    // Figure out the bpm if we have enough samples.
    if (m_iFilled > 2) {
        mixxx::Duration earlyPulseTime;
        if (m_iFilled < kRingBufferSize) {
            earlyPulseTime = m_pulseRingBuffer[0];
        } else {
            // In a filled ring buffer, the earliest pulse is the next one that
            // will get filled.
            earlyPulseTime = m_pulseRingBuffer[m_iRingBufferPos];
        }
        QMutexLocker lock(&m_mutex);
        m_dBpm = calcBpm(earlyPulseTime, timestamp, m_iFilled);
    }
}

// static
double MidiSourceClock::calcBpm(const mixxx::Duration& early_pulse,
        const mixxx::Duration& late_pulse,
        int pulse_count) {
    // Get the elapsed time between the latest pulse and the earliest pulse
    // and divide by the number of pulses in the buffer to get bpm.  Midi
    // clock information is by nature imprecise, and issues such as drift and
    // inability to adapt to abrupt tempo changes are well known.  We can not
    // expect to wring more precision out of an imprecise standard.

    // If we have too few samples, we can't calculate a bpm, so return 0.0.
    VERIFY_OR_DEBUG_ASSERT(pulse_count >= 2) {
        qWarning() << "MidiSourceClock::calcBpm called with too few pulses";
        return 0.0;
    }

    VERIFY_OR_DEBUG_ASSERT(late_pulse >= early_pulse) {
        qWarning() << "MidiSourceClock asked to calculate beat fraction but "
                   << "late_pulse < early_pulse:" << late_pulse << early_pulse;
        return 0.0;
    }

    const mixxx::Duration elapsed = late_pulse - early_pulse;
    const double elapsed_mins = elapsed.toDoubleSeconds() / 60.0;

    // We subtract one since two time values denote a single span of time --
    // so a filled value of 3 indicates 2 pulse periods, etc.
    const double bpm = static_cast<double>(pulse_count - 1) / kPulsesPerQuarter / elapsed_mins;

    if (bpm < kMinMidiBpm || bpm > kMaxMidiBpm) {
        qWarning() << "MidiSourceClock bpm out of range, returning 0:" << bpm;
        return 0;
    }
    return bpm;
}

// static
double MidiSourceClock::beatFraction(const mixxx::Duration& last_beat,
        const mixxx::Duration& now,
        const double bpm) {
    VERIFY_OR_DEBUG_ASSERT(now >= last_beat) {
        qWarning() << "MidiSourceClock asked to calculate beat fraction but "
                   << "now < last_beat:" << now << last_beat;
        return 0.0;
    }
    if (bpm == 0.0) {
        return 0.0;
    }
    // Get seconds per beat.
    const double beat_length = 60.0 / bpm;
    // seconds / secondsperbeat = fraction of beat.
    const mixxx::Duration beat_duration = now - last_beat;
    const double beat_percent = beat_duration.toDoubleSeconds() / beat_length;
    // Ensure values are < 1.0.
    return beat_percent - floor(beat_percent);
}
