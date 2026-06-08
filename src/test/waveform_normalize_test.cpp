#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>

#include "test/mixxxtest.h"
#include "track/track.h"
#include "waveform/renderers/waveformrenderersignalbase.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"

namespace {

constexpr int kSampleRate = 44100;
constexpr int kDesiredVisualSampleRate = 441;
constexpr int kStemCount = 0;

WaveformPointer createTestWaveform(
        SINT frameLength, unsigned char peakValue) {
    auto pWaveform = WaveformPointer(new Waveform(
            kSampleRate, frameLength, kDesiredVisualSampleRate, -1, kStemCount));
    const int dataSize = pWaveform->getDataSize();
    WaveformData* pData = pWaveform->data();
    for (int i = 0; i < dataSize; ++i) {
        pData[i].filtered.low = peakValue / 3;
        pData[i].filtered.mid = peakValue / 3;
        pData[i].filtered.high = peakValue / 3;
        pData[i].filtered.all = peakValue;
    }
    pWaveform->setCompletion(dataSize);
    return pWaveform;
}

WaveformPointer createTestWaveformWithSinglePeak(
        SINT frameLength,
        unsigned char baseValue,
        unsigned char peakValue,
        int peakIndex) {
    auto pWaveform = WaveformPointer(new Waveform(
            kSampleRate, frameLength, kDesiredVisualSampleRate, -1, kStemCount));
    const int dataSize = pWaveform->getDataSize();
    WaveformData* pData = pWaveform->data();
    for (int i = 0; i < dataSize; ++i) {
        unsigned char value = (i == peakIndex) ? peakValue : baseValue;
        pData[i].filtered.low = value / 3;
        pData[i].filtered.mid = value / 3;
        pData[i].filtered.high = value / 3;
        pData[i].filtered.all = value;
    }
    pWaveform->setCompletion(dataSize);
    return pWaveform;
}

float scanWaveformPeak(const Waveform& waveform) {
    const int dataSize = waveform.getDataSize();
    float peak = 1.0f;
    for (int i = 0; i < dataSize; i += 2) {
        peak = std::max(peak, static_cast<float>(waveform.getAll(i)));
        peak = std::max(peak, static_cast<float>(waveform.getAll(i + 1)));
    }
    return peak;
}

float computeNormalizationFactor(float peak) {
    if (peak > 1.0f) {
        return 255.0f / peak;
    }
    return 1.0f;
}

// ---- Unit tests: peak scanning algorithm ----

TEST(WaveformNormalizeTest, FullScaleTrackPeakIs255) {
    auto pWaveform = createTestWaveform(44100, 255);
    EXPECT_FLOAT_EQ(scanWaveformPeak(*pWaveform), 255.0f);
}

TEST(WaveformNormalizeTest, HalfScaleTrackPeakIs128) {
    auto pWaveform = createTestWaveform(44100, 128);
    EXPECT_FLOAT_EQ(scanWaveformPeak(*pWaveform), 128.0f);
}

TEST(WaveformNormalizeTest, QuietTrackPeakIs30) {
    auto pWaveform = createTestWaveform(44100, 30);
    EXPECT_FLOAT_EQ(scanWaveformPeak(*pWaveform), 30.0f);
}

TEST(WaveformNormalizeTest, SinglePeakDetectedAtOddIndex) {
    auto pWaveform = createTestWaveformWithSinglePeak(44100, 20, 200, 101);
    EXPECT_FLOAT_EQ(scanWaveformPeak(*pWaveform), 200.0f);
}

TEST(WaveformNormalizeTest, SinglePeakDetectedAtEvenIndex) {
    auto pWaveform = createTestWaveformWithSinglePeak(44100, 20, 180, 100);
    EXPECT_FLOAT_EQ(scanWaveformPeak(*pWaveform), 180.0f);
}

TEST(WaveformNormalizeTest, EmptyWaveformPeakIsMinimum) {
    auto pWaveform = createTestWaveform(44100, 0);
    EXPECT_FLOAT_EQ(scanWaveformPeak(*pWaveform), 1.0f);
}

// ---- Unit tests: normalization factor math ----

TEST(WaveformNormalizeTest, NormalizationFactorFullScale) {
    EXPECT_FLOAT_EQ(computeNormalizationFactor(255.0f), 1.0f);
}

TEST(WaveformNormalizeTest, NormalizationFactorHalfScale) {
    EXPECT_NEAR(computeNormalizationFactor(128.0f), 1.9921875f, 0.001f);
}

TEST(WaveformNormalizeTest, NormalizationFactorQuietTrack) {
    EXPECT_FLOAT_EQ(computeNormalizationFactor(30.0f), 8.5f);
}

TEST(WaveformNormalizeTest, NormalizationFactorZeroPeakReturnsOne) {
    EXPECT_FLOAT_EQ(computeNormalizationFactor(0.0f), 1.0f);
}

TEST(WaveformNormalizeTest, NormalizationFactorOnePeakReturnsOne) {
    EXPECT_FLOAT_EQ(computeNormalizationFactor(1.0f), 1.0f);
}

// ---- Unit tests: end-to-end gain math ----

TEST(WaveformNormalizeTest, NormalizedGainScalesCorrectly) {
    const float visualGain = 2.0f;
    const float headroom = 0.85f;
    const float peak = 50.0f;
    const float halfBreadth = 100.0f;

    float allGain = visualGain * headroom;
    allGain *= computeNormalizationFactor(peak);

    const float heightFactor = allGain * halfBreadth / 255.0f;
    const float sampleHeight = heightFactor * peak;
    EXPECT_NEAR(sampleHeight, visualGain * headroom * halfBreadth, 0.01f);
}

TEST(WaveformNormalizeTest, NormalizedGainPreservesRelativeAmplitudes) {
    const float peak = 80.0f;
    const float normFactor = computeNormalizationFactor(peak);

    const float loudSample = 80.0f;
    const float quietSample = 20.0f;

    const float loudNormalized = loudSample * normFactor / 255.0f;
    const float quietNormalized = quietSample * normFactor / 255.0f;

    EXPECT_NEAR(loudNormalized / quietNormalized, 4.0f, 0.001f);
}

// ---- Concrete subclass exposing protected methods for integration tests ----

class TestableSignalRenderer : public WaveformRendererSignalBase {
  public:
    using WaveformRendererSignalBase::WaveformRendererSignalBase;

