#ifndef ANALYSER_WAVEFORM_H
#define ANALYSER_WAVEFORM_H

#include <QTime>
#include <QImage>
#include <QSqlDatabase>

#include "analyser.h"
#include "waveform/waveform.h"

#include <limits>

//NOTS vrince some test to segment sound, to apply color in the waveform
//#define TEST_HEAT_MAP

class EngineFilterButterworth8;
class Waveform;
class AnalysisDao;

enum FilterIndex { Low = 0, Mid = 1, High = 2, FilterCount = 3};
enum ChannelIndex { Right = 0, Left = 1, ChannelCount = 2};

class WaveformStride {
    inline void init( int samples) {
        m_length = samples*2;
        m_convertionFactor = (float)std::numeric_limits<unsigned char>::max()/(float)samples;
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
            datum.filtered.all = (unsigned char)(m_convertionFactor * m_overallData[i]);
            datum.filtered.low = (unsigned char)(m_convertionFactor * m_filteredData[i][Low]);
            datum.filtered.mid = (unsigned char)(m_convertionFactor * m_filteredData[i][Mid]);
            datum.filtered.high = (unsigned char)(m_convertionFactor * m_filteredData[i][High]);
        }
    }

  private:
    int m_length;
    int m_position;

    float m_overallData[ChannelCount];
    float m_filteredData[ChannelCount][FilterCount];

    float m_convertionFactor;

  private:
    friend class AnalyserWaveform;
};

class AnalyserWaveform : public Analyser {
  public:
    AnalyserWaveform();
    virtual ~AnalyserWaveform();

    void initialise(TrackPointer tio, int sampleRate, int totalSamples);

    void process(const CSAMPLE *buffer, const int bufferLength);
    void finalise(TrackPointer tio);

  private:
    void storeCurentStridePower();
    void resetCurrentStride();

    void resetFilters(TrackPointer tio);
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

    EngineFilterButterworth8* m_filter[FilterCount];
    std::vector<float> m_buffers[FilterCount];

    QTime* m_timer;
    QSqlDatabase m_database;
    AnalysisDao* m_analysisDao;

#ifdef TEST_HEAT_MAP
    QImage* test_heatMap;
#endif
};

#endif
