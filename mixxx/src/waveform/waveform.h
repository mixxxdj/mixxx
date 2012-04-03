#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <QMutex>
#include <QByteArray>
#include <QString>
#include <vector>

#include "util.h"


// WARNING: Do not change this struct without double-checking you are not
// changing its byte layout. This is important for serialization of the
// waveform.
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
    Waveform(const QByteArray pData = QByteArray());
    virtual ~Waveform();

    int getId() const {
        return m_id;
    }
    void setId(int id) {
        m_id = id;
    }
    QString getVersion() const {
        return m_version;
    }
    void setVersion(QString version) {
        m_version = version;
    }
    QString getDescription() const {
        return m_description;
    }
    void setDescription(QString description) {
        m_description = description;
    }
    QByteArray toByteArray() const;

    void reset();

    double getVisualSampleRate() const { return m_visualSampleRate;}
    double getAudioVisualRatio() const { return m_audioVisualRatio;}
    int getAudioSamplesPerVisualSample() const { return m_audioSamplesPerVisualSample;}
    int getCompletion() const { return m_completion;}
    int getTextureStride() const { return m_textureStride;}
    int getTextureSize() const { return m_data.size();}

    double getActualSize() const { return m_actualSize;}
    int getDataSize() const { return m_dataSize; }

    const std::vector<WaveformData>& getConstData() const { return m_data;}

    inline const WaveformData& get(int i) const { return m_data[i];}
    inline unsigned char getLow(int i) const { return m_data[i].filtered.low;}
    inline unsigned char getMid(int i) const { return m_data[i].filtered.mid;}
    inline unsigned char getHigh(int i) const { return m_data[i].filtered.high;}
    inline unsigned char getAll(int i) const { return m_data[i].filtered.all;}

    WaveformData* data() { return &m_data[0];}
    const WaveformData* data() const { return &m_data[0];}

    inline QMutex* getMutex() const {
        return m_mutex;
    }

    void dump() const;

  private:
    void readByteArray(const QByteArray data);
    void resize(int size);
    void assign(int size, int value = 0);

    void computeBestVisualSampleRate( int audioSampleRate, double desiredVisualSampleRate);
    void allocateForAudioSamples( int audioSamples);

    inline WaveformData& at(int i) { return m_data[i];}
    inline unsigned char& low(int i) { return m_data[i].filtered.low;}
    inline unsigned char& mid(int i) { return m_data[i].filtered.mid;}
    inline unsigned char& high(int i) { return m_data[i].filtered.high;}
    inline unsigned char& all(int i) { return m_data[i].filtered.all;}

    int computeTextureSize(int getDataSize);
    void setCompletion(int completion) { m_completion = completion;}

    // If stored in the database, the ID of the waveform.
    int m_id;
    QString m_version;
    QString m_description;

    double m_actualSize; //actual song size in visual world
    int m_dataSize; //m_data allocated size
    std::vector<WaveformData> m_data;
    int m_audioSamplesPerVisualSample;
    double m_visualSampleRate;
    double m_audioVisualRatio;

    //No need to store the following members they can be recomputed
    //on waveform laoding

    int m_textureStride;
    int m_completion;

    mutable QMutex* m_mutex;

    friend class AnalyserWaveform;
    friend class WaveformStride;

    DISALLOW_COPY_AND_ASSIGN(Waveform);
};

#endif // WAVEFORM_H
