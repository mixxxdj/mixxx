#include <gtest/gtest.h>

#include <QtDebug>

#include "track/beats.h"
#include "track/track.h"
#include "util/memory.h"

using namespace mixxx;

namespace {

class BeatsTest : public testing::Test {
  protected:
    BeatsTest()
            : m_pTrack(Track::newTemporary()),
              m_iChannelCount(2),
              m_iSampleRate(100),
              m_pBeats1(new Beats(m_pTrack.get())),
              m_pBeats2(new Beats(m_pTrack.get())),
              m_bpm(60),
              m_startOffsetFrames(7) {
        m_pTrack->setAudioProperties(
                mixxx::audio::ChannelCount(m_iChannelCount),
                mixxx::audio::SampleRate(m_iSampleRate),
                mixxx::audio::Bitrate(),
                mixxx::Duration::fromSeconds(180));
        m_pBeats1->setGrid(m_bpm, m_startOffsetFrames);
        m_pBeats2->setGrid(m_bpm, m_startOffsetFrames);
    }

    ~BeatsTest() {
    }

    FrameDiff_t getBeatLengthFrames(Bpm bpm) const {
        if (bpm == Bpm()) {
            DEBUG_ASSERT(false);
            return 0;
        }
        return 60.0 * m_iSampleRate / bpm.getValue();
    }

    QVector<FramePos> createBeatVector(FramePos first_beat,
            unsigned int num_beats,
            FrameDiff_t beat_length) {
        QVector<FramePos> beats;
        for (unsigned int i = 0; i < num_beats; ++i) {
            beats.append(first_beat + beat_length * i);
        }
        return beats;
    }

