#ifndef MIDISOURCECLOCK_H
#define MIDISOURCECLOCK_H

#include <QList>

#include "util/time.h"

// Stub out the Time::elapsed function for easier testing.
class MockableClock {
public:
    virtual ~MockableClock() {
    }
    ;
    virtual qint64 now() = 0;
};

class WallClock: public MockableClock {
public:
    virtual qint64 now() {
        return Time::elapsed();
    }
};

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
    // Injectable clock for testing.  Does not take ownership of the clock.
    MidiSourceClock(MockableClock* clock)
            : m_pClock(clock) {
    }

    // Handle an incoming midi status.  Return true if handled.
    bool handleMessage(unsigned char status);

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
    void pulse();

    // Return the current BPM.  Values are significant to 5 decimal places.
    double bpm() const {
        return m_dBpm;
    }

    // Return the time of the last beat;
    qint64 lastBeatTime() const {
        return m_iLastBeatTime;
    }

    // Calculate instantaneous beat fraction based on provided values.  If
    // the beat fraction is >= 1.0, the integer value will be sliced off until
    // the result is between 0 <= x < 1.0.  Can be called from any thread
    // since it's static.
    static double beatFraction(const qint64 last_beat, const qint64 now,
                               const double bpm);

    // Convenience function for callers that have access to the MidiSourceClock
    // object.  Returns the instantaneous beat fraction.  This should only be
    // called from the same thread that makes calls to pulse().
    double beatFraction() const;

    // Returns true if the clock is running.  A master sync listener should
    // always call this to make sure that the beatfraction and bpm are
    // valid.
    bool running() const {
        return m_bRunning;
    }

private:
    // Calculate the bpm based on the pulse times and counts.  Returns values
    // between the min and max allowable bpm, or 0.0 for error conditions.
    static double calcBpm(qint64 early_pulse, qint64 late_pulse,
                          int pulse_count);

    bool m_bRunning = false;
    // It's a hack to say 124 all over the source, but it provides a sane
    // baseline in case the midi device is already running when Mixxx starts up.
    double m_dBpm = 124.0;
    qint64 m_iLastBeatTime = 0;

    MockableClock* m_pClock;

    qint64 m_iPulseRingBuffer[kRingBufferSize];
    int m_iRingBufferPos = 0;
    int m_iFilled = 0;
};

#endif  // MIDISOURCECLOCK_H
