#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <QMutex>
#include <vector>

#include "util.h"

union WaveformData {
    struct {
        unsigned char low;
        unsigned char mid;
        unsigned char high;
        unsigned char all;
    } filtered;
    int m_i;

    WaveformData() {}
    WaveformData(int i) { m_i = i;}
};

class Waveform {
  public:
    Waveform();
    virtual ~Waveform();

    double getVisualSampleRate() const { return m_visualSampleRate;}
    double getAudioVisualRatio() const { return m_audioVisualRatio;}
    int getAudioSamplesPerVisualSample() const { return m_audioSamplesPerVisualSample;}
    int getCompletion() const { return m_completion;}
    int getTextureStride() const { return m_textureStride;}
    int getTextureSize() const { return m_data.size();}

    int size() const { return m_size; }

    const std::vector<WaveformData>& getConstData() const { return m_data;}

    inline const WaveformData& get(int i) const { return m_data[i];}
    inline unsigned char getLow(int i) const { return m_data[i].filtered.low;}
    inline unsigned char getMid(int i) const { return m_data[i].filtered.mid;}
    inline unsigned char getHigh(int i) const { return m_data[i].filtered.high;}
    inline unsigned char getAll(int i) const { return m_data[i].filtered.all;}

    WaveformData* data() { return &m_data[0];}
    const WaveformData* data() const { return &m_data[0];}

    QMutex* getMutex() { return m_mutex;}

    void dump() const;

  private:
    void resize(int size);
    void assign(int size, int value = 0);

    void computeBestVisualSampleRate( int audioSampleRate, double desiredVisualSampleRate);
    void allocateForAudioSamples( int audioSamples);

    inline WaveformData& at(int i) { return m_data[i];}
    inline unsigned char& low(int i) { return m_data[i].filtered.low;}
    inline unsigned char& mid(int i) { return m_data[i].filtered.mid;}
    inline unsigned char& high(int i) { return m_data[i].filtered.high;}
    inline unsigned char& all(int i) { return m_data[i].filtered.all;}

    int computeTextureSize(int size);
    void setCompletion(int completion) { m_completion = completion;}

    unsigned int m_size; //actual useful size
    unsigned int m_textureStride;
    std::vector<WaveformData> m_data;
    int m_completion;

    int m_audioSamplesPerVisualSample;
    double m_visualSampleRate;
    double m_audioVisualRatio;
    QMutex* m_mutex;

    friend class AnalyserWaveform;
    friend class WaveformStride;
    DISALLOW_COPY_AND_ASSIGN(Waveform);
};

#endif // WAVEFORM_H