    TrackPointer m_pTrack;
    const int m_iChannelCount;
    // Sample Rate is a standard unit
    const SINT m_iSampleRate;
    // We internally use frames per second since a frame position
    // actually matters when calculating beats.
    BeatsPointer m_pBeats1;
    BeatsPointer m_pBeats2;
    const Bpm m_bpm;
    const FramePos m_startOffsetFrames;
};

TEST_F(BeatsTest, Scale) {
    // Initially must be the base value
    EXPECT_EQ(m_bpm, m_pBeats1->getBpm());

    m_pBeats1->scale(BeatsInternal::DOUBLE);
    EXPECT_EQ(m_bpm * 2, m_pBeats1->getBpm());

    m_pBeats1->scale(BeatsInternal::HALVE);
    EXPECT_EQ(m_bpm, m_pBeats1->getBpm());

    m_pBeats1->scale(BeatsInternal::TWOTHIRDS);
    EXPECT_EQ(m_bpm * 2 / 3, m_pBeats1->getBpm());

    m_pBeats1->scale(BeatsInternal::THREEHALVES);
    EXPECT_EQ(m_bpm, m_pBeats1->getBpm());

    m_pBeats1->scale(BeatsInternal::THREEFOURTHS);
    EXPECT_EQ(m_bpm * 3 / 4, m_pBeats1->getBpm());

    m_pBeats1->scale(BeatsInternal::FOURTHIRDS);
    EXPECT_EQ(m_bpm, m_pBeats1->getBpm());
}

TEST_F(BeatsTest, NthBeat) {
    // Check edge cases
    EXPECT_EQ(m_pBeats1->getLastBeatPosition(),
            m_pBeats1->findNthBeat(m_pBeats1->getLastBeatPosition(), 1).getFramePosition());
    EXPECT_EQ(m_pBeats1->getLastBeatPosition(),
            m_pBeats1->findNextBeat(m_pBeats1->getLastBeatPosition()).getFramePosition());
    EXPECT_EQ(kInvalidFramePos,
            m_pBeats1->findNthBeat(m_pBeats1->getLastBeatPosition(), 2).getFramePosition());
    EXPECT_EQ(m_pBeats1->getFirstBeatPosition(),
            m_pBeats1->findNthBeat(m_pBeats1->getFirstBeatPosition(), -1).getFramePosition());
    EXPECT_EQ(m_pBeats1->getFirstBeatPosition(),
            m_pBeats1->findPrevBeat(m_pBeats1->getFirstBeatPosition()).getFramePosition());
    EXPECT_EQ(kInvalidFramePos,
            m_pBeats1->findNthBeat(m_pBeats1->getFirstBeatPosition(), -2).getFramePosition());

    // TODO(JVC) Add some tests in the middle
}

TEST_F(BeatsTest, PrevNextBeats) {
    FramePos prevBeat, nextBeat;

    m_pBeats1->findPrevNextBeats(
            m_pBeats1->getLastBeatPosition(), &prevBeat, &nextBeat);
    EXPECT_DOUBLE_EQ(
            m_pBeats1->getLastBeatPosition().getValue(), prevBeat.getValue());
    EXPECT_EQ(kInvalidFramePos, nextBeat);

    m_pBeats1->findPrevNextBeats(
            m_pBeats1->getFirstBeatPosition(), &prevBeat, &nextBeat);
    EXPECT_DOUBLE_EQ(
            m_pBeats1->getFirstBeatPosition().getValue(), prevBeat.getValue());
    EXPECT_DOUBLE_EQ(
            (m_pBeats1->getFirstBeatPosition() + getBeatLengthFrames(m_bpm))
                    .getValue(),
            nextBeat.getValue());

    // TODO(JVC) Add some tests in the middle
}

TEST_F(BeatsTest, NthBeatWhenOnBeat) {
    // Pretend we're on the 20th beat;
    const int curBeat = 20;
    FramePos position = m_startOffsetFrames + getBeatLengthFrames(m_bpm) * curBeat;

    // The spec dictates that a value of 0 is always invalid
    EXPECT_EQ(kInvalidFramePos, m_pBeats1->findNthBeat(position, 0).getFramePosition());

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < curBeat; ++i) {
        EXPECT_DOUBLE_EQ(
                (position + getBeatLengthFrames(m_bpm) * (i - 1)).getValue(),
                m_pBeats1->findNthBeat(position, i).getFramePosition().getValue());
        EXPECT_DOUBLE_EQ(
                (position + getBeatLengthFrames(m_bpm) * (-i + 1)).getValue(),
                m_pBeats1->findNthBeat(position, -i).getFramePosition().getValue());
    }

    // Also test prev/next beat calculation.
    FramePos prevBeat, nextBeat;
    m_pBeats1->findPrevNextBeats(position, &prevBeat, &nextBeat);
    EXPECT_EQ(position, prevBeat);
    EXPECT_EQ(position + getBeatLengthFrames(m_bpm), nextBeat);

    // Both previous and next beat should return the current position.
    EXPECT_EQ(position, m_pBeats1->findNextBeat(position).getFramePosition());
    EXPECT_EQ(position, m_pBeats1->findPrevBeat(position).getFramePosition());
}

TEST_F(BeatsTest, NthBeatWhenOnBeat_BeforeEpsilon) {
    // Pretend we're just before the 20th beat;
    const int curBeat = 20;
    const FramePos kClosestBeat = m_startOffsetFrames + curBeat * getBeatLengthFrames(m_bpm);
    FramePos position = kClosestBeat - getBeatLengthFrames(m_bpm) * 0.005;

    // The spec dictates that a value of 0 is always invalid and returns invalid beat.
    EXPECT_EQ(kInvalidBeat, m_pBeats1->findNthBeat(position, 0));

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < curBeat; ++i) {
        EXPECT_DOUBLE_EQ((kClosestBeat + getBeatLengthFrames(m_bpm) * (i - 1)).getValue(),
                m_pBeats1->findNthBeat(position, i).getFramePosition().getValue());
        EXPECT_DOUBLE_EQ((kClosestBeat + getBeatLengthFrames(m_bpm) * (-i + 1)).getValue(),
                m_pBeats1->findNthBeat(position, -i).getFramePosition().getValue());
    }

    // Also test prev/next beat calculation
    FramePos prevBeat, nextBeat;
    m_pBeats1->findPrevNextBeats(position, &prevBeat, &nextBeat);
    EXPECT_EQ(kClosestBeat, prevBeat);
    EXPECT_EQ(kClosestBeat + getBeatLengthFrames(m_bpm), nextBeat);

    // Both previous and next beat should return the closest beat.
    EXPECT_EQ(kClosestBeat, m_pBeats1->findNextBeat(position).getFramePosition());
    EXPECT_EQ(kClosestBeat, m_pBeats1->findPrevBeat(position).getFramePosition());
}

