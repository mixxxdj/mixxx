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
            : m_pTrack1(Track::newTemporary()),
              m_pTrack2(Track::newTemporary()),
              m_iChannelCount(2),
              m_iSampleRate(100),
              m_bpm(60),
              m_startOffsetFrames(7) {
        m_pTrack1->setAudioProperties(
                mixxx::audio::ChannelCount(m_iChannelCount),
                mixxx::audio::SampleRate(m_iSampleRate),
                mixxx::audio::Bitrate(),
                mixxx::Duration::fromSeconds(180));
        m_pTrack2->setAudioProperties(
                mixxx::audio::ChannelCount(m_iChannelCount),
                mixxx::audio::SampleRate(m_iSampleRate),
                mixxx::audio::Bitrate(),
                mixxx::Duration::fromSeconds(180));
        m_pTrack1->setBeats(BeatsInternal());
        m_pTrack2->setBeats(BeatsInternal());
        m_pTrack1->getBeats()->setGrid(m_bpm, m_startOffsetFrames);
        m_pTrack2->getBeats()->setGrid(m_bpm, m_startOffsetFrames);
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

    TrackPointer m_pTrack1;
    TrackPointer m_pTrack2;
    const int m_iChannelCount;
    // Sample Rate is a standard unit
    const SINT m_iSampleRate;
    // We internally use frames per second since a frame position
    // actually matters when calculating beats.
    const Bpm m_bpm;
    const FramePos m_startOffsetFrames;
};

TEST_F(BeatsTest, Scale) {
    const auto& pBeats = m_pTrack1->getBeats();
    // Initially must be the base value
    EXPECT_EQ(m_bpm, pBeats->getGlobalBpm());

    pBeats->scale(BeatsInternal::DOUBLE);
    EXPECT_EQ(m_bpm * 2, pBeats->getGlobalBpm());

    pBeats->scale(BeatsInternal::HALVE);
    EXPECT_EQ(m_bpm, pBeats->getGlobalBpm());

    pBeats->scale(BeatsInternal::TWOTHIRDS);
    EXPECT_EQ(m_bpm * 2 / 3, pBeats->getGlobalBpm());

    pBeats->scale(BeatsInternal::THREEHALVES);
    EXPECT_EQ(m_bpm, pBeats->getGlobalBpm());

    pBeats->scale(BeatsInternal::THREEFOURTHS);
    EXPECT_EQ(m_bpm * 3 / 4, pBeats->getGlobalBpm());

    pBeats->scale(BeatsInternal::FOURTHIRDS);
    EXPECT_EQ(m_bpm, pBeats->getGlobalBpm());
}

TEST_F(BeatsTest, NthBeat) {
    const auto& pBeats = m_pTrack1->getBeats();
    // Check edge cases
    EXPECT_EQ(pBeats->getLastBeatPosition(),
            pBeats->findNthBeat(pBeats->getLastBeatPosition(), 1).framePosition());
    EXPECT_EQ(pBeats->getLastBeatPosition(),
            pBeats->findNextBeat(pBeats->getLastBeatPosition()).framePosition());
    EXPECT_EQ(kInvalidFramePos,
            pBeats->findNthBeat(pBeats->getLastBeatPosition(), 2).framePosition());
    EXPECT_EQ(pBeats->getFirstBeatPosition(),
            pBeats->findNthBeat(pBeats->getFirstBeatPosition(), -1).framePosition());
    EXPECT_EQ(pBeats->getFirstBeatPosition(),
            pBeats->findPrevBeat(pBeats->getFirstBeatPosition()).framePosition());
    EXPECT_EQ(kInvalidFramePos,
            pBeats->findNthBeat(pBeats->getFirstBeatPosition(), -2).framePosition());

    // TODO(JVC) Add some tests in the middle
}