    void onSetup(const QDomNode&) override {
    }
    void draw(QPainter*, QPaintEvent*) override {
    }

    using WaveformRendererSignalBase::getGains;
    using WaveformRendererSignalBase::getTrackPeak;
};

// ---- Integration tests: factory setting + renderer pipeline ----

class WaveformNormalizeIntegrationTest : public MixxxTest {
  protected:
    void SetUp() override {
        MixxxTest::SetUp();
        if (!WaveformWidgetFactory::isCreated()) {
            WaveformWidgetFactory::createInstance();
        }
        WaveformWidgetFactory::instance()->setConfig(config());

        m_pWidgetRenderer =
                std::make_unique<WaveformWidgetRenderer>(QString());

        m_pSignalRenderer = std::make_unique<TestableSignalRenderer>(
                m_pWidgetRenderer.get(),
                WaveformRendererSignalBase::Option::None);
    }

    void TearDown() override {
        m_pSignalRenderer.reset();
        m_pWidgetRenderer.reset();
        MixxxTest::TearDown();
    }

    TrackPointer createTrackWithWaveform(unsigned char peakValue) {
        TrackPointer pTrack = Track::newTemporary();
        pTrack->setAudioProperties(
                mixxx::audio::ChannelCount(2),
                mixxx::audio::SampleRate(kSampleRate),
                mixxx::audio::Bitrate(),
                mixxx::Duration::fromMillis(1000));
        auto pWaveform = createTestWaveform(kSampleRate, peakValue);
        pTrack->setWaveform(pWaveform);
        return pTrack;
    }

