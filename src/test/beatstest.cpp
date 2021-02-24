#include <gtest/gtest.h>

#include <QtDebug>

#include "track/beats.h"
#include "track/track.h"
#include "util/math.h"
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

    double framesToSeconds(audio::SampleRate sampleRate, FrameDiff_t frames) {
        return frames / sampleRate;
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
    const auto pBeats = m_pTrack1->getBeats();
    // Initially must be the base value
    EXPECT_EQ(m_bpm, pBeats->getGlobalBpm());

    pBeats->scale(BPMScale::Double);
    EXPECT_EQ(m_bpm * 2, pBeats->getGlobalBpm());

    pBeats->scale(BPMScale::Halve);
    EXPECT_EQ(m_bpm, pBeats->getGlobalBpm());

    pBeats->scale(BPMScale::TwoThirds);
    EXPECT_EQ(m_bpm * 2 / 3, pBeats->getGlobalBpm());

    pBeats->scale(BPMScale::ThreeHalves);
    EXPECT_EQ(m_bpm, pBeats->getGlobalBpm());

    pBeats->scale(BPMScale::ThreeFourths);
    EXPECT_EQ(m_bpm * 3 / 4, pBeats->getGlobalBpm());

    pBeats->scale(BPMScale::FourThirds);
    EXPECT_EQ(m_bpm, pBeats->getGlobalBpm());
}

TEST_F(BeatsTest, NthBeat) {
    const auto pBeats = m_pTrack1->getBeats();

    // Check edge cases
    EXPECT_EQ(pBeats->getLastBeatPosition(),
            pBeats->findNthBeat(pBeats->getLastBeatPosition(), 1)->framePosition());
    EXPECT_EQ(pBeats->getLastBeatPosition(),
            pBeats->findNthBeat(pBeats->getLastBeatPosition(), -1)->framePosition());
    const auto& lastBeat = pBeats->getBeatAtIndex(pBeats->size() - 1);
    EXPECT_EQ(lastBeat->framePosition() + getBeatLengthFrames(lastBeat->bpm()),
            pBeats->findNthBeat(pBeats->getLastBeatPosition(), 2)->framePosition());
    EXPECT_EQ(pBeats->getFirstBeatPosition(),
            pBeats->findNthBeat(pBeats->getFirstBeatPosition(), -1)->framePosition());
    EXPECT_EQ(pBeats->getFirstBeatPosition(),
            pBeats->findNthBeat(pBeats->getFirstBeatPosition(), -1)->framePosition());
    const auto& firstBeat = pBeats->getBeatAtIndex(0);
    EXPECT_EQ(firstBeat->framePosition() - getBeatLengthFrames(firstBeat->bpm()),
            pBeats->findNthBeat(pBeats->getFirstBeatPosition(), -2)->framePosition());

    // Use beat before track as pivot
    const auto& beforeBeat = pBeats->getBeatAtIndex(-3);
    EXPECT_EQ(beforeBeat->framePosition(),
            pBeats->findNthBeat(beforeBeat->framePosition(), -1)->framePosition());
    EXPECT_EQ(beforeBeat->framePosition(),
            pBeats->findNthBeat(beforeBeat->framePosition(), 1)->framePosition());
    EXPECT_EQ(beforeBeat->framePosition() - getBeatLengthFrames(firstBeat->bpm()),
            pBeats->findNthBeat(beforeBeat->framePosition(), -2)->framePosition());
    EXPECT_EQ(beforeBeat->framePosition() + getBeatLengthFrames(firstBeat->bpm()),
            pBeats->findNthBeat(beforeBeat->framePosition(), 2)->framePosition());
    // Try to find beat from pivot within the track
    EXPECT_EQ(beforeBeat->framePosition() + 9 * getBeatLengthFrames(firstBeat->bpm()),
            pBeats->findNthBeat(beforeBeat->framePosition(), 10)->framePosition());

    // Use beat after track as pivot
    const auto& afterBeat = pBeats->getBeatAtIndex(pBeats->size() + 3);
    EXPECT_EQ(afterBeat->framePosition(),
            pBeats->findNthBeat(afterBeat->framePosition(), -1)->framePosition());
    EXPECT_EQ(afterBeat->framePosition(),
            pBeats->findNthBeat(afterBeat->framePosition(), 1)->framePosition());
    EXPECT_EQ(afterBeat->framePosition() - getBeatLengthFrames(lastBeat->bpm()),
            pBeats->findNthBeat(afterBeat->framePosition(), -2)->framePosition());
    EXPECT_EQ(afterBeat->framePosition() + getBeatLengthFrames(lastBeat->bpm()),
            pBeats->findNthBeat(afterBeat->framePosition(), 2)->framePosition());
    // Try to find beat from pivot within the track
    EXPECT_EQ(afterBeat->framePosition() - 9 * getBeatLengthFrames(lastBeat->bpm()),
            pBeats->findNthBeat(afterBeat->framePosition(), -10)->framePosition());
}

