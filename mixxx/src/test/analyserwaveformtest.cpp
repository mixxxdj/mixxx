#include <gtest/gtest.h>
#include <QDebug>
#include <QDir>

#include "trackinfoobject.h"
#include "configobject.h"
#include "analyserwaveform.h"

#define BIGBUF_SIZE (1024 * 1024)  //Megabyte
#define CANARY_SIZE (1024*4)
#define MAGIC_FLOAT 1234.567890f
#define CANARY_FLOAT 0.0f

namespace {

class AnalyserWaveformTest: public testing::Test {
  protected:
    virtual void SetUp() {
        config = new ConfigObject<ConfigValue>(QDir::homePath().append("/").append(SETTINGS_PATH).append(SETTINGS_FILE));
        aw = new AnalyserWaveform(config);
        tio = TrackPointer(new TrackInfoObject("foo"));
        tio->setSampleRate(44100);

        bigbuf = new CSAMPLE[BIGBUF_SIZE];
        for (int i = 0; i < BIGBUF_SIZE; i++)
            bigbuf[i] = MAGIC_FLOAT;

        //Memory layout for canaryBigBuf looks like
        //  [ canary | big buf | canary ]

        canaryBigBuf = new CSAMPLE[BIGBUF_SIZE + 2*CANARY_SIZE];
        for (int i = 0; i < CANARY_SIZE; i++)
            canaryBigBuf[i] = CANARY_FLOAT;
        for (int i = CANARY_SIZE; i < CANARY_SIZE+BIGBUF_SIZE; i++)
            canaryBigBuf[i] = MAGIC_FLOAT;
        for (int i = CANARY_SIZE+BIGBUF_SIZE; i < 2*CANARY_SIZE+BIGBUF_SIZE; i++)
            canaryBigBuf[i] = CANARY_FLOAT;
    }

    virtual void TearDown() {
        delete aw;
        delete [] bigbuf;
        delete [] canaryBigBuf;
    }

    AnalyserWaveform* aw;
    ConfigObject<ConfigValue>* config;
    TrackPointer tio;
    CSAMPLE* bigbuf;
    CSAMPLE* canaryBigBuf;
};

//Test to make sure we don't modify the source buffer.
TEST_F(AnalyserWaveformTest, simpleAnalyze) {
    aw->initialise(tio, tio->getSampleRate(), BIGBUF_SIZE);
    aw->process(bigbuf, BIGBUF_SIZE);
    aw->finalise(tio);
    for (int i = 0; i < BIGBUF_SIZE; i++) {
        EXPECT_FLOAT_EQ(bigbuf[i], MAGIC_FLOAT);
    }
}

//Basic test to make sure we don't step out of bounds.
TEST_F(AnalyserWaveformTest, canary) {
    aw->initialise(tio, tio->getSampleRate(), BIGBUF_SIZE);
    aw->process(&canaryBigBuf[CANARY_SIZE], BIGBUF_SIZE);
    aw->finalise(tio);
    for (int i = 0; i < CANARY_SIZE; i++) {
        EXPECT_FLOAT_EQ(canaryBigBuf[i], CANARY_FLOAT);
    }
    for (int i = CANARY_SIZE+BIGBUF_SIZE; i < 2*CANARY_SIZE+BIGBUF_SIZE; i++) {
        EXPECT_FLOAT_EQ(canaryBigBuf[i], CANARY_FLOAT);
    }
}

//Test to make sure that if an incorrect totalSamples is passed to
//initialise(..) and process(..) is told to process more samples than that,
//that we don't step out of bounds.
TEST_F(AnalyserWaveformTest, wrongTotalSamples) {
    aw->initialise(tio, tio->getSampleRate(), BIGBUF_SIZE/2);
    // Deliver double the expected samples
    int wrongTotalSamples = BIGBUF_SIZE;
    int blockSize = 2*32768;
    for (int i = CANARY_SIZE; i < CANARY_SIZE+wrongTotalSamples; i += blockSize) {
        aw->process(&canaryBigBuf[i], blockSize);
    }
    aw->finalise(tio);
    //Ensure the source buffer is intact
    for (int i = CANARY_SIZE; i < BIGBUF_SIZE; i++) {
        EXPECT_FLOAT_EQ(canaryBigBuf[i], MAGIC_FLOAT);
    }
    //Make sure our canaries are still OK
    for (int i = 0; i < CANARY_SIZE; i++) {
        EXPECT_FLOAT_EQ(canaryBigBuf[i], CANARY_FLOAT);
    }
    for (int i = CANARY_SIZE+BIGBUF_SIZE; i < 2*CANARY_SIZE+BIGBUF_SIZE; i++) {
        EXPECT_FLOAT_EQ(canaryBigBuf[i], CANARY_FLOAT);
    }
}
}
