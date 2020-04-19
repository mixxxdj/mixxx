#include <gtest/gtest.h>

#include <QtDebug>

#include "track/beats.h"
#include "util/memory.h"

using namespace mixxx;

namespace {

class BeatsTest : public testing::Test {
  protected:
    BeatsTest()
            : m_pTrack(Track::newTemporary()),
              m_iSampleRate(44100),
              m_iFrameSize(2),
              m_pBeats(new Beats(m_pTrack.get(), m_iSampleRate)),
              m_bpm(60),
              m_startOffsetFrames(7),
              m_beatLengthFrames(getBeatLength(m_bpm)) {
        m_pBeats->setGridNew(m_bpm, m_startOffsetFrames);
        m_firstBeat = m_pBeats->getFirstBeatPosition();
        m_lastBeat = m_pBeats->getLastBeatPosition();
    }

    ~BeatsTest() {
    }

    double getBeatLength(double bpm) {
        if (bpm == 0) {
            DEBUG_ASSERT(false);
            return 0;
        }
        return (60.0 * m_iSampleRate / bpm);
    }

    QVector<double> createBeatVector(double first_beat,
            unsigned int num_beats,
            double beat_length) {
        QVector<double> beats;
        for (unsigned int i = 0; i < num_beats; ++i) {
            beats.append(first_beat + i * beat_length);
        }
        return beats;
    }

    TrackPointer m_pTrack;
    const SINT m_iSampleRate;
    const FrameNum m_iFrameSize;
    BeatsPointer m_pBeats;
    const double m_bpm;
    const double m_startOffsetFrames;
    const FrameNum m_beatLengthFrames;
    FrameNum m_firstBeat;
    FrameNum m_lastBeat;
};

TEST_F(BeatsTest, Scale) {
    // Initially must be the base value
    EXPECT_DOUBLE_EQ(m_bpm, m_pBeats->getBpmNew());

    m_pBeats->scale(Beats::DOUBLE);
    EXPECT_DOUBLE_EQ(2 * m_bpm, m_pBeats->getBpmNew());

    m_pBeats->scale(Beats::HALVE);
    EXPECT_DOUBLE_EQ(m_bpm, m_pBeats->getBpmNew());

    m_pBeats->scale(Beats::TWOTHIRDS);
    EXPECT_DOUBLE_EQ(m_bpm * 2 / 3, m_pBeats->getBpmNew());

    m_pBeats->scale(Beats::THREEHALVES);
    EXPECT_DOUBLE_EQ(m_bpm, m_pBeats->getBpmNew());

    m_pBeats->scale(Beats::THREEFOURTHS);
    EXPECT_DOUBLE_EQ(m_bpm * 3 / 4, m_pBeats->getBpmNew());

    m_pBeats->scale(Beats::FOURTHIRDS);
    EXPECT_DOUBLE_EQ(m_bpm, m_pBeats->getBpmNew());
}

TEST_F(BeatsTest, NthBeat) {
    // Check edge cases
    EXPECT_EQ(m_lastBeat, m_pBeats->findNthBeatNew(m_lastBeat, 1));
    EXPECT_EQ(m_lastBeat, m_pBeats->findNextBeatNew(m_lastBeat));
    EXPECT_EQ(-1, m_pBeats->findNthBeatNew(m_lastBeat, 2));
    EXPECT_EQ(m_firstBeat, m_pBeats->findNthBeatNew(m_firstBeat, -1));
    EXPECT_EQ(m_firstBeat, m_pBeats->findPrevBeatNew(m_firstBeat));
    EXPECT_EQ(-1, m_pBeats->findNthBeatNew(m_firstBeat, -2));

    // TODO(JVC) Add some tests in the middle
}

TEST_F(BeatsTest, PrevNextBeats) {
    FrameNum prevBeat, nextBeat;

    m_pBeats->findPrevNextBeatsNew(m_lastBeat, &prevBeat, &nextBeat);
    EXPECT_DOUBLE_EQ(m_lastBeat, prevBeat);
    EXPECT_DOUBLE_EQ(-1, nextBeat);

    m_pBeats->findPrevNextBeatsNew(m_firstBeat, &prevBeat, &nextBeat);
    EXPECT_DOUBLE_EQ(m_firstBeat, prevBeat);
    EXPECT_DOUBLE_EQ(m_firstBeat + m_beatLengthFrames, nextBeat);

    // TODO(JVC) Add some tests in the middle
}

TEST_F(BeatsTest, NthBeatWhenOnBeat) {
    // Pretend we're on the 20th beat;
    const int curBeat = 20;
    double position = m_startOffsetFrames + m_beatLengthFrames * curBeat;

    // The spec dictates that a value of 0 is always invalid and returns -1
    EXPECT_EQ(-1, m_pBeats->findNthBeatNew(position, 0));

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < curBeat; ++i) {
        EXPECT_DOUBLE_EQ(position + m_beatLengthFrames * (i - 1), m_pBeats->findNthBeatNew(position, i));
        EXPECT_DOUBLE_EQ(position + m_beatLengthFrames * (-i + 1), m_pBeats->findNthBeatNew(position, -i));
    }