TEST_F(BeatsTest, PrevNextBeats) {
    const auto& pBeats = m_pTrack1->getBeats();
    FramePos prevBeat, nextBeat;

    pBeats->findPrevNextBeats(
            pBeats->getLastBeatPosition(), &prevBeat, &nextBeat);
    EXPECT_DOUBLE_EQ(
            pBeats->getLastBeatPosition().getValue(), prevBeat.getValue());
    EXPECT_EQ(kInvalidFramePos, nextBeat);

    pBeats->findPrevNextBeats(
            pBeats->getFirstBeatPosition(), &prevBeat, &nextBeat);
    EXPECT_DOUBLE_EQ(
            pBeats->getFirstBeatPosition().getValue(), prevBeat.getValue());
    EXPECT_DOUBLE_EQ(
            (pBeats->getFirstBeatPosition() + getBeatLengthFrames(m_bpm))
                    .getValue(),
            nextBeat.getValue());

    // TODO(JVC) Add some tests in the middle
}

TEST_F(BeatsTest, NthBeatWhenOnBeat) {
    const auto& pBeats = m_pTrack1->getBeats();
    // Pretend we're on the 20th beat;
    const int curBeat = 20;
    FramePos position = m_startOffsetFrames + getBeatLengthFrames(m_bpm) * curBeat;

    // The spec dictates that a value of 0 is always invalid
    EXPECT_EQ(kInvalidFramePos, pBeats->findNthBeat(position, 0).framePosition());

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < curBeat; ++i) {
        EXPECT_DOUBLE_EQ(
                (position + getBeatLengthFrames(m_bpm) * (i - 1)).getValue(),
                pBeats->findNthBeat(position, i).framePosition().getValue());
        EXPECT_DOUBLE_EQ(
                (position + getBeatLengthFrames(m_bpm) * (-i + 1)).getValue(),
                pBeats->findNthBeat(position, -i).framePosition().getValue());
    }

    // Also test prev/next beat calculation.
    FramePos prevBeat, nextBeat;
    pBeats->findPrevNextBeats(position, &prevBeat, &nextBeat);
    EXPECT_EQ(position, prevBeat);
    EXPECT_EQ(position + getBeatLengthFrames(m_bpm), nextBeat);

    // Both previous and next beat should return the current position.
    EXPECT_EQ(position, pBeats->findNextBeat(position).framePosition());
    EXPECT_EQ(position, pBeats->findPrevBeat(position).framePosition());
}

TEST_F(BeatsTest, NthBeatWhenOnBeat_BeforeEpsilon) {
    const auto& pBeats = m_pTrack1->getBeats();
    // Pretend we're just before the 20th beat;
    const int curBeat = 20;
    const FramePos kClosestBeat = m_startOffsetFrames + curBeat * getBeatLengthFrames(m_bpm);
    FramePos position = kClosestBeat - getBeatLengthFrames(m_bpm) * 0.005;

    // The spec dictates that a value of 0 is always invalid and returns invalid beat.
    EXPECT_EQ(kInvalidBeat, pBeats->findNthBeat(position, 0));

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < curBeat; ++i) {
        EXPECT_DOUBLE_EQ((kClosestBeat + getBeatLengthFrames(m_bpm) * (i - 1)).getValue(),
                pBeats->findNthBeat(position, i).framePosition().getValue());
        EXPECT_DOUBLE_EQ((kClosestBeat + getBeatLengthFrames(m_bpm) * (-i + 1)).getValue(),
                pBeats->findNthBeat(position, -i).framePosition().getValue());
    }

    // Also test prev/next beat calculation
    FramePos prevBeat, nextBeat;
    pBeats->findPrevNextBeats(position, &prevBeat, &nextBeat);
    EXPECT_EQ(kClosestBeat, prevBeat);
    EXPECT_EQ(kClosestBeat + getBeatLengthFrames(m_bpm), nextBeat);

    // Both previous and next beat should return the closest beat.
    EXPECT_EQ(kClosestBeat, pBeats->findNextBeat(position).framePosition());
    EXPECT_EQ(kClosestBeat, pBeats->findPrevBeat(position).framePosition());
}

