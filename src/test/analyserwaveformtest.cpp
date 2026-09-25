#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QMetaEnum>
#include <QtDebug>
#include <array>
#include <utility>
#include <vector>

#include "analyzer/analyzertrack.h"
#include "analyzer/analyzerwaveform.h"
#include "analyzer/perceptualwaveformlut.h"
#include "library/dao/analysisdao.h"
#ifdef MIXXX_USE_QML
#include "qml/qmlwaveformdisplay.h"
#endif
#include "test/mixxxtest.h"
#include "track/track.h"
#include "waveform/waveformfactory.h"
#include "waveform/widgets/waveformwidgettype.h"

namespace {

constexpr std::size_t kBigBufSize = 2 * 1920; // Matches the WaveformSummary
constexpr std::size_t kCanarySize = 1024 * 4;
constexpr float kMagicFloat = 1234.567890f;
constexpr float kCanaryFloat = 0.0f;
constexpr int kChannelCount = 2;
const QString kReferenceBuffersPath = QStringLiteral("reference_buffers/");

class AnalyzerWaveformTest : public MixxxTest {
  protected:
    AnalyzerWaveformTest()
            : m_aw(config(), QSqlDatabase()) {
    }

    void SetUp() override {
        m_pTrack = Track::newTemporary();
        m_pTrack->setAudioProperties(
                mixxx::audio::ChannelCount(kChannelCount),
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

    void assertWaveformReference(
            ConstWaveformPointer pWaveform,
            const QString& reference_title) {
        pWaveform->dump();

        QFile f(getTestDir().filePath(kReferenceBuffersPath + reference_title));
        bool pass = true;
        // If the file is not there, we will fail and write out the .actual
        // reference file.
        const QByteArray actual = pWaveform->toByteArray();

        ASSERT_TRUE(f.open(QFile::ReadOnly));
        const QByteArray reference = f.readAll();

        if (actual.size() == reference.size()) {
            for (int i = 0; i < actual.size(); ++i) {
                if (actual.at(i) != reference.at(i)) {
                    qDebug() << "#" << i << QString::number(actual[i], 16)
                             << QString::number(reference[i], 16);
                    pass = false;
                }
            }
        } else {
            qDebug() << "##" << actual.size() << reference.size();
            pass = false;
        }

        // Fail if either we didn't pass, or the comparison file was empty.
        if (!pass) {
            QString fname_actual = reference_title + ".actual";
            qWarning() << "Buffer does not match" << reference_title
                       << ", actual buffer written to "
                       << "reference_buffers/" + fname_actual;
            QFile actualFile(getTestDir().filePath(kReferenceBuffersPath + fname_actual));
            ASSERT_TRUE(actualFile.open(QFile::WriteOnly));
            actualFile.write(actual);
            actualFile.close();
            EXPECT_TRUE(false);
        }
        f.close();
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
    m_aw.initialize(AnalyzerTrack(m_pTrack),
            m_pTrack->getSampleRate(),
            m_pTrack->getChannels(),
            kBigBufSize / kChannelCount);
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

    // Small reference, compare bitwise
    assertWaveformReference(m_pTrack->getWaveform(), "AnalyzerWaveformsTest");

    // The summary is always big, so we check only the metadata
    ConstWaveformPointer pWaveformSummary = m_pTrack->getWaveformSummary();
    ASSERT_NE(pWaveformSummary, nullptr);
    EXPECT_EQ(pWaveformSummary->getDataSize(), 3842);
    EXPECT_EQ(pWaveformSummary->getCompletion(), 3842);
    EXPECT_DOUBLE_EQ(pWaveformSummary->getAudioVisualRatio(), 1.0);
}

TEST(PerceptualWaveformStrideTest, storesDetailAndSummaryWithPerceptualLut) {
    PerceptualWaveformStride stride(1.0, 1.0, 44100.0, 0);
    std::array<WaveformData, ChannelCount> data{};

    stride.m_sampleCount = 4;
    stride.m_overallData[Left] = 1.0f;
    stride.m_filteredData[Left][Low] = 4.0f;
    stride.m_filteredData[Left][Mid] = 0.25f;
    stride.m_filteredData[Left][High] = 0.0f;
    stride.m_overallData[Right] = 0.25f;
    stride.m_filteredData[Right][Low] = 0.0f;
    stride.m_filteredData[Right][Mid] = 1.0f;
    stride.m_filteredData[Right][High] = 4.0f;
    stride.store(data.data());

    EXPECT_EQ(data[Left].filtered.all, 128);
    EXPECT_EQ(data[Left].filtered.low, 94);
    EXPECT_EQ(data[Left].filtered.mid, 46);
    EXPECT_EQ(data[Left].filtered.high, 0);
    EXPECT_EQ(data[Right].filtered.all, 64);
    EXPECT_EQ(data[Right].filtered.low, 0);
    EXPECT_EQ(data[Right].filtered.mid, 81);
    EXPECT_EQ(data[Right].filtered.high, 100);
    EXPECT_EQ(stride.m_sampleCount, 0);

    stride.m_summarySampleCount = 8;
    stride.m_summaryOverallData[Left] = 4.0f;
    stride.m_summaryFilteredData[Left][Low] = 0.0f;
    stride.m_summaryFilteredData[Left][Mid] = 8.0f;
    stride.m_summaryFilteredData[Left][High] = 0.5f;
    stride.m_summaryOverallData[Right] = 0.5f;
    stride.m_summaryFilteredData[Right][Low] = 2.0f;
    stride.m_summaryFilteredData[Right][Mid] = 0.0f;
    stride.m_summaryFilteredData[Right][High] = 8.0f;
    stride.averageStore(data.data());

    EXPECT_EQ(data[Left].filtered.all, 180);
    EXPECT_EQ(data[Left].filtered.low, 0);
    EXPECT_EQ(data[Left].filtered.mid, 92);
    EXPECT_EQ(data[Left].filtered.high, 77);
    EXPECT_EQ(data[Right].filtered.all, 64);
    EXPECT_EQ(data[Right].filtered.low, 77);
    EXPECT_EQ(data[Right].filtered.mid, 0);
    EXPECT_EQ(data[Right].filtered.high, 100);
    EXPECT_EQ(stride.m_summarySampleCount, 0);
}

TEST(PerceptualWaveformStrideTest, appliesBandSpecificExponentialRelease) {
    PerceptualWaveformStride stride(1.0, 1.0, 44100.0, 0);
    std::array<WaveformData, ChannelCount> data{};

    stride.m_sampleCount = 441;
    for (int band = Low; band < BandCount; ++band) {
        stride.m_filteredData[Left][band] = 110.25f;
    }
    stride.store(data.data());

    EXPECT_EQ(data[Left].filtered.low, 77);
    EXPECT_EQ(data[Left].filtered.mid, 81);
    EXPECT_EQ(data[Left].filtered.high, 100);

    stride.m_sampleCount = 441;
    stride.store(data.data());

    EXPECT_EQ(data[Left].filtered.low, 69);
    EXPECT_EQ(data[Left].filtered.mid, 70);
    EXPECT_EQ(data[Left].filtered.high, 83);
}

TEST(PerceptualWaveformLutTest, interpolatesKnotsAndUsesLowTail) {
    using mixxx::analyzer::PerceptualWaveformLut;

    const auto transform = [](float value, int band) {
        return 255.0f * PerceptualWaveformLut::applyNormalized(value / 255.0f, band);
    };
    const auto legacyLow = [](float value) {
        constexpr std::array knots{
                std::pair{0.0f, 0.0f},
                std::pair{8.0f, 8.5f},
                std::pair{16.0f, 16.0f},
                std::pair{32.0f, 27.0f},
                std::pair{48.0f, 37.0f},
                std::pair{64.0f, 44.0f},
                std::pair{80.0f, 53.0f},
                std::pair{96.0f, 58.0f},
                std::pair{112.0f, 67.5f},
                std::pair{128.0f, 77.0f}};
        for (std::size_t index = 1; index < knots.size(); ++index) {
            if (value <= knots[index].first) {
                const auto [lowerInput, lowerOutput] = knots[index - 1];
                const auto [upperInput, upperOutput] = knots[index];
                return lowerOutput +
                        (value - lowerInput) * (upperOutput - lowerOutput) /
                        (upperInput - lowerInput);
            }
        }
        return knots.back().second;
    };

    EXPECT_FLOAT_EQ(0.0f, PerceptualWaveformLut::applyNormalized(0.0f, Low));
    EXPECT_NEAR(8.5f / 255.0f,
            PerceptualWaveformLut::applyNormalized(8.0f / 255.0f, Low),
            0.000001f);
    EXPECT_NEAR(30.5f / 255.0f,
            PerceptualWaveformLut::applyNormalized(40.0f / 255.0f, Mid),
            0.000001f);
    EXPECT_NEAR(67.5f / 255.0f,
            PerceptualWaveformLut::applyNormalized(56.0f / 255.0f, High),
            0.000001f);
    EXPECT_NEAR(77.0f, transform(128.0f, Low), 0.00001f);
    EXPECT_NEAR(77.136f, transform(129.0f, Low), 0.00001f);
    EXPECT_NEAR(81.352f, transform(160.0f, Low), 0.00001f);
    EXPECT_NEAR(94.272f, transform(255.0f, Low), 0.00001f);
    EXPECT_NEAR(92.0f / 255.0f,
            PerceptualWaveformLut::applyNormalized(1.0f, Mid),
            0.000001f);
    EXPECT_NEAR(100.0f / 255.0f,
            PerceptualWaveformLut::applyNormalized(1.0f, High),
            0.000001f);

    for (int input = 0; input <= 128; ++input) {
        EXPECT_NEAR(legacyLow(static_cast<float>(input)),
                transform(static_cast<float>(input), Low),
                0.00001f);
    }

    for (const int band : {Low, Mid, High}) {
        float previous = 0.0f;
        for (int input = 0; input <= 255; ++input) {
            const float transformed = PerceptualWaveformLut::applyNormalized(
                    static_cast<float>(input) / 255.0f, band);
            EXPECT_GE(transformed, previous);
            previous = transformed;
        }
    }
}

TEST(WaveformFactoryTest, separatesLegacyAndPerceptualCacheProfiles) {
    EXPECT_EQ(WaveformFactory::waveformVersionToVersionClass(
                      WAVEFORM_LEGACY_CURRENT_VERSION,
                      mixxx::WaveformAnalysisProfile::Legacy),
            WaveformFactory::VC_USE);
    EXPECT_EQ(WaveformFactory::waveformSummaryVersionToVersionClass(
                      WAVEFORMSUMMARY_LEGACY_CURRENT_VERSION,
                      mixxx::WaveformAnalysisProfile::Legacy),
            WaveformFactory::VC_USE);
    EXPECT_EQ(WaveformFactory::waveformVersionToVersionClass(
                      WAVEFORM_PERCEPTUAL_3BAND_VERSION,
                      mixxx::WaveformAnalysisProfile::Perceptual3Band),
            WaveformFactory::VC_USE);
    EXPECT_EQ(WaveformFactory::waveformSummaryVersionToVersionClass(
                      WAVEFORMSUMMARY_PERCEPTUAL_3BAND_VERSION,
                      mixxx::WaveformAnalysisProfile::Perceptual3Band),
            WaveformFactory::VC_USE);

    EXPECT_EQ(WaveformFactory::waveformVersionToVersionClass(
                      WAVEFORM_LEGACY_CURRENT_VERSION,
                      mixxx::WaveformAnalysisProfile::Perceptual3Band),
            WaveformFactory::VC_REMOVE);
    EXPECT_EQ(WaveformFactory::waveformVersionToVersionClass(
                      WAVEFORM_PERCEPTUAL_3BAND_VERSION,
                      mixxx::WaveformAnalysisProfile::Legacy),
            WaveformFactory::VC_REMOVE);
}

TEST(WaveformWidgetTypeTest, mapsOnlyPerceptualThreeBandToThePerceptualProfile) {
    EXPECT_EQ(WaveformWidgetType::analysisProfile(WaveformWidgetType::RGB),
            mixxx::WaveformAnalysisProfile::Legacy);
    EXPECT_EQ(WaveformWidgetType::analysisProfile(WaveformWidgetType::Stacked),
            mixxx::WaveformAnalysisProfile::Legacy);
    EXPECT_EQ(WaveformWidgetType::analysisProfile(WaveformWidgetType::Perceptual3Band),
            mixxx::WaveformAnalysisProfile::Perceptual3Band);
}

#ifdef MIXXX_USE_QML
TEST(QmlWaveformDisplayTest, doesNotExposePerceptualThreeBand) {
    const QMetaObject& metaObject = mixxx::qml::QmlWaveformDisplay::staticMetaObject;
    const int enumIndex = metaObject.indexOfEnumerator("Type");
    ASSERT_NE(enumIndex, -1);

    const QMetaEnum waveformType = metaObject.enumerator(enumIndex);
    EXPECT_EQ(waveformType.keyToValue("Perceptual3Band"), -1);
}
#endif

} // namespace
