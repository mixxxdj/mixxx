#include <QScopedPointer>

#include <gmock/gmock.h>

#include "controllers/midi/midiclock.h"
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

class MidiClockTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        m_pFakeClock.reset(new FakeClock);
        m_pMidiClock.reset(new MidiClock(m_pFakeClock.data()));
    }

    QScopedPointer<FakeClock> m_pFakeClock;
    QScopedPointer<MidiClock> m_pMidiClock;
};

TEST_F(MidiClockTest, SimpleTest) {
    // Simple test for ticking at a very steady rate and then getting the bpm.

    m_pMidiClock->start();

    // Nanos per tick for 124 bpm midi ticks:
    //     60secs per min / 124bpm / kPulsesPerQuarter tpb * 1e9 nps
    // tpb = ticks per beat
    // nps = nanos per second
    const double nanos_per_tick =
            60.0 / 124 / MidiClock::kPulsesPerQuarter * 1e9;
    // This test should end before the ringbuffer is exhausted and not on
    // a beat.
    for (double t = 0; t < MidiClock::kPulsesPerQuarter * 2.5; ++t) {
        m_pFakeClock->setTime(static_cast<qint64>(t * nanos_per_tick));
        m_pMidiClock->tick();
    }

    EXPECT_FLOAT_EQ(124.0, m_pMidiClock->bpm());
    // position 60 is 12 ticks after 48, so 1/2 beat.
    EXPECT_FLOAT_EQ(0.5, m_pMidiClock->beatPercentage());
}

TEST_F(MidiClockTest, RingBufferTest) {
    m_pMidiClock->start();

    // Nanos per tick for 124 bpm midi ticks.
    const double nanos_per_tick =
            60.0 / 124 / MidiClock::kPulsesPerQuarter * 1e9;
    // This test should exhaust the ringbuffer at least once, and end on
    // a beat.
    for (double t = 0; t < MidiClock::kPulsesPerQuarter * 6; ++t) {
        m_pFakeClock->setTime(static_cast<qint64>(t * nanos_per_tick));
        m_pMidiClock->tick();
    }

    EXPECT_FLOAT_EQ(124.0, m_pMidiClock->bpm());
    EXPECT_FLOAT_EQ(0.0, m_pMidiClock->beatPercentage());
}
