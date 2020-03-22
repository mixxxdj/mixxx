#include <gtest/gtest.h>
#include <QDir>
#include <QtDebug>

#include "test/mixxxtest.h"

#include "analyzer/analyzerwaveform.h"
#include "library/dao/analysisdao.h"
#include "track/track.h"

#define BIGBUF_SIZE (1024 * 1024) //Megabyte
#define CANARY_SIZE (1024 * 4)
#define MAGIC_FLOAT 1234.567890f
#define CANARY_FLOAT 0.0f

namespace {

class AnalyzerWaveformTest : public MixxxTest {
  protected:
    AnalyzerWaveformTest()
            : aw(config(), QSqlDatabase()),
              bigbuf(nullptr),
              canaryBigBuf(nullptr) {
    }

    void SetUp() override {
        tio = Track::newTemporary();
        tio->setSampleRate(44100);

        bigbuf = new CSAMPLE[BIGBUF_SIZE];
        for (int i = 0; i < BIGBUF_SIZE; i++)
            bigbuf[i] = MAGIC_FLOAT;

        //Memory layout for canaryBigBuf looks like
        //  [ canary | big buf | canary ]

        canaryBigBuf = new CSAMPLE[BIGBUF_SIZE + 2 * CANARY_SIZE];
        for (int i = 0; i < CANARY_SIZE; i++)
            canaryBigBuf[i] = CANARY_FLOAT;
        for (int i = CANARY_SIZE; i < CANARY_SIZE + BIGBUF_SIZE; i++)
            canaryBigBuf[i] = MAGIC_FLOAT;
        for (int i = CANARY_SIZE + BIGBUF_SIZE; i < 2 * CANARY_SIZE + BIGBUF_SIZE; i++)
            canaryBigBuf[i] = CANARY_FLOAT;
    }

    void TearDown() override {
        delete[] bigbuf;
        delete[] canaryBigBuf;
    }

  protected:
    AnalyzerWaveform aw;
    TrackPointer tio;
    CSAMPLE* bigbuf;
    CSAMPLE* canaryBigBuf;
};

//Test to make sure we don't modify the source buffer.
TEST_F(AnalyzerWaveformTest, simpleAnalyze) {
    aw.initialize(tio, tio->getSampleRate(), BIGBUF_SIZE);
    aw.processSamples(bigbuf, BIGBUF_SIZE);
    aw.storeResults(tio);
    aw.cleanup();
    for (int i = 0; i < BIGBUF_SIZE; i++) {
        EXPECT_FLOAT_EQ(bigbuf[i], MAGIC_FLOAT);
    }
}

//Basic test to make sure we don't step out of bounds.
TEST_F(AnalyzerWaveformTest, canary) {
    aw.initialize(tio, tio->getSampleRate(), BIGBUF_SIZE);
    aw.processSamples(&canaryBigBuf[CANARY_SIZE], BIGBUF_SIZE);
    aw.storeResults(tio);
    aw.cleanup();
    for (int i = 0; i < CANARY_SIZE; i++) {
        EXPECT_FLOAT_EQ(canaryBigBuf[i], CANARY_FLOAT);
    }
    for (int i = CANARY_SIZE + BIGBUF_SIZE; i < 2 * CANARY_SIZE + BIGBUF_SIZE; i++) {
        EXPECT_FLOAT_EQ(canaryBigBuf[i], CANARY_FLOAT);
    }
}

//Test to make sure that if an incorrect totalSamples is passed to
//initialize(..) and process(..) is told to process more samples than that,
//that we don't step out of bounds.
TEST_F(AnalyzerWaveformTest, wrongTotalSamples) {
    aw.initialize(tio, tio->getSampleRate(), BIGBUF_SIZE / 2);
    // Deliver double the expected samples
    int wrongTotalSamples = BIGBUF_SIZE;
    int blockSize = 2 * 32768;
    for (int i = CANARY_SIZE; i < CANARY_SIZE + wrongTotalSamples; i += blockSize) {
        aw.processSamples(&canaryBigBuf[i], blockSize);
    }
    aw.storeResults(tio);
    aw.cleanup();
    //Ensure the source buffer is intact
    for (int i = CANARY_SIZE; i < BIGBUF_SIZE; i++) {
        EXPECT_FLOAT_EQ(canaryBigBuf[i], MAGIC_FLOAT);
    }
    //Make sure our canaries are still OK
    for (int i = 0; i < CANARY_SIZE; i++) {
        EXPECT_FLOAT_EQ(canaryBigBuf[i], CANARY_FLOAT);
    }
    for (int i = CANARY_SIZE + BIGBUF_SIZE; i < 2 * CANARY_SIZE + BIGBUF_SIZE; i++) {
        EXPECT_FLOAT_EQ(canaryBigBuf[i], CANARY_FLOAT);
    }
}
} // namespace
