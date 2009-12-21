#include <gtest/gtest.h>
#include <QDebug>

#include "analyserwaveform.h"

#define BIGBUF_SIZE (1024 * 1024)  //Megabyte
#define CANARY_SIZE (1024*4) 
#define MAGIC_FLOAT 1234.567890f
#define CANARY_FLOAT 2468.86420f

namespace {

    class AnalyserWaveformTest: public testing::Test {
    protected:
        
        AnalyserWaveformTest() {
            qDebug() << "AnalyserWaveformTest()";
        }
        
        virtual void SetUp() {
            qDebug() << "SetUp";
            aw = new AnalyserWaveform();
            tio = new TrackInfoObject();
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

        AnalyserWaveform* aw;
        TrackInfoObject* tio;
        CSAMPLE* bigbuf;
        CSAMPLE* canaryBigBuf;
    };
    
    //Test to make sure we don't modify the source buffer.
    TEST_F(AnalyserWaveformTest, simpleAnalyze) {
        aw->initialise(tio, 44100, BIGBUF_SIZE);
        aw->process(bigbuf, BIGBUF_SIZE);
        aw->finalise(tio);
        for (int i = 0; i < BIGBUF_SIZE; i++) {
            EXPECT_FLOAT_EQ(bigbuf[i], MAGIC_FLOAT);
        }
    }

    TEST_F(AnalyserWaveformTest, canary) {
        aw->initialise(tio, 44100, BIGBUF_SIZE);
        aw->process(bigbuf, BIGBUF_SIZE);
        aw->finalise(tio);
        for (int i = 0; i < CANARY_SIZE; i++) {
            EXPECT_FLOAT_EQ(canaryBigBuf[i], CANARY_FLOAT);
        }
        for (int i = CANARY_SIZE+BIGBUF_SIZE; i < 2*CANARY_SIZE+BIGBUF_SIZE; i++) {
            EXPECT_FLOAT_EQ(canaryBigBuf[i], CANARY_FLOAT);
        }
    }

}