    // Also test prev/next beat calculation.
    double prevBeat, nextBeat;
    m_pBeats->findPrevNextBeatsNew(position, &prevBeat, &nextBeat);
    EXPECT_EQ(position, prevBeat);
    EXPECT_EQ(position + m_beatLengthFrames, nextBeat);

    // Both previous and next beat should return the current position.
    EXPECT_EQ(position, m_pBeats->findNextBeatNew(position));
    EXPECT_EQ(position, m_pBeats->findPrevBeatNew(position));
}

TEST_F(BeatsTest, NthBeatWhenOnBeat_BeforeEpsilon) {
    // Pretend we're just before the 20th beat;
    const int curBeat = 20;
    const FrameNum kClosestBeat = m_startOffsetFrames + curBeat * m_beatLengthFrames;
    FrameNum position = kClosestBeat - m_beatLengthFrames * 0.005;

    // The spec dictates that a value of 0 is always invalid and returns -1
    EXPECT_EQ(-1, m_pBeats->findNthBeatNew(position, 0));

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < curBeat; ++i) {
        EXPECT_DOUBLE_EQ(kClosestBeat + m_beatLengthFrames * (i - 1),
                m_pBeats->findNthBeatNew(position, i));
        EXPECT_DOUBLE_EQ(kClosestBeat + m_beatLengthFrames * (-i + 1),
                m_pBeats->findNthBeatNew(position, -i));
    }

    // Also test prev/next beat calculation
    FrameNum prevBeat, nextBeat;
    m_pBeats->findPrevNextBeatsNew(position, &prevBeat, &nextBeat);
    EXPECT_EQ(kClosestBeat, prevBeat);
    EXPECT_EQ(kClosestBeat + m_beatLengthFrames, nextBeat);

    // Both previous and next beat should return the closest beat.
    EXPECT_EQ(kClosestBeat, m_pBeats->findNextBeatNew(position));
    EXPECT_EQ(kClosestBeat, m_pBeats->findPrevBeatNew(position));
}

TEST_F(BeatsTest, NthBeatWhenOnBeat_AfterEpsilon) {
    // Pretend we're just after the 20th beat;
    const int curBeat = 20;
    const double kClosestBeat = m_startOffsetFrames + curBeat * m_beatLengthFrames;
    double position = kClosestBeat + m_beatLengthFrames * 0.005;

    // The spec dictates that a value of 0 is always invalid and returns -1
    EXPECT_EQ(-1, m_pBeats->findNthBeatNew(position, 0));

    EXPECT_EQ(kClosestBeat, m_pBeats->findClosestBeatNew(position));

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < curBeat; ++i) {
        EXPECT_DOUBLE_EQ(kClosestBeat + m_beatLengthFrames * (i - 1),
                m_pBeats->findNthBeatNew(position, i));
        EXPECT_DOUBLE_EQ(kClosestBeat + m_beatLengthFrames * (-i + 1),
                m_pBeats->findNthBeatNew(position, -i));
    }

    // Also test prev/next beat calculation.
    double prevBeat, nextBeat;
    m_pBeats->findPrevNextBeatsNew(position, &prevBeat, &nextBeat);
    EXPECT_EQ(kClosestBeat, prevBeat);
    EXPECT_EQ(kClosestBeat + m_beatLengthFrames, nextBeat);

    // Both previous and next beat should return the closest beat.
    EXPECT_EQ(kClosestBeat, m_pBeats->findNextBeatNew(position));
    EXPECT_EQ(kClosestBeat, m_pBeats->findPrevBeatNew(position));
}

TEST_F(BeatsTest, NthBeatWhenNotOnBeat) {
    // Pretend we're half way between the 20th and 21st beat
    double previousBeat = m_startOffsetFrames + m_beatLengthFrames * 20.0;
    double nextBeat = m_startOffsetFrames + m_beatLengthFrames * 21.0;
    double position = (nextBeat + previousBeat) / 2.0;

    // The spec dictates that a value of 0 is always invalid and returns -1
    EXPECT_EQ(-1, m_pBeats->findNthBeatNew(position, 0));

    // findNthBeat should return multiples of beats starting from the next or
    // previous beat, depending on whether N is positive or negative.
    for (int i = 1; i < 20; ++i) {
        EXPECT_DOUBLE_EQ(nextBeat + m_beatLengthFrames * (i - 1),
                m_pBeats->findNthBeatNew(position, i));
        EXPECT_DOUBLE_EQ(previousBeat - m_beatLengthFrames * (i - 1),
                m_pBeats->findNthBeatNew(position, -i));
    }

    // Also test prev/next beat calculation
    double foundPrevBeat, foundNextBeat;
    m_pBeats->findPrevNextBeatsNew(position, &foundPrevBeat, &foundNextBeat);
    EXPECT_EQ(previousBeat, foundPrevBeat);
    EXPECT_EQ(nextBeat, foundNextBeat);
}

