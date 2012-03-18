#include <gtest/gtest.h>
#include <QDebug>

#include "track/beatmap.h"

namespace {

class BeatMapTest : public testing::Test {
  protected:

    BeatMapTest()
            : m_pTrack(new TrackInfoObject(), &QObject::deleteLater),
              m_iSampleRate(100),
              m_iFrameSize(2) {

    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

    double getBeatLength(double bpm) {
        return (60.0 * m_iSampleRate / bpm) * m_iFrameSize;
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
    int m_iSampleRate;
    int m_iFrameSize;
};

TEST_F(BeatMapTest, TestNthBeat) {
    const double bpm = 60.0;
    m_pTrack->setBpm(bpm);
    m_pTrack->setSampleRate(m_iSampleRate);
    double beatLength = getBeatLength(bpm);
    double startOffset = 15.3;
    const int numBeats = 100;
    QVector<double> beats = createBeatVector(startOffset, numBeats, beatLength);
    BeatMap* pMap = new BeatMap(m_pTrack);
    pMap->createFromVector(beats);

    // Check edge cases
    double firstBeat = startOffset + beatLength * 0;
    double lastBeat = startOffset + beatLength * numBeats;
    EXPECT_EQ(lastBeat, pMap->findNthBeat(lastBeat, 1));
    EXPECT_EQ(-1, pMap->findNthBeat(lastBeat, 1));
    EXPECT_EQ(firstBeat, pMap->findNthBeat(firstBeat, -1));
    EXPECT_EQ(-1, pMap->findNthBeat(firstBeat, -2));
}

TEST_F(BeatMapTest, TestNthBeatWhenOnBeat) {
    const double bpm = 60.0;
    m_pTrack->setBpm(bpm);
    m_pTrack->setSampleRate(m_iSampleRate);
    double beatLength = getBeatLength(bpm);
    double startOffset = 15.3;
    const int numBeats = 100;
    QVector<double> beats = createBeatVector(startOffset, numBeats, beatLength);
    BeatMap* pMap = new BeatMap(m_pTrack);
    pMap->createFromVector(beats);

    // Pretend we're on the 20th beat;
    const int curBeat = 20;
    double position = startOffset + beatLength * curBeat;

    // The spec dictates that a value of 0 is always invalid and returns -1
    EXPECT_EQ(-1, pMap->findNthBeat(position, 0));

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < curBeat; ++i) {
        EXPECT_EQ(position + beatLength*(i-1), pMap->findNthBeat(position, i));
        EXPECT_EQ(position + beatLength*(-i+1), pMap->findNthBeat(position, -i));
    }
}

TEST_F(BeatMapTest, TestNthBeatWhenNotOnBeat) {
    const double bpm = 60.0;
    m_pTrack->setBpm(bpm);
    m_pTrack->setSampleRate(m_iSampleRate);
    double beatLength = getBeatLength(bpm);
    double startOffset = 15.3;
    const int numBeats = 100;
    QVector<double> beats = createBeatVector(startOffset, numBeats, beatLength);
    BeatMap* pMap = new BeatMap(m_pTrack);
    pMap->createFromVector(beats);

    // Pretend we're half way between the 20th and 21st beat
    double previousBeat = startOffset + beatLength * 20.0;
    double nextBeat = startOffset + beatLength * 21.0;
    double position = (nextBeat + previousBeat) / 2.0;

    // The spec dictates that a value of 0 is always invalid and returns -1
    EXPECT_EQ(-1, pMap->findNthBeat(position, 0));

    // findNthBeat should return multiples of beats starting from the next or
    // previous beat, depending on whether N is positive or negative.
    for (int i = 1; i < 20; ++i) {
        EXPECT_EQ(nextBeat + beatLength*(i-1), pMap->findNthBeat(position, i));
        EXPECT_EQ(previousBeat + beatLength*(-i+1), pMap->findNthBeat(position, -i));
    }
}

}  // namespace
