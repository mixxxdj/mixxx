#include <gtest/gtest.h>

#include "test/mixxxtest.h"

#include "analyzer/analyzersilence.h"
#include "engine/engine.h"

namespace {

constexpr mixxx::AudioSignal::ChannelCount kChannelCount = mixxx::kEngineChannelCount;
constexpr int kTrackLengthFrames = 100000;
constexpr double kTonePitchHz = 1000.0; // 1kHz

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
    EXPECT_EQ(Cue::AUTOMATIC, cue.getSource());

    CuePointer pIntroCue = pTrack->findCueByType(Cue::INTRO);
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getPosition());
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getLength());
    EXPECT_EQ(Cue::AUTOMATIC, pIntroCue->getSource());

    CuePointer pOutroCue = pTrack->findCueByType(Cue::OUTRO);
    EXPECT_DOUBLE_EQ(-1.0, pOutroCue->getPosition());
    EXPECT_DOUBLE_EQ(nTrackSampleDataLength, pOutroCue->getLength());
    EXPECT_EQ(Cue::AUTOMATIC, pOutroCue->getSource());
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
    EXPECT_EQ(Cue::AUTOMATIC, cue.getSource());

    CuePointer pIntroCue = pTrack->findCueByType(Cue::INTRO);
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getPosition());
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getLength());
    EXPECT_EQ(Cue::AUTOMATIC, pIntroCue->getSource());

    CuePointer pOutroCue = pTrack->findCueByType(Cue::OUTRO);
    EXPECT_DOUBLE_EQ(-1.0, pOutroCue->getPosition());
    EXPECT_DOUBLE_EQ(nTrackSampleDataLength, pOutroCue->getLength());
    EXPECT_EQ(Cue::AUTOMATIC, pOutroCue->getSource());
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

    CuePointer pIntroCue = pTrack->findCueByType(Cue::INTRO);
    EXPECT_DOUBLE_EQ(nTrackSampleDataLength / 4, pIntroCue->getPosition());
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getLength());
    EXPECT_EQ(Cue::AUTOMATIC, pIntroCue->getSource());

    CuePointer pOutroCue = pTrack->findCueByType(Cue::OUTRO);
    EXPECT_DOUBLE_EQ(-1.0, pOutroCue->getPosition());
    EXPECT_DOUBLE_EQ(3 * nTrackSampleDataLength / 4, pOutroCue->getLength());
    EXPECT_EQ(Cue::AUTOMATIC, pOutroCue->getSource());
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

    CuePointer pIntroCue = pTrack->findCueByType(Cue::INTRO);
    EXPECT_DOUBLE_EQ(oneFifthOfTrackLength, pIntroCue->getPosition());
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getLength());
    EXPECT_EQ(Cue::AUTOMATIC, pIntroCue->getSource());

    CuePointer pOutroCue = pTrack->findCueByType(Cue::OUTRO);
    EXPECT_DOUBLE_EQ(-1.0, pOutroCue->getPosition());
    EXPECT_DOUBLE_EQ(4 * oneFifthOfTrackLength, pOutroCue->getLength());
    EXPECT_EQ(Cue::AUTOMATIC, pOutroCue->getSource());
}

TEST_F(AnalyzerSilenceTest, UpdateNonUserAdjustedCues) {
    int halfTrackLength = nTrackSampleDataLength / 2;

    pTrack->setCuePoint(CuePosition(100, Cue::AUTOMATIC)); // Arbitrary value

    CuePointer pIntroCue = pTrack->createAndAddCue();
    pIntroCue->setType(Cue::INTRO);
    pIntroCue->setSource(Cue::AUTOMATIC);
    pIntroCue->setPosition(1000); // Arbitrary value
    pIntroCue->setLength(0.0);

    CuePointer pOutroCue = pTrack->createAndAddCue();
    pOutroCue->setType(Cue::OUTRO);
    pOutroCue->setSource(Cue::AUTOMATIC);
    pOutroCue->setPosition(-1.0);
    pOutroCue->setLength(9000); // Arbitrary value

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

    EXPECT_DOUBLE_EQ(halfTrackLength, pIntroCue->getPosition());
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getLength());
    EXPECT_EQ(Cue::AUTOMATIC, pIntroCue->getSource());

    EXPECT_DOUBLE_EQ(-1.0, pOutroCue->getPosition());
    EXPECT_DOUBLE_EQ(nTrackSampleDataLength, pOutroCue->getLength());
    EXPECT_EQ(Cue::AUTOMATIC, pOutroCue->getSource());
}

