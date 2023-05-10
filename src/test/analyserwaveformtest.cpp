#include <gtest/gtest.h>

#include <QDir>
#include <QtDebug>
#include <vector>

#include "analyzer/analyzerwaveform.h"
#include "library/dao/analysisdao.h"
#include "test/mixxxtest.h"
#include "track/track.h"

namespace {

constexpr std::size_t kBigBufSize = 1024 * 1024; // Megabyte
constexpr std::size_t kCanarySize = 1024 * 4;
constexpr float kMagicFloat = 1234.567890f;
constexpr float kCanaryFloat = 0.0f;

class AnalyzerWaveformTest : public MixxxTest {
  protected:
    AnalyzerWaveformTest()
            : m_aw(config(), QSqlDatabase()) {
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

        m_canaryBigBuf.resize(kBigBufSize + 2 * kCanarySize);
        for (std::size_t i = 0; i < kCanarySize; i++) {
            m_canaryBigBuf[i] = kCanaryFloat;
        }
        for (std::size_t i = kCanarySize; i < kCanarySize + kBigBufSize; i++) {
            m_canaryBigBuf[i] = kMagicFloat;
        }
        for (std::size_t i = kCanarySize + kBigBufSize; i < 2 * kCanarySize + kBigBufSize; i++) {
            m_canaryBigBuf[i] = kCanaryFloat;
        }
    }

    void TearDown() override {
    }

  protected:
    AnalyzerWaveform m_aw;
    TrackPointer m_pTrack;
    std::vector<CSAMPLE> m_canaryBigBuf;
};

// Basic test to make sure we don't alter the input buffer and don't step out of bounds.
TEST_F(AnalyzerWaveformTest, canary) {
    m_aw.initialize(m_pTrack, m_pTrack->getSampleRate(), kBigBufSize);
    m_aw.processSamples(&m_canaryBigBuf[kCanarySize], kBigBufSize);
    m_aw.storeResults(m_pTrack);
    m_aw.cleanup();
    std::size_t i = 0;
    for (; i < kCanarySize; i++) {
        EXPECT_FLOAT_EQ(m_canaryBigBuf[i], kCanaryFloat);
    }
    for (; i < kCanarySize + kBigBufSize; i++) {
        EXPECT_FLOAT_EQ(m_canaryBigBuf[i], kMagicFloat);
    }
    for (; i < 2 * kCanarySize + kBigBufSize; i++) {
        EXPECT_FLOAT_EQ(m_canaryBigBuf[i], kCanaryFloat);
    }
}

} // namespace