TEST_F(BeatsTest, PrevNextBeats) {
    const auto pBeats = m_pTrack1->getBeats();
    auto prevNextBeats = pBeats->findPrevNextBeats(pBeats->getLastBeatPosition());
    auto prevBeat = prevNextBeats.first->framePosition();
    auto nextBeat = prevNextBeats.second->framePosition();
    EXPECT_DOUBLE_EQ(
            pBeats->getLastBeatPosition().getValue(), prevBeat.getValue());

    prevNextBeats = pBeats->findPrevNextBeats(pBeats->getFirstBeatPosition());
    prevBeat = prevNextBeats.first->framePosition();
    nextBeat = prevNextBeats.second->framePosition();
    EXPECT_DOUBLE_EQ(
            pBeats->getFirstBeatPosition().getValue(), prevBeat.getValue());
    EXPECT_DOUBLE_EQ(
            (pBeats->getFirstBeatPosition() + getBeatLengthFrames(m_bpm))
                    .getValue(),
            nextBeat.getValue());
}

TEST_F(BeatsTest, NthBeatWhenOnBeat) {
    const auto pBeats = m_pTrack1->getBeats();
    // Pretend we're on the 20th beat;
    const int curBeat = 20;
    const FrameDiff_t beatLength = getBeatLengthFrames(m_bpm);
    FramePos position = m_startOffsetFrames + beatLength * curBeat;

    // The spec dictates that a value of 0 is always invalid
    EXPECT_EQ(std::nullopt, pBeats->findNthBeat(position, 0));

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    const int spanBeats = pBeats->size() * 2;
    for (int i = 1; i < spanBeats; ++i) {
        EXPECT_DOUBLE_EQ(
                (position + beatLength * (i - 1)).getValue(),
                pBeats->findNthBeat(position, i)->framePosition().getValue());
        EXPECT_DOUBLE_EQ(
                (position + beatLength * (-i + 1)).getValue(),
                pBeats->findNthBeat(position, -i)->framePosition().getValue());
    }

    // Also test prev/next beat calculation.
    const auto prevNextBeats = pBeats->findPrevNextBeats(position);
    const auto prevBeat = prevNextBeats.first->framePosition();
    const auto nextBeat = prevNextBeats.second->framePosition();
    EXPECT_EQ(position, prevBeat);
    EXPECT_EQ(position + beatLength, nextBeat);

    // Both previous and next beat should return the current position.
    EXPECT_EQ(position, pBeats->findNextBeat(position)->framePosition());
    EXPECT_EQ(position, pBeats->findPrevBeat(position)->framePosition());
}

