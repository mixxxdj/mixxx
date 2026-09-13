#pragma once

#include <cmath>
#include <limits>
#include <vector>

#include "analyzer/analyzer.h"
#include "analyzer/perceptualwaveformlut.h"
#include "library/dao/analysisdao.h"
#include "util/performancetimer.h"
#include "util/sample.h"
#include "waveform/waveform.h"
#include "waveform/waveformanalysisprofile.h"

//NOTS vrince some test to segment sound, to apply color in the waveform
//#define TEST_HEAT_MAP
#ifdef TEST_HEAT_MAP
class QImage;
#endif

class EngineFilterIIRBase;
class QSqlDatabase;

struct WaveformStride {
    WaveformStride(double samples, double averageSamples, int stemCount)
            : m_position(0),
              m_stemCount(stemCount),
              m_length(samples),
              m_averageLength(averageSamples),
              m_averagePosition(0),
              m_averageDivisor(0),
              m_postScaleConversion(static_cast<float>(
                      std::numeric_limits<unsigned char>::max())) {
        reset();
    }

    inline void reset() {
        m_position = 0;
        m_averageDivisor = 0;
        for (int i = 0; i < ChannelCount; ++i) {
            m_overallData[i] = 0.0f;
            m_averageOverallData[i] = 0.0f;
            SampleUtil::clear(m_filteredData[i], BandCount);
            SampleUtil::clear(m_averageFilteredData[i], BandCount);
            if (m_stemCount > 0) {
                SampleUtil::clear(m_stemData[i], m_stemCount);
            } else {
                DEBUG_ASSERT(m_stemCount == 0);
            }
        }
    }

    inline void store(WaveformData* data) {
        for (int i = 0; i < ChannelCount; ++i) {
            WaveformData& datum = *(data + i);
            datum.filtered.all = static_cast<unsigned char>(std::min(255.0,
                    m_postScaleConversion * m_overallData[i] + 0.5));
            datum.filtered.low = static_cast<unsigned char>(std::min(255.0,
                    m_postScaleConversion * m_filteredData[i][Low] + 0.5));
            datum.filtered.mid = static_cast<unsigned char>(std::min(255.0,
                    m_postScaleConversion * m_filteredData[i][Mid] + 0.5));
            datum.filtered.high = static_cast<unsigned char>(std::min(255.0,
                    m_postScaleConversion * m_filteredData[i][High] + 0.5));
            for (int stemIdx = 0; stemIdx < m_stemCount; stemIdx++) {
                datum.stems[stemIdx] = static_cast<unsigned char>(std::min(255.0,
                        m_postScaleConversion * m_stemData[i][stemIdx] + 0.5));
            }
        }
        m_averageDivisor++;
        for (int i = 0; i < ChannelCount; ++i) {
            m_averageOverallData[i] += m_overallData[i];
            m_overallData[i] = 0.0f;
            for (int f = 0; f < BandCount; ++f) {
                m_averageFilteredData[i][f] += m_filteredData[i][f];
                m_filteredData[i][f] = 0.0f;
            }
            for (int stemIdx = 0; stemIdx < m_stemCount; ++stemIdx) {
                m_stemData[i][stemIdx] = 0.0f;
            }
        }
    }

    inline void averageStore(WaveformData* data) {
        if (m_averageDivisor) {
            for (int i = 0; i < ChannelCount; ++i) {
                WaveformData& datum = *(data + i);
                datum.filtered.all = static_cast<unsigned char>(std::min(255.0,
                        m_postScaleConversion * m_averageOverallData[i] / m_averageDivisor +
                                0.5));
                datum.filtered.low = static_cast<unsigned char>(std::min(255.0,
                        m_postScaleConversion * m_averageFilteredData[i][Low] /
                                        m_averageDivisor +
                                0.5));
                datum.filtered.mid = static_cast<unsigned char>(std::min(255.0,
                        m_postScaleConversion * m_averageFilteredData[i][Mid] /
                                        m_averageDivisor +
                                0.5));
                datum.filtered.high = static_cast<unsigned char>(std::min(255.0,
                        m_postScaleConversion * m_averageFilteredData[i][High] /
                                        m_averageDivisor +
                                0.5));
            }
        } else {
            // This is the case if The Overview Waveform has more samples than the detailed waveform
            for (int i = 0; i < ChannelCount; ++i) {
                WaveformData& datum = *(data + i);
                datum.filtered.all = static_cast<unsigned char>(std::min(255.0,
                        m_postScaleConversion * m_overallData[i] + 0.5));
                datum.filtered.low = static_cast<unsigned char>(std::min(255.0,
                        m_postScaleConversion * m_filteredData[i][Low] + 0.5));
                datum.filtered.mid = static_cast<unsigned char>(std::min(255.0,
                        m_postScaleConversion * m_filteredData[i][Mid] + 0.5));
                datum.filtered.high = static_cast<unsigned char>(std::min(255.0,
                        m_postScaleConversion * m_filteredData[i][High] + 0.5));
            }
        }

        m_averageDivisor = 0;
        for (int i = 0; i < ChannelCount; ++i) {
            m_averageOverallData[i] = 0.0f;
            for (int f = 0; f < BandCount; ++f) {
                m_averageFilteredData[i][f] = 0.0f;
            }
        }
    }

