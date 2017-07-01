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

    CuePosition cue = pTrack->getCuePoint();
    EXPECT_DOUBLE_EQ(0.0, cue.getPosition());
    EXPECT_EQ(Cue::UNKNOWN, cue.getSource());

    CuePointer pBeginCue = pTrack->findCueByType(Cue::BEGIN);
    EXPECT_DOUBLE_EQ(0.0, pBeginCue->getPosition());
    EXPECT_EQ(Cue::AUTOMATIC, pBeginCue->getSource());

    CuePointer pEndCue = pTrack->findCueByType(Cue::END);
    EXPECT_DOUBLE_EQ(nTrackSampleDataLength, pEndCue->getPosition());
    EXPECT_EQ(Cue::AUTOMATIC, pEndCue->getSource());
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
    EXPECT_EQ(Cue::UNKNOWN, cue.getSource());

    CuePointer pBeginCue = pTrack->findCueByType(Cue::BEGIN);
    EXPECT_DOUBLE_EQ(0.0, pBeginCue->getPosition());
    EXPECT_EQ(Cue::AUTOMATIC, pBeginCue->getSource());

    CuePointer pEndCue = pTrack->findCueByType(Cue::END);
    EXPECT_DOUBLE_EQ(nTrackSampleDataLength, pEndCue->getPosition());
    EXPECT_EQ(Cue::AUTOMATIC, pEndCue->getSource());
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
    EXPECT_EQ(Cue::AUTOMATIC, cue.getSource());

    CuePointer pBeginCue = pTrack->findCueByType(Cue::BEGIN);
    EXPECT_DOUBLE_EQ(nTrackSampleDataLength / 4, pBeginCue->getPosition());
    EXPECT_EQ(Cue::AUTOMATIC, pBeginCue->getSource());

    CuePointer pEndCue = pTrack->findCueByType(Cue::END);
    EXPECT_DOUBLE_EQ(3 * nTrackSampleDataLength / 4, pEndCue->getPosition());
    EXPECT_EQ(Cue::AUTOMATIC, pEndCue->getSource());
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
    EXPECT_EQ(Cue::AUTOMATIC, cue.getSource());

    CuePointer pBeginCue = pTrack->findCueByType(Cue::BEGIN);
    EXPECT_DOUBLE_EQ(oneFifthOfTrackLength, pBeginCue->getPosition());
    EXPECT_EQ(Cue::AUTOMATIC, pBeginCue->getSource());

    CuePointer pEndCue = pTrack->findCueByType(Cue::END);
    EXPECT_DOUBLE_EQ(4 * oneFifthOfTrackLength, pEndCue->getPosition());
    EXPECT_EQ(Cue::AUTOMATIC, pEndCue->getSource());
}

TEST_F(AnalyzerSilenceTest, UpdateNonUserAdjustedCues) {
    int halfTrackLength = nTrackSampleDataLength / 2;

    pTrack->setCuePoint(CuePosition(100, Cue::AUTOMATIC));  // Arbitrary value

    CuePointer pBeginCue = pTrack->createAndAddCue();
    pBeginCue->setType(Cue::BEGIN);
    pBeginCue->setSource(Cue::AUTOMATIC);
    pBeginCue->setPosition(1000);  // Arbitrary value

    CuePointer pEndCue = pTrack->createAndAddCue();
    pEndCue->setType(Cue::END);
    pEndCue->setSource(Cue::AUTOMATIC);
    pEndCue->setPosition(9000);  // Arbitrary value

    // Fill the first half with silence
    for (int i = 0; i < halfTrackLength; i++) {
        pTrackSampleData[i] = 0.0;
    }

    // Fill the second half with 1 kHz tone
    double omega = 2.0 * M_PI * kTonePitchHz / pTrack->getSampleRate();
    for (int i = halfTrackLength; i < nTrackSampleDataLength; i++) {
        pTrackSampleData[i] = sin(i / kChannelCount * omega);
    }

    analyzeTrack();

    CuePosition cue = pTrack->getCuePoint();
    EXPECT_DOUBLE_EQ(halfTrackLength, cue.getPosition());
    EXPECT_EQ(Cue::AUTOMATIC, cue.getSource());

    EXPECT_DOUBLE_EQ(halfTrackLength, pBeginCue->getPosition());
    EXPECT_EQ(Cue::AUTOMATIC, pBeginCue->getSource());

    EXPECT_DOUBLE_EQ(nTrackSampleDataLength, pEndCue->getPosition());
    EXPECT_EQ(Cue::AUTOMATIC, pEndCue->getSource());
}

TEST_F(AnalyzerSilenceTest, RespectUserEdits) {
    // Arbitrary values
    const double kManualCuePosition = 0.2 * nTrackSampleDataLength;
    const double kManualStartPosition = 0.1 * nTrackSampleDataLength;
    const double kManualEndPosition = 0.9 * nTrackSampleDataLength;

    pTrack->setCuePoint(CuePosition(kManualCuePosition, Cue::MANUAL));

    CuePointer pBeginCue = pTrack->createAndAddCue();
    pBeginCue->setType(Cue::BEGIN);
    pBeginCue->setSource(Cue::MANUAL);
    pBeginCue->setPosition(kManualStartPosition);

    CuePointer pEndCue = pTrack->createAndAddCue();
    pEndCue->setType(Cue::END);
    pEndCue->setSource(Cue::MANUAL);
    pEndCue->setPosition(kManualEndPosition);

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
    EXPECT_EQ(Cue::MANUAL, cue.getSource());

    EXPECT_DOUBLE_EQ(kManualStartPosition, pBeginCue->getPosition());
    EXPECT_EQ(Cue::MANUAL, pBeginCue->getSource());

    EXPECT_DOUBLE_EQ(kManualEndPosition, pEndCue->getPosition());
    EXPECT_EQ(Cue::MANUAL, pEndCue->getSource());
}

}
