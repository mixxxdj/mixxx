#pragma once

#include <QImage>
#include <QSqlDatabase>

#include <limits>

#include "analyzer/analyzer.h"
#include "library/dao/analysisdao.h"
#include "util/math.h"
#include "util/performancetimer.h"
#include "waveform/waveform.h"

//NOTS vrince some test to segment sound, to apply color in the waveform
//#define TEST_HEAT_MAP

class EngineFilterIIRBase;

inline CSAMPLE scaleSignal(CSAMPLE invalue, FilterIndex index = FilterCount) {
    if (invalue == 0.0) {
        return 0;
    } else if (index == Low || index == Mid) {
        //return pow(invalue, 2 * 0.5);
        return invalue;
    } else {
        return pow(invalue, 2.0f * 0.316f);
    }
}

struct WaveformStride {
    WaveformStride(double samples, double averageSamples)
            : m_position(0),
              m_length(samples),
              m_averageLength(averageSamples),
              m_averagePosition(0),
              m_averageDivisor(0),
              m_postScaleConversion(static_cast<float>(
                      std::numeric_limits<unsigned char>::max())) {
        for (int i = 0; i < ChannelCount; ++i) {
            m_overallData[i] = 0.0f;
            m_averageOverallData[i] = 0.0f;
            for (int f = 0; f < FilterCount; ++f) {
                m_filteredData[i][f] = 0.0f;
                m_averageFilteredData[i][f] = 0.0f;
            }
        }
    }

    inline void reset() {
        m_position = 0;
        m_averageDivisor = 0;
        for (int i = 0; i < ChannelCount; ++i) {
            m_overallData[i] = 0.0f;
            m_averageOverallData[i] = 0.0f;
            for (int f = 0; f < FilterCount; ++f) {
                m_filteredData[i][f] = 0.0f;
                m_averageFilteredData[i][f] = 0.0f;
            }
        }
    }

    inline void store(WaveformData* data) {
        for (int i = 0; i < ChannelCount; ++i) {
            WaveformData& datum = *(data + i);
            datum.filtered.all = static_cast<unsigned char>(math_min(255.0,
                    m_postScaleConversion * scaleSignal(m_overallData[i]) + 0.5));
            datum.filtered.low = static_cast<unsigned char>(math_min(255.0,
                    m_postScaleConversion * scaleSignal(m_filteredData[i][Low], Low) + 0.5));
            datum.filtered.mid = static_cast<unsigned char>(math_min(255.0,
                    m_postScaleConversion * scaleSignal(m_filteredData[i][Mid], Mid) + 0.5));
            datum.filtered.high = static_cast<unsigned char>(math_min(255.0,
                    m_postScaleConversion * scaleSignal(m_filteredData[i][High], High) + 0.5));
        }
        m_averageDivisor++;
        for (int i = 0; i < ChannelCount; ++i) {
            m_averageOverallData[i] += m_overallData[i];
            m_overallData[i] = 0.0f;
            for (int f = 0; f < FilterCount; ++f) {
                m_averageFilteredData[i][f] += m_filteredData[i][f];
                m_filteredData[i][f] = 0.0f;
            }
        }
    }

    inline void averageStore(WaveformData* data) {
        if (m_averageDivisor) {
            for (int i = 0; i < ChannelCount; ++i) {
                WaveformData& datum = *(data + i);
                datum.filtered.all = static_cast<unsigned char>(math_min(255.0,
                        m_postScaleConversion * scaleSignal(m_averageOverallData[i] / m_averageDivisor) + 0.5));
                datum.filtered.low = static_cast<unsigned char>(math_min(255.0,
                        m_postScaleConversion * scaleSignal(m_averageFilteredData[i][Low] / m_averageDivisor, Low) + 0.5));
                datum.filtered.mid = static_cast<unsigned char>(math_min(255.0,
                        m_postScaleConversion * scaleSignal(m_averageFilteredData[i][Mid] / m_averageDivisor, Mid) + 0.5));
                datum.filtered.high = static_cast<unsigned char>(math_min(255.0,
                        m_postScaleConversion * scaleSignal(m_averageFilteredData[i][High] / m_averageDivisor, High) + 0.5));
            }
        } else {
            // This is the case if The Overview Waveform has more samples than the detailed waveform
            for (int i = 0; i < ChannelCount; ++i) {
                WaveformData& datum = *(data + i);
                datum.filtered.all = static_cast<unsigned char>(math_min(255.0,
                        m_postScaleConversion * scaleSignal(m_overallData[i]) + 0.5));
                datum.filtered.low = static_cast<unsigned char>(math_min(255.0,
                        m_postScaleConversion * scaleSignal(m_filteredData[i][Low], Low) + 0.5));
                datum.filtered.mid = static_cast<unsigned char>(math_min(255.0,
                        m_postScaleConversion * scaleSignal(m_filteredData[i][Mid], Mid) + 0.5));
                datum.filtered.high = static_cast<unsigned char>(math_min(255.0,
                        m_postScaleConversion * scaleSignal(m_filteredData[i][High], High) + 0.5));
            }
        }

        m_averageDivisor = 0;
        for (int i = 0; i < ChannelCount; ++i) {
            m_averageOverallData[i] = 0.0f;
            for (int f = 0; f < FilterCount; ++f) {
                m_averageFilteredData[i][f] = 0.0f;
            }
        }
    }

    int m_position;
    double m_length;
    double m_averageLength;
    int m_averagePosition;
    int m_averageDivisor;

    float m_overallData[ChannelCount];
    float m_filteredData[ChannelCount][FilterCount];

    float m_averageOverallData[ChannelCount];
    float m_averageFilteredData[ChannelCount][FilterCount];

    float m_postScaleConversion;
};

class AnalyzerWaveform : public Analyzer {
  public:
    AnalyzerWaveform(
            UserSettingsPointer pConfig,
            const QSqlDatabase& dbConnection);
    ~AnalyzerWaveform() override;

    bool initialize(TrackPointer tio, int sampleRate, int totalSamples) override;
    bool processSamples(const CSAMPLE* buffer, const int bufferLength) override;
    void storeResults(TrackPointer tio) override;
    void cleanup() override;

  private:
    bool shouldAnalyze(TrackPointer tio) const;

    void storeCurrentStridePower();
    void resetCurrentStride();

    void createFilters(int sampleRate);
    void destroyFilters();
    void storeIfGreater(float* pDest, float source);

    mutable AnalysisDao m_analysisDao;

    WaveformPointer m_waveform;
    WaveformPointer m_waveformSummary;
    WaveformData* m_waveformData;
    WaveformData* m_waveformSummaryData;

    WaveformStride m_stride;

    int m_currentStride;
    int m_currentSummaryStride;

    EngineFilterIIRBase* m_filter[FilterCount];
    std::vector<float> m_buffers[FilterCount];

    PerformanceTimer m_timer;

#ifdef TEST_HEAT_MAP
    QImage* test_heatMap;
#endif
};