TEST_F(BeatsTest, NthBeatWhenOnBeat_AfterEpsilon) {
    // Pretend we're just after the 20th beat;
    const int curBeat = 20;
    const FramePos kClosestBeat = m_startOffsetFrames + curBeat * getBeatLengthFrames(m_bpm);
    FramePos position =
            kClosestBeat + getBeatLengthFrames(m_bpm) * 0.005;

    // The spec dictates that a value of 0 is always invalid
    EXPECT_EQ(kInvalidFramePos, m_pBeats1->findNthBeat(position, 0).getFramePosition());

    EXPECT_EQ(kClosestBeat, m_pBeats1->findClosestBeat(position));

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < curBeat; ++i) {
        EXPECT_DOUBLE_EQ(
                (kClosestBeat + getBeatLengthFrames(m_bpm) * (i - 1)).getValue(),
                m_pBeats1->findNthBeat(position, i).getFramePosition().getValue());
        EXPECT_DOUBLE_EQ(
                (kClosestBeat + getBeatLengthFrames(m_bpm) * (-i + 1)).getValue(),
                m_pBeats1->findNthBeat(position, -i).getFramePosition().getValue());
    }

    // Also test prev/next beat calculation.
    FramePos prevBeat, nextBeat;
    m_pBeats1->findPrevNextBeats(position, &prevBeat, &nextBeat);
    EXPECT_EQ(kClosestBeat, prevBeat);
    EXPECT_EQ(kClosestBeat + getBeatLengthFrames(m_bpm), nextBeat);

    // Both previous and next beat should return the closest beat.
    EXPECT_EQ(kClosestBeat, m_pBeats1->findNextBeat(position).getFramePosition());
    EXPECT_EQ(kClosestBeat, m_pBeats1->findPrevBeat(position).getFramePosition());
}

TEST_F(BeatsTest, NthBeatWhenNotOnBeat) {
    // Pretend we're half way between the 20th and 21st beat
    FramePos previousBeat =
            m_startOffsetFrames + getBeatLengthFrames(m_bpm) * 20.0;
    FramePos nextBeat =
            m_startOffsetFrames + getBeatLengthFrames(m_bpm) * 21.0;
    FramePos position = FramePos((previousBeat.getValue() + nextBeat.getValue()) / 2);

    // The spec dictates that a value of 0 is always invalid
    EXPECT_EQ(kInvalidFramePos, m_pBeats1->findNthBeat(position, 0).getFramePosition());

    // findNthBeat should return multiples of beats starting from the next or
    // previous beat, depending on whether N is positive or negative.
    for (int i = 1; i < 20; ++i) {
        EXPECT_DOUBLE_EQ(
                (nextBeat + getBeatLengthFrames(m_bpm) * (i - 1))
                        .getValue(),
                m_pBeats1->findNthBeat(position, i).getFramePosition().getValue());
        EXPECT_DOUBLE_EQ(
                (previousBeat - getBeatLengthFrames(m_bpm) * (i - 1))
                        .getValue(),
                m_pBeats1->findNthBeat(position, -i).getFramePosition().getValue());
    }

    // Also test prev/next beat calculation
    FramePos foundPrevBeat, foundNextBeat;
    m_pBeats1->findPrevNextBeats(position, &foundPrevBeat, &foundNextBeat);
    EXPECT_EQ(previousBeat, foundPrevBeat);
    EXPECT_EQ(nextBeat, foundNextBeat);
}

