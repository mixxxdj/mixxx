#ifndef MIDICLOCK_H
#define MIDICLOCK_H

#include <QList>

#include "util/time.h"

// Stub out the Time::elapsed function for easier testing.
class MockableClock {
  public:
    virtual ~MockableClock() { };
    virtual qint64 now() = 0;
};

class MixxxClock : public MockableClock {
  public:
    virtual qint64 now() {
        return Time::elapsed();
    }
};

// MidiClock is not thread-safe, but is thread compatible using ControlObjects.
// The MIDI thread will make calls into MidiClock and then update two Control
// Objects with the current reported BPM and last beat time.  The engine thread
// can use those values to call a static function to calculate beat percentage
// at any time in the future.  Time values are in nanoseconds for compatibility
// with Time::elapsed().
class MidiClock {
  public:
    // The number of midi ticks per quarter note (1 beat in 4/4 time).
    static const int kPulsesPerQuarter = 24;

  private:
    // Some of the tests use the ring buffer size, so keep those test in sync
    // with this constant.
    static const int kRingBufferSize = kPulsesPerQuarter * 4;

  public:
    // Injectable clock for testing.  Does not take ownership of the clock.
    MidiClock(MockableClock* clock) : m_pClock(clock) { }

    // Handle an incoming midi status.  Return true if handled.
    bool handleMessage(unsigned char status);

    // Signals MIDI Start Sequence.  The MidiClock will reset its beat
    // percentage to 0, but the bpm value will be seeded with the last recorded
    // value.
    void start();

    // Signals MIDI Stop Sequence.  The MidiClock will stop updating its beat
    // precentage.  Subsequent calls to beatPercentage will return valid results
    // based on the last recorded beat time and last reported bpm.
    void stop();

    // Signals MIDI Timing Clock.  The timing between ticks will be used to
    // determine bpm.  kPulsesPerQuarter ticks = 1 beat.
    void tick();

    // Return the current BPM.  Values are significant to 5 decimal places.
    double bpm() const {
        return m_dBpm;
    }

    // Return the time of the last beat;
    qint64 lastBeatTime() const {
        return m_iLastBeatTime;
    }

    // Calculate instantaneous beat percentage based on provided values.  If
    // the beat percentage is >= 1.0, the integer value will be sliced off until
    // the result is between 0 <= x < 1.0.  Can be called from any thread
    // since it's static.
    static double beatPercentage(const qint64 last_beat, const qint64 now,
                                 const double bpm);

    // Convenience function for callers that have access to the MidiClock
    // object.  Returns the instantaneous beat percentage.  This should only be
    // called from the same thread that makes calls to tick().
    double beatPercentage() const;


    // Returns true if the clock is running.  A master sync listener should
    // always call this to make sure that the beatpercentage and bpm are
    // valid.
    bool running() const {
        return m_bRunning;
    }

  private:
    // Calculate the bpm based on the
    static double calcBpm(
            qint64 early_tick, qint64 late_tick, int tick_count);

    bool m_bRunning = false;
    // It's a hack to say 124 all over the source, but it provides a sane
    // baseline in case the midi device is already running when Mixxx starts up.
    double m_dBpm = 124.0;
    qint64 m_iLastBeatTime = 0;

    MockableClock* m_pClock;

    qint64 m_iTickRingBuffer[kRingBufferSize];
    int m_iRingBufferPos = 0;
    int m_iFilled = 0;
};


#endif  // MIDICLOCK_H