    int m_position;
    int m_stemCount;
    double m_length;
    double m_averageLength;
    int m_averagePosition;
    int m_averageDivisor;

    float m_overallData[ChannelCount];
    float m_filteredData[ChannelCount][BandCount];
    float m_stemData[ChannelCount][mixxx::kMaxSupportedStems];

    float m_averageOverallData[ChannelCount];
    float m_averageFilteredData[ChannelCount][BandCount];

    float m_postScaleConversion;
};

struct PerceptualWaveformStride {
    PerceptualWaveformStride(double samples,
            double averageSamples,
            double sampleRate,
            int stemCount)
            : m_position(0),
              m_stemCount(stemCount),
              m_length(samples),
              m_averageLength(averageSamples),
              m_sampleRate(sampleRate),
              m_averagePosition(0),
              m_sampleCount(0),
              m_summarySampleCount(0),
              m_postScaleConversion(static_cast<float>(
                      std::numeric_limits<unsigned char>::max())) {
        reset();
    }

    inline void reset() {
        m_position = 0;
        m_sampleCount = 0;
        m_summarySampleCount = 0;
        for (int i = 0; i < ChannelCount; ++i) {
            m_overallData[i] = 0.0f;
            m_summaryOverallData[i] = 0.0f;
            SampleUtil::clear(m_filteredData[i], BandCount);
            SampleUtil::clear(m_summaryFilteredData[i], BandCount);
            SampleUtil::clear(m_filteredEnvelope[i], BandCount);
            SampleUtil::clear(m_summaryFilteredEnvelope[i], BandCount);
            SampleUtil::clear(m_stemData[i], m_stemCount);
        }
    }

    inline void store(WaveformData* data) {
        for (int i = 0; i < ChannelCount; ++i) {
            WaveformData& datum = *(data + i);
            datum.filtered.all = rmsToByte(m_overallData[i], m_sampleCount);
            datum.filtered.low = rmsEnvelopeToByte(
                    m_filteredData[i][Low], m_sampleCount, &m_filteredEnvelope[i][Low], Low);
            datum.filtered.mid = rmsEnvelopeToByte(
                    m_filteredData[i][Mid], m_sampleCount, &m_filteredEnvelope[i][Mid], Mid);
            datum.filtered.high = rmsEnvelopeToByte(m_filteredData[i][High],
                    m_sampleCount,
                    &m_filteredEnvelope[i][High],
                    High);
            for (int stemIdx = 0; stemIdx < m_stemCount; stemIdx++) {
                datum.stems[stemIdx] = static_cast<unsigned char>(std::min(255.0,
                        m_postScaleConversion * m_stemData[i][stemIdx] + 0.5));
            }
        }
        m_sampleCount = 0;
        for (int i = 0; i < ChannelCount; ++i) {
            m_overallData[i] = 0.0f;
            for (int f = 0; f < BandCount; ++f) {
                m_filteredData[i][f] = 0.0f;
            }
            for (int stemIdx = 0; stemIdx < m_stemCount; ++stemIdx) {
                m_stemData[i][stemIdx] = 0.0f;
            }
        }
    }

    inline void averageStore(WaveformData* data) {
        for (int i = 0; i < ChannelCount; ++i) {
            WaveformData& datum = *(data + i);
            datum.filtered.all = rmsToByte(m_summaryOverallData[i], m_summarySampleCount);
            datum.filtered.low = rmsEnvelopeToByte(m_summaryFilteredData[i][Low],
                    m_summarySampleCount,
                    &m_summaryFilteredEnvelope[i][Low],
                    Low);
            datum.filtered.mid = rmsEnvelopeToByte(m_summaryFilteredData[i][Mid],
                    m_summarySampleCount,
                    &m_summaryFilteredEnvelope[i][Mid],
                    Mid);
            datum.filtered.high = rmsEnvelopeToByte(m_summaryFilteredData[i][High],
                    m_summarySampleCount,
                    &m_summaryFilteredEnvelope[i][High],
                    High);
        }

        m_summarySampleCount = 0;
        for (int i = 0; i < ChannelCount; ++i) {
            m_summaryOverallData[i] = 0.0f;
            for (int f = 0; f < BandCount; ++f) {
                m_summaryFilteredData[i][f] = 0.0f;
            }
        }
    }

