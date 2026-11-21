#pragma once

#include <cmath>
#include <cstdint>
#include <limits>

#include "analyzer/analyzer.h"
#include "library/dao/analysisdao.h"
#include "util/performancetimer.h"
#include "util/sample.h"
#include "waveform/waveform.h"

//NOTS vrince some test to segment sound, to apply color in the waveform
//#define TEST_HEAT_MAP
#ifdef TEST_HEAT_MAP
class QImage;
#endif

class WaveformBandFilters;
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
        // Reset the stride counters
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
                        m_postScaleConversion * m_averageOverallData[i] / m_averageDivisor + 0.5));
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

// The stride positions p > 0 with fmod(p, length) < 1, where a stride of the
// given length ends. They are calculated with exact integer arithmetic.
class WaveformStrideEnds {
  public:
    explicit WaveformStrideEnds(double length = 0);

    // The next position, where a stride ends
    int next() const {
        return m_next;
    }
    void advance();

  private:
    // length = m_lengthFraction / 2^m_shift
    int64_t m_lengthFraction = 1;
    int m_shift = 0;
    // fmod(m_next, length) * 2^m_shift
    int64_t m_remainder = 0;
    int m_next = 0;
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

    void storeCurrentStridePower();
    void resetCurrentStride();

    void createFilters(mixxx::audio::SampleRate sampleRate);
    void destroyFilters();

    mutable AnalysisDao m_analysisDao;

    WaveformPointer m_waveform;
    WaveformPointer m_waveformSummary;
    WaveformData* m_waveformData;
    WaveformData* m_waveformSummaryData;

    WaveformStride m_stride;

    int m_currentStride;
    int m_currentSummaryStride;
    WaveformStrideEnds m_storeEnds;
    WaveformStrideEnds m_summaryEnds;
    mixxx::audio::ChannelCount m_channelCount;

    std::unique_ptr<WaveformBandFilters> m_filters;

    PerformanceTimer m_timer;

#ifdef TEST_HEAT_MAP
    QImage* test_heatMap;
#endif
};
