#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <QVector>

#include "util.h"

class Waveform {
  public:
    enum WaveformFilteredType { NotFiltered = 0,
                                LowPass = 1,
                                MidPass = 2,
                                HighPass = 3,
                                FilteredTypeCount = 4};

    Waveform();
    virtual ~Waveform();

    void setSampleRate(double sample) {
        m_sampleRate = sample;
    }

    double getSampleRate() const {
        m_sampleRate;
    }

    int size() const {
        return m_data[NotFiltered].size();
    }

    void resize(int size);
    void reset(unsigned char value = 0);
    void assign(int size, unsigned char value = 0);

    const QVector<unsigned char>& getConstData() const { return m_data[NotFiltered];}
    const QVector<unsigned char>& getConstLowData() const { return m_data[LowPass];}
    const QVector<unsigned char>& getConstMidData() const { return m_data[MidPass];}
    const QVector<unsigned char>& getConstHighData() const { return m_data[HighPass];}

  private:
    QVector<unsigned char>& getData() { return m_data[NotFiltered];}
    QVector<unsigned char>& getLowData() { return m_data[LowPass];}
    QVector<unsigned char>& getMidData() { return m_data[MidPass];}
    QVector<unsigned char>& getHighData() { return m_data[HighPass];}
    QVector<unsigned char> m_data[FilteredTypeCount];
    double m_sampleRate;

    friend class AnalyserWaveform;
    DISALLOW_COPY_AND_ASSIGN(Waveform);
};

#endif // WAVEFORM_H