TEST_F(BeatsTest, BpmAround) {
    double approx_beat_length = getBeatLength(m_bpm);
    const int numBeats = 64;

    // Constant BPM, constructed in BeatsTest
    for (unsigned int i = 0; i < 100; i++) {
        EXPECT_DOUBLE_EQ(60, m_pBeats->getBpmAroundPositionNew(10, 5));
    }

    // Prepare a new Beats to test the behavior for variable BPM
    QVector<double> beats;
    double beat_pos = 0;
    for (unsigned int i = 0, bpm = 60; i < numBeats; ++i, ++bpm) {
        double beat_length = getBeatLength(bpm);
        beats.append(beat_pos);
        beat_pos += beat_length;
    }

    auto pMap = std::make_unique<Beats>(m_pTrack.get(), beats, m_iSampleRate);

    // Values calculated externally using a spreadsheet, verifying the results
    // and making some changes in the last decimals to fix rounding differences
    // the real and the theoretical results. What this tests checks is that
    // future changes do not modify the results.

    // Test values of n < 12 for simplified algorithm
    EXPECT_DOUBLE_EQ(67.990269973961901,
            pMap->getBpmAroundPositionNew(10 * approx_beat_length, 4));
    EXPECT_DOUBLE_EQ(120.99503094229186,
            pMap->getBpmAroundPositionNew(50 * approx_beat_length, 4));
    // Also test at the beginning and end of the track
    EXPECT_DOUBLE_EQ(60.989289610768779,
            pMap->getBpmAroundPositionNew(0, 4));
    EXPECT_DOUBLE_EQ(120.99503094229186,
            pMap->getBpmAroundPositionNew(65 * approx_beat_length, 4));

    // Test values of n > 12 for complete algorithm
    EXPECT_DOUBLE_EQ(75.340000000000003,
            pMap->getBpmAroundPositionNew(20 * approx_beat_length, 15));
    EXPECT_DOUBLE_EQ(115.40000000000001,
            pMap->getBpmAroundPositionNew(60 * approx_beat_length, 15));
    // Also test at the beginning and end of the track
    EXPECT_DOUBLE_EQ(66.320000000000007,
            pMap->getBpmAroundPositionNew(0, 15));
    EXPECT_DOUBLE_EQ(115.40000000000001,
            pMap->getBpmAroundPositionNew(65 * approx_beat_length, 15));

    // Try a really, really short track, 3 beats, constant BPM
    beats = createBeatVector(10, 3, getBeatLength(m_bpm));
    m_pBeats = std::make_unique<mixxx::Beats>(m_pTrack.get(), beats, m_iSampleRate);
    EXPECT_DOUBLE_EQ(m_bpm, m_pBeats->getBpmAroundPositionNew(1 * approx_beat_length, 4));
}

TEST_F(BeatsTest, Signature) {
    // Undefined time signature must be default
    EXPECT_EQ(m_pBeats->getSignatureNew(),
            kDefaultTimeSignature)
            << "If no Time Signature defined, it must be default(4/4)";

    // Add time signature to the beginning
    m_pBeats->setSignatureNew(TimeSignature(3, 4));

    // Add time signature in beats not at the beginning
    m_pBeats->setSignatureNew(TimeSignature(5, 4), 1000000);
    m_pBeats->setSignatureNew(TimeSignature(5, 3), 5000000);

    TimeSignature test = m_pBeats->getSignatureNew();
    EXPECT_EQ(m_pBeats->getSignatureNew(),
            TimeSignature(3, 4))
            << "Starting Time Signature must be 3/4";
    EXPECT_EQ(m_pBeats->getSignatureNew(500000),
            TimeSignature(3, 4))
            << "Time Signature at 500000 must be 3/4";
    EXPECT_EQ(m_pBeats->getSignatureNew(1000000),
            TimeSignature(5, 4))
            << "Time Signature at 1000000 must be 5/4";
    EXPECT_EQ(m_pBeats->getSignatureNew(5000000),
            TimeSignature(5, 3))
            << "Time Signature at 5000000 must be 5/3";
    EXPECT_EQ(m_pBeats->getSignatureNew(100000000),
            TimeSignature(5, 3))
            << "Time Signature at 100000000 must be 5/3";

    // Add a signature past the end of the track, must have no effect, and check
    m_pBeats->setSignatureNew(TimeSignature(6, 4), 10000000);
    EXPECT_EQ(m_pBeats->getSignatureNew(100000000),
            TimeSignature(5, 3))
            << "setSignature after the end of track must have no effect";
}

} // namespace
