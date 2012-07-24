#include <gtest/gtest.h>
#include <QDebug>

#include "trackinfoobject.h"
#include "analyserwavesummary.h"

#define BIGBUF_SIZE (1024 * 1024)  //Megabyte
#define CANARY_SIZE (1024*4)
#define MAGIC_FLOAT 1234.567890f
#define CANARY_FLOAT 0.0f

namespace {

class AnalyserWavesummaryTest: public testing::Test {
  protected:
    virtual void SetUp() {
        qDebug() << "SetUp";
        aw = new AnalyserWavesummary();
        tio = TrackPointer(new TrackInfoObject("foo"));
        //Subpixels per second, from waveformrenderer.cpp:247
        tio->setVisualResampleRate(200);

        bigbuf = new CSAMPLE[BIGBUF_SIZE];
        for (int i = 0; i < BIGBUF_SIZE; i++)
            bigbuf[i] = MAGIC_FLOAT;

        //Memory layout for canaryBigBuf looks like
        //  [ canary | big buf | canary ]
        //

        canaryBigBuf = new CSAMPLE[BIGBUF_SIZE + 2*CANARY_SIZE];
        for (int i = 0; i < CANARY_SIZE; i++)
            canaryBigBuf[i] = CANARY_FLOAT;
        for (int i = CANARY_SIZE; i < CANARY_SIZE+BIGBUF_SIZE; i++)
            canaryBigBuf[i] = MAGIC_FLOAT;
        for (int i = CANARY_SIZE+BIGBUF_SIZE; i < 2*CANARY_SIZE+BIGBUF_SIZE; i++)
            canaryBigBuf[i] = CANARY_FLOAT;
    }

    virtual void TearDown() {
        qDebug() << "TearDown";
        qDebug() << "delete aw";
        delete aw;
        delete [] bigbuf;
        delete [] canaryBigBuf;
    }

    AnalyserWavesummary* aw;
    TrackPointer tio;
    CSAMPLE* bigbuf;
    CSAMPLE* canaryBigBuf;
};

//Test to make sure we don't modify the source buffer.
TEST_F(AnalyserWavesummaryTest, simpleAnalyze) {
    aw->initialise(tio, 44100, BIGBUF_SIZE);
    aw->process(bigbuf, BIGBUF_SIZE);
    aw->finalise(tio);
    for (int i = 0; i < BIGBUF_SIZE; i++) {
        EXPECT_FLOAT_EQ(bigbuf[i], MAGIC_FLOAT);
    }
}

//Basic test to make sure we don't step out of bounds.
TEST_F(AnalyserWavesummaryTest, canary) {
    aw->initialise(tio, 44100, BIGBUF_SIZE);
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
TEST_F(AnalyserWavesummaryTest, wrongTotalSamples) {
    aw->initialise(tio, 44100, BIGBUF_SIZE);
    //Process in a loop
    int wrongTotalSamples = BIGBUF_SIZE+1; //Too big by 1 sample...
    //Note that the correct totalSamples would just be BIGBUF_SIZE. :)
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
