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

    CuePosition cue = pTrack->getCuePoint();
    EXPECT_DOUBLE_EQ(0.0, cue.getPosition());

    CuePointer pIntroCue = pTrack->findCueByType(mixxx::CueType::Intro);
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getPosition());
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getLength());

    CuePointer pOutroCue = pTrack->findCueByType(mixxx::CueType::Outro);
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, pOutroCue->getPosition());
    EXPECT_DOUBLE_EQ(nTrackSampleDataLength, pOutroCue->getLength());
}

TEST_F(AnalyzerSilenceTest, EndToEndToneTrack) {
    // Fill the entire buffer with 1 kHz tone
    double omega = 2.0 * M_PI * kTonePitchHz / pTrack->getSampleRate();
    for (int i = 0; i < nTrackSampleDataLength; i++) {
        pTrackSampleData[i] = cos(i / kChannelCount * omega);
    }

    analyzeTrack();

    CuePosition cue = pTrack->getCuePoint();
    EXPECT_DOUBLE_EQ(0.0, cue.getPosition());

    CuePointer pIntroCue = pTrack->findCueByType(mixxx::CueType::Intro);
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getPosition());
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getLength());

    CuePointer pOutroCue = pTrack->findCueByType(mixxx::CueType::Outro);
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, pOutroCue->getPosition());
    EXPECT_DOUBLE_EQ(nTrackSampleDataLength, pOutroCue->getLength());
}

TEST_F(AnalyzerSilenceTest, ToneTrackWithSilence) {
    // Fill the first quarter with silence
    for (int i = 0; i < nTrackSampleDataLength / 4; i++) {
        pTrackSampleData[i] = 0.0;
    }

    // Fill the middle with 1 kHz tone
    double omega = 2.0 * M_PI * kTonePitchHz / pTrack->getSampleRate();
    for (int i = nTrackSampleDataLength / 4; i < 3 * nTrackSampleDataLength / 4; i++) {
        pTrackSampleData[i] = cos(i / kChannelCount * omega);
    }

    // Fill the last quarter with silence
    for (int i = 3 * nTrackSampleDataLength / 4; i < nTrackSampleDataLength; i++) {
        pTrackSampleData[i] = 0.0;
    }

    analyzeTrack();

    CuePosition cue = pTrack->getCuePoint();
    EXPECT_DOUBLE_EQ(nTrackSampleDataLength / 4, cue.getPosition());

    CuePointer pIntroCue = pTrack->findCueByType(mixxx::CueType::Intro);
    EXPECT_DOUBLE_EQ(nTrackSampleDataLength / 4, pIntroCue->getPosition());
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getLength());

    CuePointer pOutroCue = pTrack->findCueByType(mixxx::CueType::Outro);
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, pOutroCue->getPosition());
    EXPECT_DOUBLE_EQ(3 * nTrackSampleDataLength / 4, pOutroCue->getLength());
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
        pTrackSampleData[i] = cos(i / kChannelCount * omega);
    }

    // Fill the third fifth with silence
    for (int i = 2 * oneFifthOfTrackLength; i < 3 * oneFifthOfTrackLength; i++) {
        pTrackSampleData[i] = 0.0;
    }

    // Fill the fourth fifth with 1 kHz tone
    for (int i = 3 * oneFifthOfTrackLength; i < 4 * oneFifthOfTrackLength; i++) {
        pTrackSampleData[i] = cos(i / kChannelCount * omega);
    }

    // Fill the fifth fifth with silence
    for (int i = 4 * oneFifthOfTrackLength; i < nTrackSampleDataLength; i++) {
        pTrackSampleData[i] = 0.0;
    }

    analyzeTrack();

    CuePosition cue = pTrack->getCuePoint();
    EXPECT_DOUBLE_EQ(oneFifthOfTrackLength, cue.getPosition());

    CuePointer pIntroCue = pTrack->findCueByType(mixxx::CueType::Intro);
    EXPECT_DOUBLE_EQ(oneFifthOfTrackLength, pIntroCue->getPosition());
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getLength());

    CuePointer pOutroCue = pTrack->findCueByType(mixxx::CueType::Outro);
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, pOutroCue->getPosition());
    EXPECT_DOUBLE_EQ(4 * oneFifthOfTrackLength, pOutroCue->getLength());
}

TEST_F(AnalyzerSilenceTest, RespectUserEdits) {
    // Arbitrary values
    const double kManualCuePosition = 0.2 * nTrackSampleDataLength;
    const double kManualIntroPosition = 0.1 * nTrackSampleDataLength;
    const double kManualOutroPosition = 0.9 * nTrackSampleDataLength;

    pTrack->setCuePoint(CuePosition(kManualCuePosition));

    CuePointer pIntroCue = pTrack->createAndAddCue();
    pIntroCue->setType(mixxx::CueType::Intro);
    pIntroCue->setStartPosition(kManualIntroPosition);
    pIntroCue->setEndPosition(Cue::kNoPosition);

    CuePointer pOutroCue = pTrack->createAndAddCue();
    pOutroCue->setType(mixxx::CueType::Outro);
    pOutroCue->setStartPosition(Cue::kNoPosition);
    pOutroCue->setEndPosition(kManualOutroPosition);

    // Fill the first half with silence
    for (int i = 0; i < nTrackSampleDataLength / 2; i++) {
        pTrackSampleData[i] = 0.0;
    }

    // Fill the second half with 1 kHz tone
    double omega = 2.0 * M_PI * kTonePitchHz / pTrack->getSampleRate();
    for (int i = nTrackSampleDataLength / 2; i < nTrackSampleDataLength; i++) {
        pTrackSampleData[i] = sin(i / kChannelCount * omega);
    }

    analyzeTrack();

    CuePosition cue = pTrack->getCuePoint();
    EXPECT_DOUBLE_EQ(kManualCuePosition, cue.getPosition());

    EXPECT_DOUBLE_EQ(kManualIntroPosition, pIntroCue->getPosition());
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getLength());

    EXPECT_DOUBLE_EQ(Cue::kNoPosition, pOutroCue->getPosition());
    EXPECT_DOUBLE_EQ(kManualOutroPosition, pOutroCue->getLength());
}

} // namespace