TEST_F(BeatsTest, BpmAround) {
    const FrameDiff_t approxBeatLengthFrames = getBeatLengthFrames(m_bpm);
    const int numBeats = 64;

    // Constant BPM, constructed in BeatsTest
    for (unsigned int i = 0; i < 100; i++) {
        EXPECT_EQ(Bpm(60), m_pBeats1->getBpmAroundPosition(FramePos(i), 5));
    }

    // Prepare a new Beats to test the behavior for variable BPM
    QVector<FramePos> beats;
    FramePos beat_pos;
    Bpm bpm(60);
    for (unsigned int i = 0; i < numBeats; ++i, bpm = bpm + 1) {
        FrameDiff_t beat_length = getBeatLengthFrames(bpm);
        beats.append(beat_pos);
        beat_pos += beat_length;
    }
    TrackPointer track = Track::newTemporary();
    track->setAudioProperties(
            mixxx::audio::ChannelCount(m_iChannelCount),
            mixxx::audio::SampleRate(m_iSampleRate),
            mixxx::audio::Bitrate(),
            mixxx::Duration::fromSeconds(beat_pos.getValue() / m_iSampleRate));
    BeatsPointer pMap =
            std::make_unique<Beats>(track.get(), beats);
    // The average of the first 8 beats should be different than the average
    // of the last 8 beats.
    EXPECT_DOUBLE_EQ(63.937454161267674,
            pMap->getBpmAroundPosition(kStartFramePos + approxBeatLengthFrames * 4, 4).getValue());
    EXPECT_DOUBLE_EQ(118.96637943082918,
            pMap->getBpmAroundPosition(kStartFramePos + approxBeatLengthFrames * 60, 4).getValue());
    // Also test at the beginning and end of the track
    EXPECT_DOUBLE_EQ(62.936459878052396,
            pMap->getBpmAroundPosition(kStartFramePos, 4).getValue());
    EXPECT_DOUBLE_EQ(118.96637943082918,
            pMap->getBpmAroundPosition(kStartFramePos + approxBeatLengthFrames * 65, 4).getValue());

    // Try a really, really short track
    beats = createBeatVector(FramePos(10), 3, getBeatLengthFrames(m_bpm));
    track->setDuration(beats.last().getValue() / m_iSampleRate);
    BeatsPointer pBeats =
            std::make_unique<Beats>(track.get(), beats);
    EXPECT_DOUBLE_EQ(m_bpm.getValue(),
            pBeats->getBpmAroundPosition(
                          kStartFramePos + approxBeatLengthFrames * 1, 4)
                    .getValue());
}

