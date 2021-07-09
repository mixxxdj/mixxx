#include "analyzer/analyzersilence.h"

#include <gtest/gtest.h>

#include "engine/engine.h"
#include "test/mixxxtest.h"
#include "track/track.h"

namespace {

constexpr mixxx::audio::ChannelCount kChannelCount = mixxx::kEngineChannelCount;
constexpr int kTrackLengthFrames = 100000;
constexpr double kTonePitchHz = 1000.0; // 1kHz

class AnalyzerSilenceTest : public MixxxTest {
  protected:
    AnalyzerSilenceTest()
            : analyzerSilence(config()) {
    }

    void SetUp() override {
        pTrack = Track::newTemporary();
        pTrack->setAudioProperties(
                mixxx::audio::ChannelCount(kChannelCount),
                mixxx::audio::SampleRate(44100),
                mixxx::audio::Bitrate(),
                mixxx::Duration::fromSeconds(kTrackLengthFrames / 44100.0));

        nTrackSampleDataLength = kChannelCount * kTrackLengthFrames;
        pTrackSampleData = new CSAMPLE[nTrackSampleDataLength];
    }

    void TearDown() override {
        delete[] pTrackSampleData;
    }

    void analyzeTrack() {
        analyzerSilence.initialize(pTrack, pTrack->getSampleRate(), nTrackSampleDataLength);
        analyzerSilence.processSamples(pTrackSampleData, nTrackSampleDataLength);
        analyzerSilence.storeResults(pTrack);
        analyzerSilence.cleanup();
    }

  protected:
    AnalyzerSilence analyzerSilence;
    TrackPointer pTrack;
    CSAMPLE* pTrackSampleData;
    int nTrackSampleDataLength; // in samples
};

TEST_F(AnalyzerSilenceTest, SilenceTrack) {
    // Fill the entire buffer with silence
    for (int i = 0; i < nTrackSampleDataLength; i++) {
        pTrackSampleData[i] = 0.0;
    }

    analyzeTrack();

    const mixxx::audio::FramePos cuePosition = pTrack->getMainCuePosition();
    EXPECT_EQ(mixxx::audio::kStartFramePos, cuePosition);

    CuePointer pIntroCue = pTrack->findCueByType(mixxx::CueType::Intro);
    EXPECT_EQ(mixxx::audio::kStartFramePos, pIntroCue->getPosition());
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getLengthFrames() * kChannelCount);

    CuePointer pOutroCue = pTrack->findCueByType(mixxx::CueType::Outro);
    EXPECT_EQ(mixxx::audio::kInvalidFramePos, pOutroCue->getPosition());
    EXPECT_DOUBLE_EQ(nTrackSampleDataLength, pOutroCue->getLengthFrames() * kChannelCount);
}

TEST_F(AnalyzerSilenceTest, EndToEndToneTrack) {
    // Fill the entire buffer with 1 kHz tone
    double omega = 2.0 * M_PI * kTonePitchHz / pTrack->getSampleRate();
    for (int i = 0; i < nTrackSampleDataLength; i++) {
        pTrackSampleData[i] = static_cast<CSAMPLE>(cos(i / kChannelCount * omega));
    }

    analyzeTrack();

    const mixxx::audio::FramePos cuePosition = pTrack->getMainCuePosition();
    EXPECT_EQ(mixxx::audio::kStartFramePos, cuePosition);

    CuePointer pIntroCue = pTrack->findCueByType(mixxx::CueType::Intro);
    EXPECT_EQ(mixxx::audio::kStartFramePos, pIntroCue->getPosition());
    EXPECT_DOUBLE_EQ(0.0 / kChannelCount, pIntroCue->getLengthFrames());

    CuePointer pOutroCue = pTrack->findCueByType(mixxx::CueType::Outro);
    EXPECT_EQ(mixxx::audio::kInvalidFramePos, pOutroCue->getPosition());
    EXPECT_DOUBLE_EQ(nTrackSampleDataLength / kChannelCount, pOutroCue->getLengthFrames());
}

TEST_F(AnalyzerSilenceTest, ToneTrackWithSilence) {
    // Fill the first quarter with silence
    for (int i = 0; i < nTrackSampleDataLength / 4; i++) {
        pTrackSampleData[i] = 0.0;
    }

    // Fill the middle with 1 kHz tone
    double omega = 2.0 * M_PI * kTonePitchHz / pTrack->getSampleRate();
    for (int i = nTrackSampleDataLength / 4; i < 3 * nTrackSampleDataLength / 4; i++) {
        pTrackSampleData[i] = static_cast<CSAMPLE>(cos(i / kChannelCount * omega));
    }

    // Fill the last quarter with silence
    for (int i = 3 * nTrackSampleDataLength / 4; i < nTrackSampleDataLength; i++) {
        pTrackSampleData[i] = 0.0;
    }

    analyzeTrack();

    const mixxx::audio::FramePos cuePosition = pTrack->getMainCuePosition();
    EXPECT_DOUBLE_EQ(nTrackSampleDataLength / 4, cuePosition.toEngineSamplePos());

    CuePointer pIntroCue = pTrack->findCueByType(mixxx::CueType::Intro);
    EXPECT_DOUBLE_EQ(nTrackSampleDataLength / 4, pIntroCue->getPosition().toEngineSamplePos());
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getLengthFrames());

    CuePointer pOutroCue = pTrack->findCueByType(mixxx::CueType::Outro);
    EXPECT_EQ(mixxx::audio::kInvalidFramePos, pOutroCue->getPosition());
    EXPECT_DOUBLE_EQ(3 * nTrackSampleDataLength / 4 / kChannelCount, pOutroCue->getLengthFrames());
}

