#ifndef ANALYSER_WAVEFORM_H
#define ANALYSER_WAVEFORM_H

#include <QTime>
#include <QImage>
#include <QSqlDatabase>

#include "configobject.h"
#include "analyser.h"
#include "waveform/waveform.h"

#include <limits>

//NOTS vrince some test to segment sound, to apply color in the waveform
//#define TEST_HEAT_MAP

class EngineObject;
class EngineFilterButterworth8;
class EngineFilterIIR;
class Waveform;
class AnalysisDao;

enum FilterIndex { Low = 0, Mid = 1, High = 2, FilterCount = 3};
enum ChannelIndex { Left = 0, Right = 1, ChannelCount = 2};

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

class WaveformStride {
    inline void init(double samples, double averageSamples) {
        m_length = samples;
        m_averageLength = averageSamples;
        m_postScaleConversion = (float)std::numeric_limits<unsigned char>::max();
        m_position = 0;
        m_averagePosition = 0;
        m_averageDivisor = 0;
        for( int i = 0; i < ChannelCount; i++) {
            m_overallData[i] = 0.0f;
            m_averageOverallData[i] = 0.0f;
            for( int f = 0; f < FilterCount; f++) {
                m_filteredData[i][f] = 0.0f;
                m_averageFilteredData[i][f] = 0.0f;
            }
        }
    }

    inline void reset() {
        m_position = 0;
        m_averageDivisor = 0;
        for( int i = 0; i < ChannelCount; i++) {
            m_overallData[i] = 0.0f;
            m_averageOverallData[i] = 0.0f;
            for( int f = 0; f < FilterCount; f++) {
                m_filteredData[i][f] = 0.0f;
                m_averageFilteredData[i][f] = 0.0f;
            }
        }
    }

    inline void store(WaveformData* data) {
        for( int i = 0; i < ChannelCount; i++) {
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
        for( int i = 0; i < ChannelCount; i++) {
            m_averageOverallData[i] += m_overallData[i];
            m_overallData[i] = 0.0f;
            for( int f = 0; f < FilterCount; f++) {
                m_averageFilteredData[i][f] += m_filteredData[i][f];
                m_filteredData[i][f] = 0.0f;
            }
        }
    }

    inline void averageStore(WaveformData* data) {
        if (m_averageDivisor) {
            for( int i = 0; i < ChannelCount; i++) {
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
            for( int i = 0; i < ChannelCount; i++) {
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
        for( int i = 0; i < ChannelCount; i++) {
            m_averageOverallData[i] = 0.0f;
            for( int f = 0; f < FilterCount; f++) {
                m_averageFilteredData[i][f] = 0.0f;
            }
        }
    }

  private:
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

  private:
    friend class AnalyserWaveform;
};

class AnalyserWaveform : public Analyser {
  public:
    AnalyserWaveform(ConfigObject<ConfigValue>* pConfig);
    virtual ~AnalyserWaveform();

    bool initialise(TrackPointer tio, int sampleRate, int totalSamples);
    bool loadStored(TrackPointer tio) const;
    void process(const CSAMPLE *buffer, const int bufferLength);
    void cleanup(TrackPointer tio);
    void finalise(TrackPointer tio);

  private:
    void storeCurentStridePower();
    void resetCurrentStride();

    void resetFilters(TrackPointer tio, int sampleRate);
    void destroyFilters();
    void storeIfGreater(float* pDest, float source);

  private:
    bool m_skipProcessing;

    Waveform* m_waveform;
    Waveform* m_waveformSummary;
    int m_waveformDataSize;
    int m_waveformSummaryDataSize;
    WaveformData* m_waveformData;
    WaveformData* m_waveformSummaryData;

    WaveformStride m_stride;

    int m_currentStride;
    int m_currentSummaryStride;

    EngineObject* m_filter[FilterCount];
    std::vector<float> m_buffers[FilterCount];

    QTime* m_timer;
    QSqlDatabase m_database;
    AnalysisDao* m_analysisDao;

#ifdef TEST_HEAT_MAP
    QImage* test_heatMap;
#endif
};

#endif