TEST_F(BeatsTest, NthBeatWhenOnBeat_BeforeEpsilon) {
    const auto pBeats = m_pTrack1->getBeats();
    // Pretend we're just before the 20th beat;
    const int curBeat = 20;
    const FrameDiff_t beatLength = getBeatLengthFrames(m_bpm);
    const FramePos kClosestBeat = m_startOffsetFrames + curBeat * beatLength;
    FramePos position = kClosestBeat - beatLength * 0.005;

    // The spec dictates that a value of 0 is always invalid and returns empty beat.
    EXPECT_EQ(std::nullopt, pBeats->findNthBeat(position, 0));

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    const int spanBeats = pBeats->size() * 2;
    for (int i = 1; i < spanBeats; ++i) {
        EXPECT_DOUBLE_EQ((kClosestBeat + beatLength * (i - 1)).getValue(),
                pBeats->findNthBeat(position, i)->framePosition().getValue());
        EXPECT_DOUBLE_EQ((kClosestBeat + beatLength * (-i + 1)).getValue(),
                pBeats->findNthBeat(position, -i)->framePosition().getValue());
    }

    // Also test prev/next beat calculation
    const auto prevNextBeats = pBeats->findPrevNextBeats(position);
    const auto prevBeat = prevNextBeats.first->framePosition();
    const auto nextBeat = prevNextBeats.second->framePosition();
    EXPECT_EQ(kClosestBeat, prevBeat);
    EXPECT_EQ(kClosestBeat + beatLength, nextBeat);

    // Both previous and next beat should return the closest beat.
    EXPECT_EQ(kClosestBeat, pBeats->findNextBeat(position)->framePosition());
    EXPECT_EQ(kClosestBeat, pBeats->findPrevBeat(position)->framePosition());
}

TEST_F(BeatsTest, NthBeatWhenOnBeat_AfterEpsilon) {
    const auto pBeats = m_pTrack1->getBeats();
    // Pretend we're just after the 20th beat;
    const int curBeat = 20;
    const FrameDiff_t beatLength = getBeatLengthFrames(m_bpm);
    const FramePos kClosestBeat = m_startOffsetFrames + curBeat * beatLength;
    FramePos position =
            kClosestBeat + beatLength * 0.005;

    // The spec dictates that a value of 0 is always invalid
    EXPECT_EQ(std::nullopt, pBeats->findNthBeat(position, 0));

    EXPECT_EQ(kClosestBeat, pBeats->findClosestBeat(position));

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    const int spanBeats = pBeats->size() * 2;
    for (int i = 1; i < spanBeats; ++i) {
        EXPECT_DOUBLE_EQ(
                (kClosestBeat + beatLength * (i - 1)).getValue(),
                pBeats->findNthBeat(position, i)->framePosition().getValue());
        EXPECT_DOUBLE_EQ(
                (kClosestBeat + beatLength * (-i + 1)).getValue(),
                pBeats->findNthBeat(position, -i)->framePosition().getValue());
    }

    // Also test prev/next beat calculation.
    const auto prevNextBeats = pBeats->findPrevNextBeats(position);
    const auto prevBeat = prevNextBeats.first->framePosition();
    const auto nextBeat = prevNextBeats.second->framePosition();
    EXPECT_EQ(kClosestBeat, prevBeat);
    EXPECT_EQ(kClosestBeat + beatLength, nextBeat);

    // Both previous and next beat should return the closest beat.
    EXPECT_EQ(kClosestBeat, pBeats->findNextBeat(position)->framePosition());
    EXPECT_EQ(kClosestBeat, pBeats->findPrevBeat(position)->framePosition());
}

