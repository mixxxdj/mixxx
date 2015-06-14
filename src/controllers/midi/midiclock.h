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

// MidiClock will be accessed from two threads.  One, the MIDI thread, will call
// start, stop, and tick.  The other, the engine thread, will call
// beatPercentage on every buffer to determine where the midi clock is.

// Bpm and beat percentage are stored as atomic integers for lock-free design.
// The reduction in floating point precision does not affect internal state, and
// is more than precise enough for midi (which is notoriously drifty).
class MidiClock {
    static const int kRingBufferSize = 96;  // 24 * 4 = 1 measure
    static const int kFixedPrecision = 1e5;  // 5 decimal places.

 public:
    // Injectable clock for testing.  Does not take ownership of the clock.
    MidiClock(MockableClock* clock) : m_pClock(clock) { }

    // Signals MIDI Start Sequence.  The MidiClock will reset its beat
    // percentage to 0.
    void start();

    // Signals MIDI Stop Sequence.  The MidiClock will stop updating its beat
    // precentage.
    void stop();

    // Signals MIDI Timing Clock.  The timing between ticks will be used to
    // determine bpm.  24 ticks = 1 beat.
    void tick();

    // Return the current BPM.  Values are significant to 5 decimal places.
    double bpm() const;

    // Return the instantaneous beat percentage.
    double beatPercentage() const;

    // Returns true if the clock is running.  A master sync listener should
    // always call this to make sure that the beatpercentage and bpm are
    // valid.
    bool running() const {
        return static_cast<bool>(m_bRunning);
    }

 private:
    double calcBpm(qint64 early_tick, qint64 late_tick) const;

    MockableClock* m_pClock;

    QAtomicInt m_bRunning = false;
    qint64 m_iTickRingBuffer[kRingBufferSize];
    qint64 m_iLastBeatTime = 0;
    int m_iRingBufferPos = 0;
    int m_iFilled = 0;

    QAtomicInt m_aiBpm = 0.0;
};


#endif  // MIDICLOCK_H