TEST_F(BeatsTest, NthBeatWhenOnBeat_AfterEpsilon) {
    const auto& pBeats = m_pTrack1->getBeats();
    // Pretend we're just after the 20th beat;
    const int curBeat = 20;
    const FramePos kClosestBeat = m_startOffsetFrames + curBeat * getBeatLengthFrames(m_bpm);
    FramePos position =
            kClosestBeat + getBeatLengthFrames(m_bpm) * 0.005;

    // The spec dictates that a value of 0 is always invalid
    EXPECT_EQ(kInvalidFramePos, pBeats->findNthBeat(position, 0).framePosition());

    EXPECT_EQ(kClosestBeat, pBeats->findClosestBeat(position));

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < curBeat; ++i) {
        EXPECT_DOUBLE_EQ(
                (kClosestBeat + getBeatLengthFrames(m_bpm) * (i - 1)).getValue(),
                pBeats->findNthBeat(position, i).framePosition().getValue());
        EXPECT_DOUBLE_EQ(
                (kClosestBeat + getBeatLengthFrames(m_bpm) * (-i + 1)).getValue(),
                pBeats->findNthBeat(position, -i).framePosition().getValue());
    }

    // Also test prev/next beat calculation.
    FramePos prevBeat, nextBeat;
    pBeats->findPrevNextBeats(position, &prevBeat, &nextBeat);
    EXPECT_EQ(kClosestBeat, prevBeat);
    EXPECT_EQ(kClosestBeat + getBeatLengthFrames(m_bpm), nextBeat);

    // Both previous and next beat should return the closest beat.
    EXPECT_EQ(kClosestBeat, pBeats->findNextBeat(position).framePosition());
    EXPECT_EQ(kClosestBeat, pBeats->findPrevBeat(position).framePosition());
}

TEST_F(BeatsTest, NthBeatWhenNotOnBeat) {
    const auto& pBeats = m_pTrack1->getBeats();
    // Pretend we're half way between the 20th and 21st beat
    FramePos previousBeat =
            m_startOffsetFrames + getBeatLengthFrames(m_bpm) * 20.0;
    FramePos nextBeat =
            m_startOffsetFrames + getBeatLengthFrames(m_bpm) * 21.0;
    FramePos position = FramePos((previousBeat.getValue() + nextBeat.getValue()) / 2);

    // The spec dictates that a value of 0 is always invalid
    EXPECT_EQ(kInvalidFramePos, pBeats->findNthBeat(position, 0).framePosition());

    // findNthBeat should return multiples of beats starting from the next or
    // previous beat, depending on whether N is positive or negative.
    for (int i = 1; i < 20; ++i) {
        EXPECT_DOUBLE_EQ(
                (nextBeat + getBeatLengthFrames(m_bpm) * (i - 1))
                        .getValue(),
                pBeats->findNthBeat(position, i).framePosition().getValue());
        EXPECT_DOUBLE_EQ(
                (previousBeat - getBeatLengthFrames(m_bpm) * (i - 1))
                        .getValue(),
                pBeats->findNthBeat(position, -i).framePosition().getValue());
    }

    // Also test prev/next beat calculation
    FramePos foundPrevBeat, foundNextBeat;
    pBeats->findPrevNextBeats(position, &foundPrevBeat, &foundNextBeat);
    EXPECT_EQ(previousBeat, foundPrevBeat);
    EXPECT_EQ(nextBeat, foundNextBeat);
}

