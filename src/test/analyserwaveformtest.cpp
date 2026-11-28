#include <gtest/gtest.h>

#include <QDir>
#include <QtDebug>
#include <cmath>
#include <random>
#include <vector>

#include "analyzer/analyzertrack.h"
#include "analyzer/analyzerwaveform.h"
#include "engine/filters/enginefilterbessel4.h"
#include "library/dao/analysisdao.h"
#include "test/mixxxtest.h"
#include "track/track.h"

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

// The stride ends must be the positions, where the original implementation
// stored a stride: fmod(position, length) < 1
TEST(WaveformStrideEndsTest, matchesFmod) {
    const double lengths[] = {0.5,
            1.0,
            1.5,
            100.0,
            22050.0 / 441,
            48000.0 / 441,
            96000.0 / 441,
            127.99999999,
            128.0,
            12345678.0 / 1920,
            987654.0 / 1920};
    for (double length : lengths) {
        WaveformStrideEnds ends(length);
        for (int position = 1; position < 100000; ++position) {
            const bool expected = std::fmod(position, length) < 1;
            const bool actual = position == ends.next();
            ASSERT_EQ(expected, actual) << "length " << length << " position " << position;
            if (actual) {
                ends.advance();
            }
        }
    }
}

// Reference implementation of the waveform analysis with the engine filters
// and a stride check per sample.
void referenceWaveform(const std::vector<CSAMPLE>& samples,
        mixxx::audio::SampleRate sampleRate,
        Waveform* pWaveform,
        Waveform* pSummary) {
    EngineFilterBessel4Low low(sampleRate, 600);
    EngineFilterBessel4Band mid(sampleRate, 600, 4000);
    EngineFilterBessel4High high(sampleRate, 4000);
    low.assumeSettled();
    mid.assumeSettled();
    high.assumeSettled();
    WaveformStride stride(pWaveform->getAudioVisualRatio(), pSummary->getAudioVisualRatio(), 0);
    int currentStride = 0;
    int currentSummaryStride = 0;
    const SINT chunkSize = 2 * mixxx::kAnalysisFramesPerChunk;
    std::vector<CSAMPLE> lowBuf(chunkSize);
    std::vector<CSAMPLE> midBuf(chunkSize);
    std::vector<CSAMPLE> highBuf(chunkSize);
    for (std::size_t offset = 0; offset < samples.size(); offset += chunkSize) {
        const SINT count = std::min<SINT>(chunkSize, samples.size() - offset);
        const CSAMPLE* pIn = &samples[offset];
        low.process(pIn, lowBuf.data(), count);
        mid.process(pIn, midBuf.data(), count);
        high.process(pIn, highBuf.data(), count);
        for (SINT i = 0; i < count; i += 2) {
            for (int ch = 0; ch < 2; ++ch) {
                stride.m_overallData[ch] = std::max(
                        stride.m_overallData[ch], std::fabs(pIn[i + ch]));
                stride.m_filteredData[ch][Low] = std::max(
                        stride.m_filteredData[ch][Low], std::fabs(lowBuf[i + ch]));
                stride.m_filteredData[ch][Mid] = std::max(
                        stride.m_filteredData[ch][Mid], std::fabs(midBuf[i + ch]));
                stride.m_filteredData[ch][High] = std::max(
                        stride.m_filteredData[ch][High], std::fabs(highBuf[i + ch]));
            }
            stride.m_position++;
            if (std::fmod(stride.m_position, stride.m_length) < 1) {
                ASSERT_LE(currentStride + 2, pWaveform->getDataSize());
                stride.store(pWaveform->data() + currentStride);
                currentStride += 2;
            }
            if (std::fmod(stride.m_position, stride.m_averageLength) < 1) {
                ASSERT_LE(currentSummaryStride + 2, pSummary->getDataSize());
                stride.averageStore(pSummary->data() + currentSummaryStride);
                currentSummaryStride += 2;
            }
        }
    }
}

void expectSameData(const Waveform& expected, const Waveform& actual) {
    ASSERT_EQ(expected.getDataSize(), actual.getDataSize());
    for (int i = 0; i < expected.getDataSize(); ++i) {
        const WaveformFilteredData& e = expected.data()[i].filtered;
        const WaveformFilteredData& a = actual.data()[i].filtered;
        // Allow rounding differences of the filters on other CPUs
        ASSERT_LE(std::abs(e.all - a.all), 1) << i;
        ASSERT_LE(std::abs(e.low - a.low), 1) << i;
        ASSERT_LE(std::abs(e.mid - a.mid), 1) << i;
        ASSERT_LE(std::abs(e.high - a.high), 1) << i;
    }
}

TEST_F(AnalyzerWaveformTest, matchesReference) {
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> noise(-1.0f, 1.0f);
    for (int sampleRate : {22050, 44100, 48000, 96000}) {
        // Noise with changing level, some seconds and a partial chunk
        const SINT frames = 3 * sampleRate + 1234;
        std::vector<CSAMPLE> samples(2 * frames);
        for (SINT i = 0; i < frames; ++i) {
            const float level = 0.5f + 0.5f * std::sin(i * 6.0f / sampleRate);
            samples[2 * i] = level * noise(rng);
            samples[2 * i + 1] = (1.0f - level) * noise(rng);
        }

        auto pTrack = Track::newTemporary();
        AnalyzerWaveform analyzer(config(), QSqlDatabase());
        ASSERT_TRUE(analyzer.initialize(AnalyzerTrack(pTrack),
                mixxx::audio::SampleRate(sampleRate),
                mixxx::audio::ChannelCount(2),
                frames));
        for (SINT frame = 0; frame < frames; frame += mixxx::kAnalysisFramesPerChunk) {
            const SINT count = std::min<SINT>(mixxx::kAnalysisFramesPerChunk, frames - frame);
            ASSERT_TRUE(analyzer.processSamples(&samples[2 * frame], 2 * count));
        }
        analyzer.storeResults(pTrack);
        analyzer.cleanup();

        Waveform waveform(sampleRate, frames, 441, -1, 0);
        Waveform summary(sampleRate, frames, 441, 2 * 1920, 0);
        referenceWaveform(samples, mixxx::audio::SampleRate(sampleRate), &waveform, &summary);
        expectSameData(waveform, *pTrack->getWaveform());
        expectSameData(summary, *pTrack->getWaveformSummary());
    }
}

} // namespace