TEST_F(BeatsTest, NthBeatWhenNotOnBeat) {
    const auto pBeats = m_pTrack1->getBeats();
    const FrameDiff_t beatLength = getBeatLengthFrames(m_bpm);
    // Pretend we're half way between the 20th and 21st beat
    FramePos previousBeat =
            m_startOffsetFrames + beatLength * 20.0;
    FramePos nextBeat =
            m_startOffsetFrames + beatLength * 21.0;
    FramePos position = FramePos((previousBeat.getValue() + nextBeat.getValue()) / 2);

    // The spec dictates that a value of 0 is always invalid
    EXPECT_EQ(std::nullopt, pBeats->findNthBeat(position, 0));

    // findNthBeat should return multiples of beats starting from the next or
    // previous beat, depending on whether N is positive or negative.
    const int spanBeats = pBeats->size() * 2;
    for (int i = 1; i < spanBeats; ++i) {
        EXPECT_DOUBLE_EQ(
                (nextBeat + beatLength * (i - 1))
                        .getValue(),
                pBeats->findNthBeat(position, i)->framePosition().getValue());
        EXPECT_DOUBLE_EQ(
                (previousBeat - beatLength * (i - 1))
                        .getValue(),
                pBeats->findNthBeat(position, -i)->framePosition().getValue());
    }

    // Also test prev/next beat calculation
    const auto prevNextBeats = pBeats->findPrevNextBeats(position);
    const auto foundPrevBeat = prevNextBeats.first->framePosition();
    const auto foundNextBeat = prevNextBeats.second->framePosition();
    EXPECT_EQ(previousBeat, foundPrevBeat);
    EXPECT_EQ(nextBeat, foundNextBeat);
}

TEST_F(BeatsTest, InstantaneousBpm) {
    const auto pBeats = m_pTrack1->getBeats();
    const FrameDiff_t approxBeatLengthFrames = getBeatLengthFrames(m_bpm);
    const int numBeats = 64;

    // Constant BPM, constructed in BeatsTest
    for (unsigned int i = 0; i < 100; i++) {
        EXPECT_EQ(m_bpm, pBeats->getBpmAtPosition(FramePos(i)));
    }

    // Check before the start of the track.
    EXPECT_EQ(m_bpm, pBeats->getBpmAtPosition(m_startOffsetFrames - 100));

    // Prepare a new Beats to test the behavior for variable BPM
    QVector<FramePos> beats;
    FramePos beat_pos;
    Bpm bpm = m_bpm;
    for (unsigned int i = 0; i < numBeats; ++i, bpm = bpm + 1) {
        FrameDiff_t beat_length = getBeatLengthFrames(bpm);
        beats.append(beat_pos);
        beat_pos += beat_length;
    }

    m_pTrack1->setDuration(mixxx::Duration::fromSeconds(beat_pos.getValue() / m_iSampleRate));
    pBeats->initWithAnalyzer(beats);

    // Test within the track.
    EXPECT_TRUE(qFuzzyCompare(64,
            pBeats->getBpmAtPosition(kStartFramePos + approxBeatLengthFrames * 4).getValue()));
    EXPECT_TRUE(qFuzzyCompare(100,
            pBeats->getBpmAtPosition(kStartFramePos + approxBeatLengthFrames * 31).getValue()));
    // Before
    EXPECT_TRUE(qFuzzyCompare(60,
            pBeats->getBpmAtPosition(kStartFramePos - 1000).getValue()));
    // Beginning
    EXPECT_TRUE(qFuzzyCompare(60,
            pBeats->getBpmAtPosition(kStartFramePos).getValue()));
    // End
    EXPECT_TRUE(qFuzzyCompare(m_bpm.getValue() + numBeats - 2,
            pBeats->getBpmAtPosition(beats.last()).getValue()));
    // Beyond the end
    EXPECT_TRUE(qFuzzyCompare(m_bpm.getValue() + numBeats - 2,
            pBeats->getBpmAtPosition(beats.last() + 1000).getValue()));

    // Try a really, really short track
    auto startOffset = FramePos(10);
    beats = createBeatVector(startOffset, 3, getBeatLengthFrames(m_bpm));
    m_pTrack1->setDuration(beats.last().getValue() / m_iSampleRate);
    pBeats->initWithAnalyzer(beats);
    EXPECT_TRUE(qFuzzyCompare(m_bpm.getValue(),
            pBeats->getBpmAtPosition(
                          startOffset + approxBeatLengthFrames * 1)
                    .getValue()));
}

