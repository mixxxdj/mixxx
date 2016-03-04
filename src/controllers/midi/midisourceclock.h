#ifndef MIDISOURCECLOCK_H
#define MIDISOURCECLOCK_H

#include <QList>
#include <QMutex>

#include "util/duration.h"

// MidiSourceClock is not thread-safe, but is thread compatible using ControlObjects.
// The MIDI thread will make calls into MidiSourceClock and then update two Control
// Objects with the current reported BPM and last beat time.  The engine thread
// can use those values to call a static function to calculate beat fraction
// at any time in the future.  Time values are in nanoseconds for compatibility
// with Time::elapsed().

// TODO(owen): MidiSourceClock needs to support MIDI_CONTINUE.  This is tricky
// because all of our times are absolute and beatFraction information is not
// stored in this class.  Probably the solution is to move beatFraction into
// this class and move away from storing absolute timestamps in the ringbuffer.
class MidiSourceClock {
public:
    // The number of midi pulses per quarter note (1 beat in 4/4 time).
    static constexpr int kPulsesPerQuarter = 24;
    // Minimum allowable calculated bpm.  The bpm can still be reported as 0.0
    // if there is no incoming data or if there is a problem with the
    // calculation.
    static constexpr double kMinMidiBpm = 10.0;
    static constexpr double kMaxMidiBpm = 300.0;

private:
    // Some of the tests use the ring buffer size, so keep those test in sync
    // with this constant.
    static const int kRingBufferSize = kPulsesPerQuarter * 4;

public:
    MidiSourceClock() {}

    // Handle an incoming midi status.  Return true if handled.
    bool handleMessage(unsigned char status,
                       const mixxx::Duration& timestamp);

    // Signals MIDI Start Sequence.  The MidiSourceClock will reset its beat
    // fraction to 0, but the bpm value will be seeded with the last recorded
    // value.
    void start();

    // Signals MIDI Stop Sequence.  The MidiSourceClock will stop updating its beat
    // precentage.  Subsequent calls to beatFraction will return valid results
    // based on the last recorded beat time and last reported bpm.
    void stop();

    // Signals MIDI Timing Clock.  The timing between pulses will be used to
    // determine bpm.  kPulsesPerQuarter pulses = 1 beat.
    void pulse(const mixxx::Duration& timestamp);

    // Return the current BPM.  Values are significant to 5 decimal places.
    double bpm() const {
        QMutexLocker lock(&m_mutex);
        return m_dBpm;
    }

    // Return the exact recorded time of the last beat pulse.
    mixxx::Duration lastBeatTime() const {
        QMutexLocker lock(&m_mutex);
        return m_lastBeatTime;
    }

    // Return a smoothed beat time interpolated from received data.
    mixxx::Duration smoothedBeatTime() const {
        QMutexLocker lock(&m_mutex);
        return m_smoothedBeatTime;
    }

    // Calculate instantaneous beat fraction based on provided values.  If
    // the beat fraction is >= 1.0, the integer value will be sliced off until
    // the result is between 0 <= x < 1.0.  Can be called from any thread
    // since it's static.
    static double beatFraction(const mixxx::Duration& last_beat,
                               const mixxx::Duration& now,
                               const double bpm);

    // Returns true if the clock is running.  A master sync listener should
    // always call this to make sure that the beatfraction and bpm are
    // valid.
    bool running() const {
        return m_bRunning;
    }

private:
    // Calculate the bpm based on the pulse times and counts.  Returns values
    // between the min and max allowable bpm, or 0.0 for error conditions.
    static double calcBpm(const mixxx::Duration& early_pulse,
                          const mixxx::Duration& late_pulse,
                          int pulse_count);

    bool m_bRunning = false;
    // It's a hack to say 124 all over the source, but it provides a sane
    // baseline in case the midi device is already running when Mixxx starts up.
    double m_dBpm = 124.0;
    // Reported time of the last beat
    mixxx::Duration m_lastBeatTime;
    // De-jittered time of the last beat
    mixxx::Duration m_smoothedBeatTime;
    // Mutex for accessing bpm and last beat time for thread safety.
    mutable QMutex m_mutex;

    mixxx::Duration m_pulseRingBuffer[kRingBufferSize];
    int m_iRingBufferPos = 0;
    int m_iFilled = 0;
};

#endif  // MIDISOURCECLOCK_H