  private:
    inline float rms(float sumSquares, int sampleCount) const {
        return sampleCount > 0
                ? std::sqrt(sumSquares / static_cast<float>(sampleCount))
                : 0.0f;
    }

    inline unsigned char valueToByte(float value) const {
        return static_cast<unsigned char>(
                std::min(255.0f, m_postScaleConversion * value + 0.5f));
    }

    inline unsigned char rmsToByte(float sumSquares, int sampleCount) const {
        return valueToByte(rms(sumSquares, sampleCount));
    }

    inline unsigned char rmsEnvelopeToByte(float sumSquares,
            int sampleCount,
            float* envelope,
            int band) const {
        const float releaseTimeSeconds = band == Low
                ? 0.097816f
                : band == Mid ? 0.048390f
                              : 0.017013f;
        const float releaseFactor = std::exp(-static_cast<float>(sampleCount) /
                static_cast<float>(m_sampleRate * releaseTimeSeconds));
        *envelope = std::max(rms(sumSquares, sampleCount), *envelope * releaseFactor);
        const float normalizedEnvelope = std::clamp(*envelope, 0.0f, 1.0f);
        return valueToByte(mixxx::analyzer::PerceptualWaveformLut::applyNormalized(
                normalizedEnvelope, band));
    }

  public:
    int m_position;
    int m_stemCount;
    double m_length;
    double m_averageLength;
    double m_sampleRate;
    int m_averagePosition;
    int m_sampleCount;
    int m_summarySampleCount;

    float m_overallData[ChannelCount];
    float m_filteredData[ChannelCount][BandCount];
    float m_filteredEnvelope[ChannelCount][BandCount];
    float m_stemData[ChannelCount][mixxx::kMaxSupportedStems];

    float m_summaryOverallData[ChannelCount];
    float m_summaryFilteredData[ChannelCount][BandCount];
    float m_summaryFilteredEnvelope[ChannelCount][BandCount];

    float m_postScaleConversion;
};

class AnalyzerWaveform : public Analyzer {
  public:
    AnalyzerWaveform(
            UserSettingsPointer pConfig,
            const QSqlDatabase& dbConnection);
    ~AnalyzerWaveform() override;

    bool initialize(const AnalyzerTrack& track,
            mixxx::audio::SampleRate sampleRate,
            mixxx::audio::ChannelCount channelCount,
            SINT frameLength) override;
    bool processSamples(const CSAMPLE* buffer, SINT count) override;
    void storeResults(TrackPointer tio) override;
    void cleanup() override;

  private:
    bool shouldAnalyze(TrackPointer tio) const;

    void createFilters(mixxx::audio::SampleRate sampleRate);
    void destroyFilters();
    void storeIfGreater(float* pDest, float source);

    UserSettingsPointer m_pConfig;
    mixxx::WaveformAnalysisProfile m_analysisProfile;
    mutable AnalysisDao m_analysisDao;

    WaveformPointer m_waveform;
    WaveformPointer m_waveformSummary;
    WaveformData* m_waveformData;
    WaveformData* m_waveformSummaryData;
    WaveformStride m_legacyStride;
    PerceptualWaveformStride m_perceptualStride;

    int m_currentStride;
    int m_currentSummaryStride;
    mixxx::audio::ChannelCount m_channelCount;

    struct Filters {
        std::unique_ptr<EngineFilterIIRBase> low;
        std::unique_ptr<EngineFilterIIRBase> mid;
        std::unique_ptr<EngineFilterIIRBase> high;
    };

    Filters m_filters;

    struct Buffers {
        std::vector<float> low;
        std::vector<float> mid;
        std::vector<float> high;

        SINT size;

        Buffers()
                : low(),
                  mid(),
                  high(),
                  size(0) {
        }
    };

    Buffers m_buffers;

    PerformanceTimer m_timer;

#ifdef TEST_HEAT_MAP
    QImage* test_heatMap;
#endif
};