TEST_F(BeatsTest, Signature) {
    // Undefined time signature must be default
    EXPECT_EQ(m_pBeats1->getBeatAtIndex(0).getTimeSignature(),
            TimeSignature())
            << "If no Time Signature defined, it must be default(4/4)";
    const auto timeSignatureInitial = TimeSignature(3, 4);
    const auto timeSignatureIntermediate = TimeSignature(4, 8);
    const auto timeSignatureLater = TimeSignature(5, 2);

    // Add time signature to the beginning
    m_pBeats1->setSignature(timeSignatureInitial, 0);
    int firstSwitchDownbeatIndex = 3;
    int secondSwitchBeatIndex = firstSwitchDownbeatIndex + 4;

    // Add time signature in beats not at the beginning
    m_pBeats1->setSignature(timeSignatureIntermediate, firstSwitchDownbeatIndex);
    m_pBeats1->setSignature(timeSignatureLater, secondSwitchBeatIndex);

    EXPECT_EQ(m_pBeats1->getBeatAtIndex(0).getTimeSignature(),
            timeSignatureInitial);
    EXPECT_EQ(m_pBeats1->getBeatAtIndex(1).getTimeSignature(),
            timeSignatureInitial);
    EXPECT_EQ(m_pBeats1->getBeatAtIndex(5).getTimeSignature(),
            timeSignatureInitial);
    EXPECT_EQ(m_pBeats1->getBeatAtIndex(9).getTimeSignature(),
            timeSignatureIntermediate);
    EXPECT_EQ(m_pBeats1->getBeatAtIndex(15).getTimeSignature(),
            timeSignatureIntermediate);
    EXPECT_EQ(m_pBeats1->getBeatAtIndex(25).getTimeSignature(),
            timeSignatureLater);
    EXPECT_EQ(m_pBeats1->getBeatAtIndex(40).getTimeSignature(),
            timeSignatureLater);

    EXPECT_EQ(m_pBeats1->getBeatAtIndex(3).getType(), mixxx::Beat::DOWNBEAT);
    EXPECT_EQ(m_pBeats1->getBeatAtIndex(9).getType(), mixxx::Beat::DOWNBEAT);
    EXPECT_EQ(m_pBeats1->getBeatAtIndex(13).getType(), mixxx::Beat::DOWNBEAT);
    EXPECT_EQ(m_pBeats1->getBeatAtIndex(25).getType(), mixxx::Beat::DOWNBEAT);
    EXPECT_EQ(m_pBeats1->getBeatAtIndex(30).getType(), mixxx::Beat::DOWNBEAT);

    FrameDiff_t beatLengthCrochet = getBeatLengthFrames(m_bpm);

    FrameDiff_t firstBeatLength =
            m_pBeats1->getBeatAtIndex(1).getFramePosition() -
            m_pBeats1->getBeatAtIndex(0).getFramePosition();
    EXPECT_DOUBLE_EQ(beatLengthCrochet, firstBeatLength);

    FrameDiff_t ninthBeatLength =
            m_pBeats1->getBeatAtIndex(9).getFramePosition() -
            m_pBeats1->getBeatAtIndex(8).getFramePosition();
    EXPECT_DOUBLE_EQ(beatLengthCrochet, ninthBeatLength);

    FrameDiff_t tenthBeatLength =
            m_pBeats1->getBeatAtIndex(10).getFramePosition() -
            m_pBeats1->getBeatAtIndex(9).getFramePosition();
    EXPECT_DOUBLE_EQ(beatLengthCrochet / 2, tenthBeatLength);

    FrameDiff_t twentyFifthBeatLength =
            m_pBeats1->getBeatAtIndex(25).getFramePosition() -
            m_pBeats1->getBeatAtIndex(24).getFramePosition();
    EXPECT_DOUBLE_EQ(beatLengthCrochet / 2, twentyFifthBeatLength);

    FrameDiff_t twentySixthBeatLength =
            m_pBeats1->getBeatAtIndex(26).getFramePosition() -
            m_pBeats1->getBeatAtIndex(25).getFramePosition();
    EXPECT_DOUBLE_EQ(beatLengthCrochet * 2, twentySixthBeatLength);
}

