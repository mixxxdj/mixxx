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
            
            canary1 = new CSAMPLE[CANARY_SIZE];
            for (int i = 0; i < CANARY_SIZE; i++)
                canary1 = CANARY_FLOAT;
            
            bigbuf = new CSAMPLE[BIGBUF_SIZE];
            for (int i = 0; i < BIGBUF_SIZE; i++)
                bigbuf = MAGIC_FLOAT;

            canary2 = new CSAMPLE[CANARY_SIZE];
            for (int i = 0; i < CANARY_SIZE; i++)
                canary2 = CANARY_FLOAT;
        }

        virtual void TearDown() {
            qDebug() << "TearDown";
            qDebug() << "delete aw";
            delete aw;
            delete [] bigbuf;
            delete [] canary1;
            delete [] canary2;
        }

        AnalyserWaveform* aw;
        TrackInfoObject* tio;
        CSAMPLE* canary1;
        CSAMPLE* bigbuf;
        CSAMPLE* canary2;
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
            EXPECT_FLOAT_EQ(canary1[i], CANARY_FLOAT);
        }
        for (int i = 0; i < CANARY_SIZE; i++) {
            EXPECT_FLOAT_EQ(canary2[i], CANARY_FLOAT);
        }
    }

}