    std::unique_ptr<WaveformWidgetRenderer> m_pWidgetRenderer;
    std::unique_ptr<TestableSignalRenderer> m_pSignalRenderer;
};

TEST_F(WaveformNormalizeIntegrationTest, FactorySettingDefaultsToOff) {
    EXPECT_FALSE(WaveformWidgetFactory::instance()->getNormalizeWaveform());
}

TEST_F(WaveformNormalizeIntegrationTest, FactorySettingPersistsToConfig) {
    auto* factory = WaveformWidgetFactory::instance();
    factory->setNormalizeWaveform(true);
    EXPECT_TRUE(factory->getNormalizeWaveform());

    factory->setNormalizeWaveform(false);
    EXPECT_FALSE(factory->getNormalizeWaveform());
}

TEST_F(WaveformNormalizeIntegrationTest, FactorySignalReachesRenderer) {
    auto* factory = WaveformWidgetFactory::instance();

    QObject::connect(factory,
            &WaveformWidgetFactory::normalizeWaveformChanged,
            m_pSignalRenderer.get(),
            &TestableSignalRenderer::setNormalizeWaveform);

    m_pSignalRenderer->setNormalizeWaveform(false);
    factory->setNormalizeWaveform(true);

    float allGain = 0.0f;
    m_pSignalRenderer->setAllChannelVisualGain(2.0);

    TrackPointer pTrack = createTrackWithWaveform(50);
    m_pWidgetRenderer->setTrack(pTrack);

    m_pSignalRenderer->getGains(&allGain, nullptr, nullptr, nullptr);

    // With normalization on and peak=50: allGain = visualGain * headroom * (255/50)
    // Track gain (getGain()) is ignored when normalization is enabled
    constexpr float kHeadroom = 0.85f;
    EXPECT_NEAR(allGain, 2.0f * kHeadroom * 255.0f / 50.0f, 0.01f);
}

TEST_F(WaveformNormalizeIntegrationTest, GetGainsWithoutNormalization) {
    m_pSignalRenderer->setNormalizeWaveform(false);
    m_pSignalRenderer->setAllChannelVisualGain(3.0);

    TrackPointer pTrack = createTrackWithWaveform(50);
    m_pWidgetRenderer->setTrack(pTrack);

    float allGain = 0.0f;
    m_pSignalRenderer->getGains(&allGain, nullptr, nullptr, nullptr);

    // Without normalization: getGain() (default 1.0) * visualGain (3.0) = 3.0
    EXPECT_FLOAT_EQ(allGain, 3.0f);
}

TEST_F(WaveformNormalizeIntegrationTest, GetGainsWithNormalizationFullScale) {
    m_pSignalRenderer->setNormalizeWaveform(true);
    m_pSignalRenderer->setAllChannelVisualGain(2.0);

    TrackPointer pTrack = createTrackWithWaveform(255);
    m_pWidgetRenderer->setTrack(pTrack);

    float allGain = 0.0f;
    m_pSignalRenderer->getGains(&allGain, nullptr, nullptr, nullptr);

    // Full-scale track: allGain = visualGain * headroom * (255/255) = 2.0 * 0.85
    EXPECT_FLOAT_EQ(allGain, 2.0f * 0.85f);
}

TEST_F(WaveformNormalizeIntegrationTest, GetGainsWithNormalizationQuietTrack) {
    m_pSignalRenderer->setNormalizeWaveform(true);
    m_pSignalRenderer->setAllChannelVisualGain(2.0);

    TrackPointer pTrack = createTrackWithWaveform(30);
    m_pWidgetRenderer->setTrack(pTrack);

    float allGain = 0.0f;
    m_pSignalRenderer->getGains(&allGain, nullptr, nullptr, nullptr);

    // Quiet track: allGain = visualGain * headroom * (255/30) = 2.0 * 0.85 * 8.5
    EXPECT_NEAR(allGain, 2.0f * 0.85f * 8.5f, 0.001f);
}

TEST_F(WaveformNormalizeIntegrationTest, GetTrackPeakReturnsCorrectValue) {
    m_pSignalRenderer->setNormalizeWaveform(true);

    TrackPointer pTrack = createTrackWithWaveform(120);
    m_pWidgetRenderer->setTrack(pTrack);

    EXPECT_FLOAT_EQ(m_pSignalRenderer->getTrackPeak(), 120.0f);
}

TEST_F(WaveformNormalizeIntegrationTest, GetTrackPeakCachesAcrossCalls) {
    m_pSignalRenderer->setNormalizeWaveform(true);

    TrackPointer pTrack = createTrackWithWaveform(80);
    m_pWidgetRenderer->setTrack(pTrack);

    float peak1 = m_pSignalRenderer->getTrackPeak();
    float peak2 = m_pSignalRenderer->getTrackPeak();
    EXPECT_FLOAT_EQ(peak1, peak2);
    EXPECT_FLOAT_EQ(peak1, 80.0f);
}

TEST_F(WaveformNormalizeIntegrationTest, GetTrackPeakUpdatesOnTrackChange) {
    m_pSignalRenderer->setNormalizeWaveform(true);

    TrackPointer pTrack1 = createTrackWithWaveform(80);
    m_pWidgetRenderer->setTrack(pTrack1);
    EXPECT_FLOAT_EQ(m_pSignalRenderer->getTrackPeak(), 80.0f);

    TrackPointer pTrack2 = createTrackWithWaveform(200);
    m_pWidgetRenderer->setTrack(pTrack2);
    EXPECT_FLOAT_EQ(m_pSignalRenderer->getTrackPeak(), 200.0f);
}

TEST_F(WaveformNormalizeIntegrationTest, GetTrackPeakNoTrackReturnsZero) {
    EXPECT_FLOAT_EQ(m_pSignalRenderer->getTrackPeak(), 0.0f);
}

TEST_F(WaveformNormalizeIntegrationTest, NormalizationDisabledIgnoresPeak) {
    m_pSignalRenderer->setNormalizeWaveform(false);
    m_pSignalRenderer->setAllChannelVisualGain(2.0);

    // Even with a very quiet track, gain should NOT be boosted
    TrackPointer pTrack = createTrackWithWaveform(10);
    m_pWidgetRenderer->setTrack(pTrack);

    float allGain = 0.0f;
    m_pSignalRenderer->getGains(&allGain, nullptr, nullptr, nullptr);
    EXPECT_FLOAT_EQ(allGain, 2.0f);
}

TEST_F(WaveformNormalizeIntegrationTest, HeightFactorFillsDisplayForQuietTrack) {
    m_pSignalRenderer->setNormalizeWaveform(true);
    m_pSignalRenderer->setAllChannelVisualGain(1.0);

    const unsigned char peakValue = 40;
    TrackPointer pTrack = createTrackWithWaveform(peakValue);
    m_pWidgetRenderer->setTrack(pTrack);

    float allGain = 0.0f;
    m_pSignalRenderer->getGains(&allGain, nullptr, nullptr, nullptr);

    // Simulate the renderer's height calculation
    const float halfBreadth = 100.0f;
    const float maxValue = 255.0f;
    const float heightFactor = allGain * halfBreadth / maxValue;

    // A sample at peak should fill 85% of the waveform area (headroom)
    const float peakHeight = heightFactor * static_cast<float>(peakValue);
    EXPECT_NEAR(peakHeight, halfBreadth * 0.85f, 0.1f);
}

} // namespace
