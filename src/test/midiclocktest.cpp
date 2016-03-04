#include <QScopedPointer>

#include <gmock/gmock.h>

#include "controllers/midi/midisourceclock.h"
#include "test/mixxxtest.h"

class MidiSourceClockTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        m_pMidiSourceClock.reset(new MidiSourceClock());
    }
    QScopedPointer<MidiSourceClock> m_pMidiSourceClock;
};

TEST_F(MidiSourceClockTest, SimpleTest) {
    // Simple test for pulsing at a very steady rate and then getting the bpm.

    m_pMidiSourceClock->start();

    // Nanos per pulse for 124 bpm midi pulses:
    //     60secs per min / 124bpm / kPulsesPerQuarter ppb * 1e9 nps
    // ppb = pulses per beat
    // nps = nanos per second
    const double nanos_per_pulse =
            60.0 / 124 / MidiSourceClock::kPulsesPerQuarter * 1e9;
    // This test should end before the ringbuffer is exhausted and not on
    // a beat.
    mixxx::Duration now;
    for (double t = 0; t < MidiSourceClock::kPulsesPerQuarter * 2.5; ++t) {
        now = mixxx::Duration::fromNanos(t * nanos_per_pulse);
        m_pMidiSourceClock->pulse(now);
    }

    EXPECT_FLOAT_EQ(124.0, m_pMidiSourceClock->bpm());
    // position 60 is 12 pulses after 48, so 1/2 beat.
    EXPECT_FLOAT_EQ(0.5, m_pMidiSourceClock->beatFraction(
            m_pMidiSourceClock->lastBeatTime(), now,
            m_pMidiSourceClock->bpm()));
    // No jitter, so no difference in smoothed result.
    EXPECT_FLOAT_EQ(0.5, m_pMidiSourceClock->beatFraction(
            m_pMidiSourceClock->smoothedBeatTime(), now,
            m_pMidiSourceClock->bpm()));
}

TEST_F(MidiSourceClockTest, RingBufferTest) {
    m_pMidiSourceClock->start();

    // Nanos per pulse for 124 bpm midi pulses.
    const double nanos_per_pulse =
            60.0 / 124 / MidiSourceClock::kPulsesPerQuarter * 1e9;
    // This test should exhaust the ringbuffer at least once, and end on
    // a beat.  Any multiple of pulses per quarter is a beat.
    mixxx::Duration now;
    for (double t = 0; t < MidiSourceClock::kPulsesPerQuarter * 6; ++t) {
        now = mixxx::Duration::fromNanos(t * nanos_per_pulse);
        m_pMidiSourceClock->pulse(now);
    }

    EXPECT_FLOAT_EQ(124.0, m_pMidiSourceClock->bpm());
    EXPECT_FLOAT_EQ(0.0, m_pMidiSourceClock->beatFraction(
            m_pMidiSourceClock->lastBeatTime(), now,
            m_pMidiSourceClock->bpm()));
    // Rounding error should not be very significant.
    EXPECT_LT(fabs(0.0 - m_pMidiSourceClock->beatFraction(
            m_pMidiSourceClock->smoothedBeatTime(), now,
            m_pMidiSourceClock->bpm())), 1e-7);
}

TEST_F(MidiSourceClockTest, JitterBufferTest) {
    // smoothed downbeat and bpm should not drift very much when there is
    // jitter in the signal.
    m_pMidiSourceClock->start();
    const double nanos_per_pulse =
            60.0 / 124 / MidiSourceClock::kPulsesPerQuarter * 1e9;
    mixxx::Duration now;
    for (double t = 0; t < MidiSourceClock::kPulsesPerQuarter * 10000; ++t) {
        now = mixxx::Duration::fromNanos(t * nanos_per_pulse);
        // Test with up to 10ms error.
        const auto error = mixxx::Duration::fromMillis(qrand() % 10 - 5);
        //const mixxx::Duration error;
        m_pMidiSourceClock->pulse(now + error);
    }

    // Values will not be exact because of the jitter -- but they should
    // be fairly close.  This test is inherently flaky because we use random
    // numbers, but these tolerances are large enough that repeating the test
    // 10000 times results in no failures.
    EXPECT_LT(fabs(124.0 - m_pMidiSourceClock->bpm()), 1.0);

    // Make sure now is well ahead of the last beat time (could be less due
    // to the jitter.).  The .5 ensures we're at half a beat.
    now += mixxx::Duration::fromNanos(
            10.5 * MidiSourceClock::kPulsesPerQuarter * nanos_per_pulse);

    const double frac_last = m_pMidiSourceClock->beatFraction(
            m_pMidiSourceClock->lastBeatTime(), now,
            m_pMidiSourceClock->bpm());
    const double frac_smoothed = m_pMidiSourceClock->beatFraction(
            m_pMidiSourceClock->smoothedBeatTime(), now,
            m_pMidiSourceClock->bpm());
    EXPECT_LT(fabs(0.5 - frac_last), 0.1);
    EXPECT_LT(fabs(0.5 - frac_smoothed), 0.1);
}
