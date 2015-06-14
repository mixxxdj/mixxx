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
    //     60secs per min / 124bpm / 24 tpb * 1e9 nps
    // tpb = ticks per beat
    // nps = nanos per second
    const double nanos_per_tick = 20161290.322580643;
    for (double t = 0; t < 60; ++t) {
        m_pFakeClock->setTime(static_cast<qint64>(t * nanos_per_tick));
        m_pMidiClock->tick();
    }

    EXPECT_FLOAT_EQ(124.0, m_pMidiClock->bpm());
    // position 60 is 12 ticks after 48, so 1/2 beat.
    EXPECT_FLOAT_EQ(0.5, m_pMidiClock->beatPercentage());
}

TEST_F(MidiClockTest, RingBufferTest) {
    // Same test, but more ticks to test the ringbuffer

    m_pMidiClock->start();

    // Nanos per tick for 124 bpm midi ticks.
    const double nanos_per_tick = 20161290.322580643;
    for (double t = 0; t < 144; ++t) {
        m_pFakeClock->setTime(static_cast<qint64>(t * nanos_per_tick));
        m_pMidiClock->tick();
    }

    EXPECT_FLOAT_EQ(124.0, m_pMidiClock->bpm());
    EXPECT_FLOAT_EQ(0.0, m_pMidiClock->beatPercentage());
}