TEST_F(BeatsTest, BpmAround) {
    const auto& pBeats = m_pTrack1->getBeats();
    const FrameDiff_t approxBeatLengthFrames = getBeatLengthFrames(m_bpm);
    const int numBeats = 64;

    // Constant BPM, constructed in BeatsTest
    for (unsigned int i = 0; i < 100; i++) {
        EXPECT_EQ(Bpm(60), pBeats->getBpmAroundPosition(FramePos(i), 5));
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

    m_pTrack1->setDuration(mixxx::Duration::fromSeconds(beat_pos.getValue() / m_iSampleRate));
    pBeats->initWithAnalyzer(beats);
    BeatsPointer pMap = m_pTrack1->getBeats();
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
    m_pTrack1->setDuration(beats.last().getValue() / m_iSampleRate);
    m_pTrack1->getBeats()->initWithAnalyzer(beats);
    EXPECT_DOUBLE_EQ(m_bpm.getValue(),
            pBeats->getBpmAroundPosition(
                          kStartFramePos + approxBeatLengthFrames * 1, 4)
                    .getValue());
}

TEST_F(BeatsTest, Signature) {
    const auto& pBeats = m_pTrack1->getBeats();
    // Undefined time signature must be default
    EXPECT_EQ(pBeats->getBeatAtIndex(0).timeSignature(),
            TimeSignature())
            << "If no Time Signature defined, it must be default(4/4)";
    const auto timeSignatureInitial = TimeSignature(3, 4);
    const auto timeSignatureIntermediate = TimeSignature(4, 8);
    const auto timeSignatureLater = TimeSignature(5, 2);

    // Add time signature to the beginning
    pBeats->setSignature(timeSignatureInitial, 0);
    int firstSwitchDownbeatIndex = 3;
    int secondSwitchBeatIndex = firstSwitchDownbeatIndex + 4;

    // Add time signature in beats not at the beginning
    pBeats->setSignature(timeSignatureIntermediate, firstSwitchDownbeatIndex);
    pBeats->setSignature(timeSignatureLater, secondSwitchBeatIndex);

    EXPECT_EQ(pBeats->getBeatAtIndex(0).timeSignature(),
            timeSignatureInitial);
    EXPECT_EQ(pBeats->getBeatAtIndex(1).timeSignature(),
            timeSignatureInitial);
    EXPECT_EQ(pBeats->getBeatAtIndex(5).timeSignature(),
            timeSignatureInitial);
    EXPECT_EQ(pBeats->getBeatAtIndex(9).timeSignature(),
            timeSignatureIntermediate);
    EXPECT_EQ(pBeats->getBeatAtIndex(15).timeSignature(),
            timeSignatureIntermediate);
    EXPECT_EQ(pBeats->getBeatAtIndex(25).timeSignature(),
            timeSignatureLater);
    EXPECT_EQ(pBeats->getBeatAtIndex(40).timeSignature(),
            timeSignatureLater);

    EXPECT_EQ(pBeats->getBeatAtIndex(3).type(), mixxx::BeatType::Downbeat);
    EXPECT_EQ(pBeats->getBeatAtIndex(9).type(), mixxx::BeatType::Downbeat);
    EXPECT_EQ(pBeats->getBeatAtIndex(13).type(), mixxx::BeatType::Downbeat);
    EXPECT_EQ(pBeats->getBeatAtIndex(25).type(), mixxx::BeatType::Downbeat);
    EXPECT_EQ(pBeats->getBeatAtIndex(30).type(), mixxx::BeatType::Downbeat);

    FrameDiff_t beatLengthCrochet = getBeatLengthFrames(m_bpm);

    FrameDiff_t firstBeatLength =
            pBeats->getBeatAtIndex(1).framePosition() -
            pBeats->getBeatAtIndex(0).framePosition();
    EXPECT_DOUBLE_EQ(beatLengthCrochet, firstBeatLength);

    FrameDiff_t ninthBeatLength =
            pBeats->getBeatAtIndex(9).framePosition() -
            pBeats->getBeatAtIndex(8).framePosition();
    EXPECT_DOUBLE_EQ(beatLengthCrochet, ninthBeatLength);

    FrameDiff_t tenthBeatLength =
            pBeats->getBeatAtIndex(10).framePosition() -
            pBeats->getBeatAtIndex(9).framePosition();
    EXPECT_DOUBLE_EQ(beatLengthCrochet / 2, tenthBeatLength);

    FrameDiff_t twentyFifthBeatLength =
            pBeats->getBeatAtIndex(25).framePosition() -
            pBeats->getBeatAtIndex(24).framePosition();
    EXPECT_DOUBLE_EQ(beatLengthCrochet / 2, twentyFifthBeatLength);

    FrameDiff_t twentySixthBeatLength =
            pBeats->getBeatAtIndex(26).framePosition() -
            pBeats->getBeatAtIndex(25).framePosition();
    EXPECT_DOUBLE_EQ(beatLengthCrochet * 2, twentySixthBeatLength);
}

TEST_F(BeatsTest, Iterator) {
    const auto& pBeats = m_pTrack1->getBeats();
    FramePos pos;

    // Full Beatsbeat
    auto iter1 = pBeats->findBeats(pBeats->getFirstBeatPosition(),
            pBeats->getLastBeatPosition());
    EXPECT_DOUBLE_EQ(iter1->next().framePosition().getValue(),
            pBeats->getFirstBeatPosition().getValue());
    while (iter1->hasNext()) {
        auto beat = iter1->next();
        pos = FramePos(beat.framePosition().getValue());
        EXPECT_TRUE(pos.getValue());
    }
    EXPECT_DOUBLE_EQ(
            pos.getValue(), pBeats->getLastBeatPosition().getValue());

    // Past end
    auto iter2 = pBeats->findBeats(pBeats->getFirstBeatPosition(),
            FramePos(
                    pBeats->getLastBeatPosition().getValue() + 10000000000));
    while (iter2->hasNext()) {
        auto beat = iter2->next();
        pos = FramePos(beat.framePosition().getValue());
        EXPECT_TRUE(pos.getValue());
    }
    EXPECT_DOUBLE_EQ(
            pos.getValue(), pBeats->getLastBeatPosition().getValue());

    // Before begining
    auto iter3 = pBeats->findBeats(
            FramePos(pBeats->getFirstBeatPosition().getValue() - 1000000),
            pBeats->getLastBeatPosition());
    EXPECT_DOUBLE_EQ(iter3->next().framePosition().getValue(),
            pBeats->getFirstBeatPosition().getValue());
    while (iter3->hasNext()) {
        auto beat = iter3->next();
        pos = FramePos(beat.framePosition().getValue());
        EXPECT_TRUE(pos.getValue());
    }
    EXPECT_DOUBLE_EQ(pos.getValue(), pBeats->getLastBeatPosition().getValue());
}

TEST_F(BeatsTest, Translate) {
    const auto& pBeats1 = m_pTrack1->getBeats();
    const auto& pBeats2 = m_pTrack2->getBeats();
    FrameDiff_t delta = 500;

    // Move the grid delta frames
    pBeats1->translate(delta);

    // All beats must have been displaced by delta frames
    auto iter1 = pBeats1->findBeats(pBeats1->getFirstBeatPosition(),
            pBeats1->getLastBeatPosition());
    auto iter2 = pBeats2->findBeats(pBeats2->getFirstBeatPosition(),
            pBeats2->getLastBeatPosition());
    while (iter1->hasNext()) {
        double pos1 = iter1->next().framePosition().getValue();
        double pos2 = iter2->next().framePosition().getValue();
        EXPECT_DOUBLE_EQ(pos1, pos2 + delta);
    }
    //    EXPECT_EQ(iter1->hasNext(), iter2->hasNext());
}

TEST_F(BeatsTest, FindClosest) {
    const auto& pBeats = m_pTrack1->getBeats();
    // Test deltas ranging from previous beat to next beat
    for (FrameDiff_t delta = -m_iSampleRate; delta <= m_iSampleRate; delta++) {
        auto iter1 = pBeats->findBeats(pBeats->getFirstBeatPosition(),
                pBeats->getLastBeatPosition());
        while (iter1->hasNext()) {
            FramePos pos = FramePos(iter1->next().framePosition().getValue());
            FramePos foundPos = pBeats->findClosestBeat(pos + delta);
            // Correct change of beat
            FramePos expectedPos = pos +
                    (delta > (m_iSampleRate / 2.0) ? m_iSampleRate : 0) +
                    (delta < (-m_iSampleRate / 2.0) ? -m_iSampleRate : 0);
            // Enforce boundaries
            expectedPos = std::min(expectedPos, pBeats->getLastBeatPosition());
            expectedPos = std::max(expectedPos, pBeats->getFirstBeatPosition());
            EXPECT_DOUBLE_EQ(foundPos.getValue(), expectedPos.getValue());
        }
        break;
    }
}

TEST_F(BeatsTest, ChangingTimeSignatureShouldNotChangeBpm) {
    const auto& pBeats = m_pTrack1->getBeats();
    // Set the track to have multiple BPM values.
    pBeats->setBpm(Bpm(60), 0);
    pBeats->setBpm(Bpm(120), 5);
    pBeats->setBpm(Bpm(240), 10);
    pBeats->setBpm(Bpm(75), 15);
    // Let's test global BPM first
    auto oldBpm = pBeats->getGlobalBpm();
    auto oldTimeSignature = pBeats->getBeatAtIndex(0).timeSignature();
    pBeats->setSignature(TimeSignature(oldTimeSignature.getBeatsPerBar(),
                                 oldTimeSignature.getNoteValue() * 2),
            0);
    auto newBpm = pBeats->getGlobalBpm();

    ASSERT_EQ(oldBpm, newBpm);
}
} // namespace
