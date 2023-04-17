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
            : m_aw(config(), QSqlDatabase()),
              m_canaryBigBuf(nullptr) {
    }

    void SetUp() override {
        m_pTrack = Track::newTemporary();
        m_pTrack->setAudioProperties(
                mixxx::audio::ChannelCount(2),
                mixxx::audio::SampleRate(44100),
                mixxx::audio::Bitrate(),
                mixxx::Duration::fromMillis(1000));

        // Memory layout for m_canaryBigBuf looks like
        //   [ canary | big buf | canary ]

        m_canaryBigBuf = new CSAMPLE[BIGBUF_SIZE + 2 * CANARY_SIZE];
        for (int i = 0; i < CANARY_SIZE; i++) {
            m_canaryBigBuf[i] = CANARY_FLOAT;
        }
        for (int i = CANARY_SIZE; i < CANARY_SIZE + BIGBUF_SIZE; i++) {
            m_canaryBigBuf[i] = MAGIC_FLOAT;
        }
        for (int i = CANARY_SIZE + BIGBUF_SIZE; i < 2 * CANARY_SIZE + BIGBUF_SIZE; i++) {
            m_canaryBigBuf[i] = CANARY_FLOAT;
        }
    }

    void TearDown() override {
        delete[] m_canaryBigBuf;
    }

  protected:
    AnalyzerWaveform m_aw;
    TrackPointer m_pTrack;
    CSAMPLE* m_canaryBigBuf;
};

// Basic test to make sure we don't alter the input buffer and don't step out of bounds.
TEST_F(AnalyzerWaveformTest, canary) {
    m_aw.initialize(m_pTrack, m_pTrack->getSampleRate(), BIGBUF_SIZE);
    m_aw.processSamples(&m_canaryBigBuf[CANARY_SIZE], BIGBUF_SIZE);
    m_aw.storeResults(m_pTrack);
    m_aw.cleanup();
    int i = 0;
    for (; i < CANARY_SIZE; i++) {
        EXPECT_FLOAT_EQ(m_canaryBigBuf[i], CANARY_FLOAT);
    }
    for (; i < CANARY_SIZE + BIGBUF_SIZE; i++) {
        EXPECT_FLOAT_EQ(m_canaryBigBuf[i], MAGIC_FLOAT);
    }
    for (; i < 2 * CANARY_SIZE + BIGBUF_SIZE; i++) {
        EXPECT_FLOAT_EQ(m_canaryBigBuf[i], CANARY_FLOAT);
    }
}

} // namespace
