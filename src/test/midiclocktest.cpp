#include <QScopedPointer>

#include <gmock/gmock.h>

#include "controllers/midi/midisourceclock.h"
#include "test/mixxxtest.h"

class FakeClock : public MockableClock {
  public:
    void setTime(qint64 time) {
        m_time = time;
    }

    virtual qint64 now() {
        return m_time;
    }
  private:
    qint64 m_time;
};

class MidiSourceClockTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        m_pFakeClock.reset(new FakeClock);
        m_pMidiSourceClock.reset(new MidiSourceClock(m_pFakeClock.data()));
    }

    QScopedPointer<FakeClock> m_pFakeClock;
    QScopedPointer<MidiSourceClock> m_pMidiSourceClock;
};

TEST_F(MidiSourceClockTest, SimpleTest) {
    // Simple test for ticking at a very steady rate and then getting the bpm.

    m_pMidiSourceClock->start();

    // Nanos per tick for 124 bpm midi ticks:
    //     60secs per min / 124bpm / kPulsesPerQuarter tpb * 1e9 nps
    // tpb = ticks per beat
    // nps = nanos per second
    const double nanos_per_tick =
            60.0 / 124 / MidiSourceClock::kPulsesPerQuarter * 1e9;
    // This test should end before the ringbuffer is exhausted and not on
    // a beat.
    for (double t = 0; t < MidiSourceClock::kPulsesPerQuarter * 2.5; ++t) {
        m_pFakeClock->setTime(static_cast<qint64>(t * nanos_per_tick));
        m_pMidiSourceClock->tick();
    }

    EXPECT_FLOAT_EQ(124.0, m_pMidiSourceClock->bpm());
    // position 60 is 12 ticks after 48, so 1/2 beat.
    EXPECT_FLOAT_EQ(0.5, m_pMidiSourceClock->beatPercentage());
}

TEST_F(MidiSourceClockTest, RingBufferTest) {
    m_pMidiSourceClock->start();

    // Nanos per tick for 124 bpm midi ticks.
    const double nanos_per_tick =
            60.0 / 124 / MidiSourceClock::kPulsesPerQuarter * 1e9;
    // This test should exhaust the ringbuffer at least once, and end on
    // a beat.
    for (double t = 0; t < MidiSourceClock::kPulsesPerQuarter * 6; ++t) {
        m_pFakeClock->setTime(static_cast<qint64>(t * nanos_per_tick));
        m_pMidiSourceClock->tick();
    }

    EXPECT_FLOAT_EQ(124.0, m_pMidiSourceClock->bpm());
    EXPECT_FLOAT_EQ(0.0, m_pMidiSourceClock->beatPercentage());
}