TEST_F(BeatsTest, Iterator) {
    FramePos pos;

    // Full Beatsbeat
    auto iter1 = m_pBeats1->findBeats(m_pBeats1->getFirstBeatPosition(),
            m_pBeats1->getLastBeatPosition());
    EXPECT_DOUBLE_EQ(iter1->next().getFramePosition().getValue(),
            m_pBeats1->getFirstBeatPosition().getValue());
    while (iter1->hasNext()) {
        auto beat = iter1->next();
        pos = FramePos(beat.getFramePosition().getValue());
        EXPECT_TRUE(pos.getValue());
    }
    EXPECT_DOUBLE_EQ(
            pos.getValue(), m_pBeats1->getLastBeatPosition().getValue());

    // Past end
    auto iter2 = m_pBeats1->findBeats(m_pBeats1->getFirstBeatPosition(),
            FramePos(
                    m_pBeats1->getLastBeatPosition().getValue() + 10000000000));
    while (iter2->hasNext()) {
        auto beat = iter2->next();
        pos = FramePos(beat.getFramePosition().getValue());
        EXPECT_TRUE(pos.getValue());
    }
    EXPECT_DOUBLE_EQ(
            pos.getValue(), m_pBeats1->getLastBeatPosition().getValue());

    // Before begining
    auto iter3 = m_pBeats1->findBeats(
            FramePos(m_pBeats1->getFirstBeatPosition().getValue() - 1000000),
            m_pBeats1->getLastBeatPosition());
    EXPECT_DOUBLE_EQ(iter3->next().getFramePosition().getValue(),
            m_pBeats1->getFirstBeatPosition().getValue());
    while (iter3->hasNext()) {
        auto beat = iter3->next();
        pos = FramePos(beat.getFramePosition().getValue());
        EXPECT_TRUE(pos.getValue());
    }
    EXPECT_DOUBLE_EQ(pos.getValue(), m_pBeats1->getLastBeatPosition().getValue());
}

TEST_F(BeatsTest, Translate) {
    FrameDiff_t delta = 500;

    // Move the grid delta frames
    m_pBeats1->translate(delta);

    // All beats must have been displaced by delta frames
    auto iter1 = m_pBeats1->findBeats(m_pBeats1->getFirstBeatPosition(),
            m_pBeats1->getLastBeatPosition());
    auto iter2 = m_pBeats2->findBeats(m_pBeats2->getFirstBeatPosition(),
            m_pBeats2->getLastBeatPosition());
    while (iter1->hasNext()) {
        double pos1 = iter1->next().getFramePosition().getValue();
        double pos2 = iter2->next().getFramePosition().getValue();
        EXPECT_DOUBLE_EQ(pos1, pos2 + delta);
    }
    //    EXPECT_EQ(iter1->hasNext(), iter2->hasNext());
}

TEST_F(BeatsTest, FindClosest) {
    // Test deltas ranging from previous beat to next beat
    for (FrameDiff_t delta = -m_iSampleRate; delta <= m_iSampleRate; delta++) {
        auto iter1 = m_pBeats1->findBeats(m_pBeats1->getFirstBeatPosition(),
                m_pBeats1->getLastBeatPosition());
        while (iter1->hasNext()) {
            FramePos pos = FramePos(iter1->next().getFramePosition().getValue());
            FramePos foundPos = m_pBeats1->findClosestBeat(pos + delta);
            // Correct change of beat
            FramePos expectedPos = pos +
                    (delta > (m_iSampleRate / 2.0) ? m_iSampleRate : 0) +
                    (delta < (-m_iSampleRate / 2.0) ? -m_iSampleRate : 0);
            // Enforce boundaries
            expectedPos = std::min(expectedPos, m_pBeats1->getLastBeatPosition());
            expectedPos = std::max(expectedPos, m_pBeats1->getFirstBeatPosition());
            EXPECT_DOUBLE_EQ(foundPos.getValue(), expectedPos.getValue());
        }
        break;
    }
}

TEST_F(BeatsTest, ChangingTimeSignatureShouldNotChangeBpm) {
    // Set the track to have multiple BPM values.
    m_pBeats1->setBpm(Bpm(60), 0);
    m_pBeats1->setBpm(Bpm(120), 5);
    m_pBeats1->setBpm(Bpm(240), 10);
    m_pBeats1->setBpm(Bpm(75), 15);
    // Let's test global BPM first
    auto oldBpm = m_pBeats1->getBpm();
    auto oldTimeSignature = m_pBeats1->getBeatAtIndex(0).getTimeSignature();
    m_pBeats1->setSignature(TimeSignature(oldTimeSignature.getBeatsPerBar(),
                                    oldTimeSignature.getNoteValue() * 2),
            0);
    auto newBpm = m_pBeats1->getBpm();

    ASSERT_EQ(oldBpm, newBpm);
}
} // namespace