TEST_F(BeatsTest, Signature) {
    const auto pBeats = m_pTrack1->getBeats();
    // Undefined time signature must be default
    EXPECT_EQ(pBeats->getBeatAtIndex(0)->timeSignature(),
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

    EXPECT_EQ(pBeats->getBeatAtIndex(0)->timeSignature(),
            timeSignatureInitial);
    EXPECT_EQ(pBeats->getBeatAtIndex(1)->timeSignature(),
            timeSignatureInitial);
    EXPECT_EQ(pBeats->getBeatAtIndex(5)->timeSignature(),
            timeSignatureInitial);
    EXPECT_EQ(pBeats->getBeatAtIndex(9)->timeSignature(),
            timeSignatureIntermediate);
    EXPECT_EQ(pBeats->getBeatAtIndex(15)->timeSignature(),
            timeSignatureIntermediate);
    EXPECT_EQ(pBeats->getBeatAtIndex(25)->timeSignature(),
            timeSignatureLater);
    EXPECT_EQ(pBeats->getBeatAtIndex(40)->timeSignature(),
            timeSignatureLater);

    EXPECT_EQ(pBeats->getBeatAtIndex(3)->type(), mixxx::BeatType::Downbeat);
    EXPECT_EQ(pBeats->getBeatAtIndex(9)->type(), mixxx::BeatType::Downbeat);
    EXPECT_EQ(pBeats->getBeatAtIndex(13)->type(), mixxx::BeatType::Downbeat);
    EXPECT_EQ(pBeats->getBeatAtIndex(25)->type(), mixxx::BeatType::Downbeat);
    EXPECT_EQ(pBeats->getBeatAtIndex(30)->type(), mixxx::BeatType::Downbeat);

    FrameDiff_t beatLengthCrochet = getBeatLengthFrames(m_bpm);

    FrameDiff_t firstBeatLength =
            pBeats->getBeatAtIndex(1)->framePosition() -
            pBeats->getBeatAtIndex(0)->framePosition();
    EXPECT_DOUBLE_EQ(beatLengthCrochet, firstBeatLength);

    FrameDiff_t ninthBeatLength =
            pBeats->getBeatAtIndex(9)->framePosition() -
            pBeats->getBeatAtIndex(8)->framePosition();
    EXPECT_DOUBLE_EQ(beatLengthCrochet, ninthBeatLength);

    FrameDiff_t tenthBeatLength =
            pBeats->getBeatAtIndex(10)->framePosition() -
            pBeats->getBeatAtIndex(9)->framePosition();
    EXPECT_DOUBLE_EQ(beatLengthCrochet / 2, tenthBeatLength);

    FrameDiff_t twentyFifthBeatLength =
            pBeats->getBeatAtIndex(25)->framePosition() -
            pBeats->getBeatAtIndex(24)->framePosition();
    EXPECT_DOUBLE_EQ(beatLengthCrochet / 2, twentyFifthBeatLength);

    FrameDiff_t twentySixthBeatLength =
            pBeats->getBeatAtIndex(26)->framePosition() -
            pBeats->getBeatAtIndex(25)->framePosition();
    EXPECT_DOUBLE_EQ(beatLengthCrochet * 2, twentySixthBeatLength);
}

TEST_F(BeatsTest, Translate) {
    const auto pBeats1 = m_pTrack1->getBeats();
    const auto pBeats2 = m_pTrack2->getBeats();
    FrameDiff_t delta = 500;

    // Move the grid delta frames
    pBeats1->translateBySeconds(framesToSeconds(m_pTrack1->getSampleRate(), delta));

    // All beats must have been displaced by delta frames

    for (int i = 0; i < pBeats1->size(); i++) {
        double pos1 = pBeats1->getBeatAtIndex(i)->framePosition().getValue();
        double pos2 = pBeats2->getBeatAtIndex(i)->framePosition().getValue();
        EXPECT_DOUBLE_EQ(pos1, pos2 + delta);
    }
}

TEST_F(BeatsTest, FindClosest) {
    const auto pBeats = m_pTrack1->getBeats();
    const FrameDiff_t beatLength = getBeatLengthFrames(m_bpm);
    // Test deltas ranging from previous beat to next beat
    for (FrameDiff_t delta = -0.9 * beatLength; delta <= 0.9 * beatLength;
            delta += 0.1 * beatLength) {
        for (int i = -pBeats->size(); i < 2 * pBeats->size(); i++) {
            const auto pos = pBeats->getBeatAtIndex(i)->framePosition();
            const auto foundPos = pBeats->findClosestBeat(pos + delta);
            FramePos expectedPos = pos +
                    (delta >= (beatLength / 2.0) ? beatLength : 0) +
                    (delta < (-beatLength / 2.0) ? -beatLength : 0);
            EXPECT_DOUBLE_EQ(foundPos.getValue(), expectedPos.getValue()) << delta;
        }
    }
}

TEST_F(BeatsTest, ChangingTimeSignatureShouldNotChangeBpm) {
    const auto pBeats = m_pTrack1->getBeats();
    // Set the track to have multiple BPM values.
    pBeats->setBpm(Bpm(60), 0);
    pBeats->setBpm(Bpm(120), 5);
    pBeats->setBpm(Bpm(240), 10);
    pBeats->setBpm(Bpm(75), 15);
    // Let's test global BPM first
    auto oldBpm = pBeats->getGlobalBpm();
    auto oldTimeSignature = pBeats->getBeatAtIndex(0)->timeSignature();
    pBeats->setSignature(TimeSignature(oldTimeSignature.getBeatsPerBar(),
                                 oldTimeSignature.getNoteValue() * 2),
            0);
    auto newBpm = pBeats->getGlobalBpm();

    ASSERT_EQ(oldBpm, newBpm);
}

TEST_F(BeatsTest, IndexRetrieval) {
    const auto pBeats = m_pTrack1->getBeats();
    const FrameDiff_t beatLengthFrames = getBeatLengthFrames(m_bpm);

    // We assume 4/4 throughout the track

    // Shift first downbeat ahead
    const int firstDownbeatIndex = 1;
    pBeats->setAsDownbeat(firstDownbeatIndex);
    const int startingBeatIndex = -10;
    const auto& timeSignature = pBeats->getBeatAtIndex(0)->timeSignature();
    for (int beatIndex = startingBeatIndex; beatIndex < pBeats->size() + 10; beatIndex++) {
        const auto& beat = pBeats->getBeatAtIndex(beatIndex);
        EXPECT_NE(std::nullopt, beat);
        EXPECT_EQ(beat->beatIndex(), beatIndex);
        EXPECT_EQ(beat->barIndex(),
                std::floor(static_cast<double>(beatIndex - firstDownbeatIndex) /
                        timeSignature.getBeatsPerBar()));
        const uint beatInBarIndex = clockModulo(
                beatIndex - firstDownbeatIndex, timeSignature.getBeatsPerBar());
        EXPECT_EQ(beatInBarIndex, beat->beatInBarIndex());
        EXPECT_EQ(beat->type(), beatInBarIndex == 0 ? BeatType::Downbeat : BeatType::Beat);
        EXPECT_DOUBLE_EQ(beat->framePosition().getValue(),
                m_startOffsetFrames.getValue() + beatLengthFrames * beatIndex);
        if (beatIndex == kFirstBeatIndex) {
            EXPECT_EQ(beat->markers(), BeatMarkers(BeatMarker::Bpm));
        } else if (beatIndex == firstDownbeatIndex) {
            EXPECT_EQ(beat->markers(), BeatMarkers(BeatMarker::TimeSignature));
        } else {
            EXPECT_EQ(beat->markers(), BeatMarkers(BeatMarker::None));
        }
    }

    // Track with empty beats should give std::nullopt when finding beats.
    TrackPointer emptyTrack = Track::newTemporary();
    emptyTrack->setBeats(BeatsInternal());
    EXPECT_EQ(std::nullopt, emptyTrack->getBeats()->getBeatAtIndex(0));
}
} // namespace