TEST_F(AnalyzerSilenceTest, ToneTrackWithSilenceInTheMiddle) {
    double omega = 2.0 * M_PI * kTonePitchHz / pTrack->getSampleRate();
    int oneFifthOfTrackLength = nTrackSampleDataLength / 5;

    // Fill the first fifth with silence
    for (int i = 0; i < oneFifthOfTrackLength; i++) {
        pTrackSampleData[i] = 0.0;
    }

    // Fill the second fifth with 1 kHz tone
    for (int i = oneFifthOfTrackLength; i < 2 * oneFifthOfTrackLength; i++) {
        pTrackSampleData[i] = static_cast<CSAMPLE>(cos(i / kChannelCount * omega));
    }

    // Fill the third fifth with silence
    for (int i = 2 * oneFifthOfTrackLength; i < 3 * oneFifthOfTrackLength; i++) {
        pTrackSampleData[i] = 0.0;
    }

    // Fill the fourth fifth with 1 kHz tone
    for (int i = 3 * oneFifthOfTrackLength; i < 4 * oneFifthOfTrackLength; i++) {
        pTrackSampleData[i] = static_cast<CSAMPLE>(cos(i / kChannelCount * omega));
    }

    // Fill the fifth fifth with silence
    for (int i = 4 * oneFifthOfTrackLength; i < nTrackSampleDataLength; i++) {
        pTrackSampleData[i] = 0.0;
    }

    analyzeTrack();

    const mixxx::audio::FramePos cuePosition = pTrack->getMainCuePosition();
    EXPECT_DOUBLE_EQ(oneFifthOfTrackLength / kChannelCount, cuePosition.value());

    CuePointer pIntroCue = pTrack->findCueByType(mixxx::CueType::Intro);
    EXPECT_DOUBLE_EQ(oneFifthOfTrackLength, pIntroCue->getPosition().toEngineSamplePos());
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getLengthFrames());

    CuePointer pOutroCue = pTrack->findCueByType(mixxx::CueType::Outro);
    EXPECT_EQ(mixxx::audio::kInvalidFramePos, pOutroCue->getPosition());
    EXPECT_DOUBLE_EQ(4 * oneFifthOfTrackLength, pOutroCue->getLengthFrames() * kChannelCount);
}

TEST_F(AnalyzerSilenceTest, RespectUserEdits) {
    // Arbitrary values
    const auto kManualCuePosition = mixxx::audio::FramePos::fromEngineSamplePos(
            0.2 * nTrackSampleDataLength);
    const auto kManualIntroPosition =
            mixxx::audio::FramePos::fromEngineSamplePos(
                    0.1 * nTrackSampleDataLength);
    const auto kManualOutroPosition =
            mixxx::audio::FramePos::fromEngineSamplePos(
                    0.9 * nTrackSampleDataLength);

    pTrack->setMainCuePosition(kManualCuePosition);

    CuePointer pIntroCue = pTrack->createAndAddCue(
            mixxx::CueType::Intro,
            Cue::kNoHotCue,
            kManualIntroPosition,
            mixxx::audio::kInvalidFramePos);

    CuePointer pOutroCue = pTrack->createAndAddCue(
            mixxx::CueType::Outro,
            Cue::kNoHotCue,
            mixxx::audio::kInvalidFramePos,
            kManualOutroPosition);

    // Fill the first half with silence
    for (int i = 0; i < nTrackSampleDataLength / 2; i++) {
        pTrackSampleData[i] = 0.0;
    }

    // Fill the second half with 1 kHz tone
    double omega = 2.0 * M_PI * kTonePitchHz / pTrack->getSampleRate();
    for (int i = nTrackSampleDataLength / 2; i < nTrackSampleDataLength; i++) {
        pTrackSampleData[i] = static_cast<CSAMPLE>(sin(i / kChannelCount * omega));
    }

    analyzeTrack();

    mixxx::audio::FramePos cuePosition = pTrack->getMainCuePosition();
    EXPECT_EQ(kManualCuePosition, cuePosition);

    EXPECT_EQ(kManualIntroPosition, pIntroCue->getPosition());
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getLengthFrames());

    EXPECT_EQ(mixxx::audio::kInvalidFramePos, pOutroCue->getPosition());
    EXPECT_DOUBLE_EQ(kManualOutroPosition.value(), pOutroCue->getLengthFrames());
}

} // namespace
