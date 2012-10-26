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
    inline void init( int samples) {
        m_length = samples*2;
        m_postScaleConversion = (float)std::numeric_limits<unsigned char>::max();
        m_conversionFactor = 1.0; //because we are taking a max, not an average any more

        // This averages over the window. For now we're taking the max-envelope.
        //m_conversionFactor = 1.0 / samples;
        reset();
    }

    inline void reset() {
        m_position = 0;
        for( int i = 0; i < ChannelCount; i++) {
            m_overallData[i] = 0.0f;
            for( int f = 0; f < FilterCount; f++) {
                m_filteredData[i][f] = 0.0f;
            }
        }
    }

    inline void store(WaveformData* data) {
        for( int i = 0; i < ChannelCount; i++) {
            WaveformData& datum = *(data + i);
            datum.filtered.all = static_cast<unsigned char>(math_min(255.0, m_postScaleConversion * scaleSignal(m_conversionFactor * m_overallData[i]) + 0.5));
            datum.filtered.low = static_cast<unsigned char>(math_min(255.0, m_postScaleConversion * scaleSignal(m_conversionFactor * m_filteredData[i][Low], Low) + 0.5));
            datum.filtered.mid = static_cast<unsigned char>(math_min(255.0, m_postScaleConversion * scaleSignal(m_conversionFactor * m_filteredData[i][Mid], Mid) + 0.5));
            datum.filtered.high = static_cast<unsigned char>(math_min(255.0, m_postScaleConversion * scaleSignal(m_conversionFactor * m_filteredData[i][High], High) + 0.5));
        }
    }

  private:
    int m_length;
    int m_position;

    float m_overallData[ChannelCount];
    float m_filteredData[ChannelCount][FilterCount];

    float m_conversionFactor;
    float m_postScaleConversion;

  private:
    friend class AnalyserWaveform;
};

class AnalyserWaveform : public Analyser {
  public:
    AnalyserWaveform(ConfigObject<ConfigValue>* pConfig);
    virtual ~AnalyserWaveform();

    bool initialise(TrackPointer tio, int sampleRate, int totalSamples);

    void process(const CSAMPLE *buffer, const int bufferLength);
    void cleanup(TrackPointer tio);
    void finalise(TrackPointer tio);

  private:
    void storeCurentStridePower();
    void resetCurrentStride();

    void resetFilters(TrackPointer tio, int sampleRate);
    void destroyFilters();

  private:
    bool m_skipProcessing;

    Waveform* m_waveform;
    Waveform* m_waveformSummary;
    int m_waveformDataSize;
    int m_waveformSummaryDataSize;
    WaveformData* m_waveformData;
    WaveformData* m_waveformSummaryData;

    WaveformStride m_stride;
    WaveformStride m_strideSummary;

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