TEST_F(AnalyzerSilenceTest, UpdateNonUserAdjustedRangeCues) {
    int thirdTrackLength = nTrackSampleDataLength / 3;

    CuePointer pIntroCue = pTrack->createAndAddCue();
    pIntroCue->setType(Cue::INTRO);
    pIntroCue->setSource(Cue::AUTOMATIC);
    pIntroCue->setPosition(1500.0); // Arbitrary value
    pIntroCue->setLength(1000.0);   // Arbitrary value

    CuePointer pOutroCue = pTrack->createAndAddCue();
    pOutroCue->setType(Cue::OUTRO);
    pOutroCue->setSource(Cue::AUTOMATIC);
    pOutroCue->setPosition(9000.0); // Arbitrary value
    pOutroCue->setLength(1000.0);   // Arbitrary value

    // Fill the first third with silence
    for (int i = 0; i < thirdTrackLength; i++) {
        pTrackSampleData[i] = 0.0;
    }

    // Fill the second third with 1 kHz tone
    double omega = 2.0 * M_PI * kTonePitchHz / pTrack->getSampleRate();
    for (int i = thirdTrackLength; i < 2 * thirdTrackLength; i++) {
        pTrackSampleData[i] = sin(i / kChannelCount * omega);
    }

    // Fill the last third with silence
    for (int i = 2 * thirdTrackLength; i < nTrackSampleDataLength; i++) {
        pTrackSampleData[i] = 0.0;
    }

    analyzeTrack();

    EXPECT_DOUBLE_EQ(thirdTrackLength, pIntroCue->getPosition());
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getLength());
    EXPECT_EQ(Cue::AUTOMATIC, pIntroCue->getSource());

    EXPECT_DOUBLE_EQ(-1.0, pOutroCue->getPosition());
    EXPECT_DOUBLE_EQ(2 * thirdTrackLength, pOutroCue->getLength());
    EXPECT_EQ(Cue::AUTOMATIC, pOutroCue->getSource());
}

TEST_F(AnalyzerSilenceTest, RespectUserEdits) {
    // Arbitrary values
    const double kManualCuePosition = 0.2 * nTrackSampleDataLength;
    const double kManualIntroPosition = 0.1 * nTrackSampleDataLength;
    const double kManualOutroPosition = 0.9 * nTrackSampleDataLength;

    pTrack->setCuePoint(CuePosition(kManualCuePosition, Cue::MANUAL));

    CuePointer pIntroCue = pTrack->createAndAddCue();
    pIntroCue->setType(Cue::INTRO);
    pIntroCue->setSource(Cue::MANUAL);
    pIntroCue->setPosition(kManualIntroPosition);
    pIntroCue->setLength(0.0);

    CuePointer pOutroCue = pTrack->createAndAddCue();
    pOutroCue->setType(Cue::OUTRO);
    pOutroCue->setSource(Cue::MANUAL);
    pOutroCue->setPosition(-1.0);
    pOutroCue->setLength(kManualOutroPosition);

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

    EXPECT_DOUBLE_EQ(kManualIntroPosition, pIntroCue->getPosition());
    EXPECT_DOUBLE_EQ(0.0, pIntroCue->getLength());
    EXPECT_EQ(Cue::MANUAL, pIntroCue->getSource());

    EXPECT_DOUBLE_EQ(-1.0, pOutroCue->getPosition());
    EXPECT_DOUBLE_EQ(kManualOutroPosition, pOutroCue->getLength());
    EXPECT_EQ(Cue::MANUAL, pOutroCue->getSource());
}

} // namespace
