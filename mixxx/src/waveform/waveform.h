#ifndef WAVEFORM_H
#define WAVEFORM_H

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

    void setVisualSampleRate(double rate) {
        m_visualSampleRate = rate;
    }

    double getVisualSampleRate() const {
        return m_visualSampleRate;
    }

    void setAudioVisualRatio(double ratio) {
        m_audioVisualRatio = ratio;
    }

    double getAudioVisualRatio() const {
        return m_audioVisualRatio;
    }

    int getTextureStride() const { return m_textureStride;}
    int getTextureSize() const { return m_data.size();}

    int size() const { return m_size; }

    void resize(int size);
    void reset(int value = 0);
    void assign(int size, int value = 0);

    const std::vector<WaveformData>& getConstData() const { return m_data;}

    inline WaveformData& get(int i) { return m_data[i];}
    inline unsigned char getLow(int i) const { return m_data[i].filtered.low;}
    inline unsigned char getMid(int i) const { return m_data[i].filtered.mid;}
    inline unsigned char getHigh(int i) const { return m_data[i].filtered.high;}
    inline unsigned char getAll(int i) const { return m_data[i].filtered.all;}

    WaveformData* data() { return &m_data[0];}
    const WaveformData* data() const { return &m_data[0];}

private:
    inline WaveformData& at(int i) { return m_data[i];}
    inline unsigned char& low(int i) { return m_data[i].filtered.low;}
    inline unsigned char& mid(int i) { return m_data[i].filtered.mid;}
    inline unsigned char& high(int i) { return m_data[i].filtered.high;}
    inline unsigned char& all(int i) { return m_data[i].filtered.all;}

    int computeTextureSize(int size);

    unsigned int m_size; //actual usefull size
    unsigned int m_textureStride;
    std::vector<WaveformData> m_data;

    double m_visualSampleRate;
    double m_audioVisualRatio;

    friend class AnalyserWaveform;
    DISALLOW_COPY_AND_ASSIGN(Waveform);
};

#endif // WAVEFORM_H
