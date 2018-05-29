#include <gtest/gtest.h>

#include "test/mixxxtest.h"

#include "analyzer/analyzersilence.h"
#include "engine/engine.h"

namespace {

constexpr mixxx::AudioSignal::ChannelCount kChannelCount = mixxx::kEngineChannelCount;
constexpr int kTrackLengthFrames = 100000;
constexpr double kTonePitchHz = 1000.0;  // 1kHz

class AnalyzerSilenceTest : public MixxxTest {
  protected:
    AnalyzerSilenceTest()
        : analyzerSilence(config()) {
    }

    void SetUp() override {
        pTrack = Track::newTemporary();
        pTrack->setSampleRate(44100);

        nTrackSampleDataLength = kChannelCount * kTrackLengthFrames;
        pTrackSampleData = new CSAMPLE[nTrackSampleDataLength];
    }

    void TearDown() override {
        delete [] pTrackSampleData;
    }

    void analyzeTrack() {
        analyzerSilence.initialize(pTrack, pTrack->getSampleRate(), nTrackSampleDataLength);
        analyzerSilence.process(pTrackSampleData, nTrackSampleDataLength);
        analyzerSilence.finalize(pTrack);
    }

  protected:
    AnalyzerSilence analyzerSilence;
    TrackPointer pTrack;
    CSAMPLE* pTrackSampleData;
    int nTrackSampleDataLength;  // in samples
};

TEST_F(AnalyzerSilenceTest, SilenceTrack) {
    // Fill the entire buffer with silence
    for (int i = 0; i < nTrackSampleDataLength; i++) {
        pTrackSampleData[i] = 0.0;
    }

    analyzeTrack();

    CuePointer pBeginCue = pTrack->findCueByType(Cue::BEGIN);
    EXPECT_EQ(pBeginCue->getPosition(), 0.0);
    EXPECT_EQ(pBeginCue->getSource(), Cue::AUTOMATIC);

    CuePointer pEndCue = pTrack->findCueByType(Cue::END);
    EXPECT_EQ(pEndCue->getPosition(), nTrackSampleDataLength);
    EXPECT_EQ(pEndCue->getSource(), Cue::AUTOMATIC);
}

TEST_F(AnalyzerSilenceTest, EndToEndToneTrack) {
    // Fill the entire buffer with 1 kHz tone
    double omega = 2.0 * M_PI * kTonePitchHz / pTrack->getSampleRate();
    for (int i = 0; i < nTrackSampleDataLength; i++) {
        pTrackSampleData[i] = cos(i / kChannelCount * omega);
    }

    analyzeTrack();

    CuePointer pBeginCue = pTrack->findCueByType(Cue::BEGIN);
    EXPECT_EQ(pBeginCue->getPosition(), 0.0);
    EXPECT_EQ(pBeginCue->getSource(), Cue::AUTOMATIC);

    CuePointer pEndCue = pTrack->findCueByType(Cue::END);
    EXPECT_EQ(pEndCue->getPosition(), nTrackSampleDataLength);
    EXPECT_EQ(pEndCue->getSource(), Cue::AUTOMATIC);
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

    CuePointer pBeginCue = pTrack->findCueByType(Cue::BEGIN);
    EXPECT_EQ(pBeginCue->getPosition(), nTrackSampleDataLength / 4);
    EXPECT_EQ(pBeginCue->getSource(), Cue::AUTOMATIC);

    CuePointer pEndCue = pTrack->findCueByType(Cue::END);
    EXPECT_EQ(pEndCue->getPosition(), 3 * nTrackSampleDataLength / 4);
    EXPECT_EQ(pEndCue->getSource(), Cue::AUTOMATIC);
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

    CuePointer pBeginCue = pTrack->findCueByType(Cue::BEGIN);
    EXPECT_EQ(pBeginCue->getPosition(), oneFifthOfTrackLength);
    EXPECT_EQ(pBeginCue->getSource(), Cue::AUTOMATIC);

    CuePointer pEndCue = pTrack->findCueByType(Cue::END);
    EXPECT_EQ(pEndCue->getPosition(), 4 * oneFifthOfTrackLength);
    EXPECT_EQ(pEndCue->getSource(), Cue::AUTOMATIC);
}

}
