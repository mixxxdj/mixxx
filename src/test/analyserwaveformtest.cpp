#include <gtest/gtest.h>

#include <QDir>
#include <QtDebug>
#include <vector>

#include "analyzer/analyzerwaveform.h"
#include "library/dao/analysisdao.h"
#include "test/mixxxtest.h"
#include "track/track.h"

#define BIGBUF_SIZE (1024 * 1024) //Megabyte
#define CANARY_SIZE (1024 * 4)
#define MAGIC_FLOAT 1234.567890f
#define CANARY_FLOAT 0.0f

namespace {

class AnalyzerWaveformTest : public MixxxTest {
  protected:
    AnalyzerWaveformTest()
            : aw(config(), QSqlDatabase()) {
    }

    void SetUp() override {
        tio = Track::newTemporary();
        tio->setAudioProperties(
                mixxx::audio::ChannelCount(2),
                mixxx::audio::SampleRate(44100),
                mixxx::audio::Bitrate(),
                mixxx::Duration::fromMillis(1000));

        bigbuf.resize(BIGBUF_SIZE);
        for (int i = 0; i < BIGBUF_SIZE; i++)
            bigbuf[i] = MAGIC_FLOAT;

        //Memory layout for canaryBigBuf looks like
        //  [ canary | big buf | canary ]

        canaryBigBuf.resize(BIGBUF_SIZE + 2 * CANARY_SIZE);
        for (int i = 0; i < CANARY_SIZE; i++)
            canaryBigBuf[i] = CANARY_FLOAT;
        for (int i = CANARY_SIZE; i < CANARY_SIZE + BIGBUF_SIZE; i++)
            canaryBigBuf[i] = MAGIC_FLOAT;
        for (int i = CANARY_SIZE + BIGBUF_SIZE; i < 2 * CANARY_SIZE + BIGBUF_SIZE; i++)
            canaryBigBuf[i] = CANARY_FLOAT;
    }

    void TearDown() override {
    }

  protected:
    AnalyzerWaveform aw;
    TrackPointer tio;
    std::vector<CSAMPLE> bigbuf;
    std::vector<CSAMPLE> canaryBigBuf;
};

//Test to make sure we don't modify the source buffer.
TEST_F(AnalyzerWaveformTest, simpleAnalyze) {
    aw.initialize(tio, tio->getSampleRate(), BIGBUF_SIZE);
    aw.processSamples(bigbuf.data(), BIGBUF_SIZE);
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

} // namespace
