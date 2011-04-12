#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <QVector>

class Waveform
{
public:
    enum WaveformFilteredType { NotFiltered = 0,
                                LowPass = 1,
                                BandPass = 2,
                                HighPass = 3,
                                FilteredTypeCount = 4};

    Waveform();
    virtual ~Waveform();

    int size() const { return m_data[NotFiltered].size();}

    void resize( int size);
    void reset( unsigned char value = 0);
    void assign( int size, unsigned char value = 0);

    const QVector<unsigned char>& getConstData() const { return m_data[NotFiltered];}
    const QVector<unsigned char>& getConstLowData() const { return m_data[LowPass];}
    const QVector<unsigned char>& getConstBandData() const { return m_data[BandPass];}
    const QVector<unsigned char>& getConstHighData() const { return m_data[HighPass];}

private:
    QVector<unsigned char>& getData() { return m_data[NotFiltered];}
    QVector<unsigned char>& getLowData() { return m_data[LowPass];}
    QVector<unsigned char>& getBandData() { return m_data[BandPass];}
    QVector<unsigned char>& getHighData() { return m_data[HighPass];}

private:
    QVector<unsigned char> m_data[FilteredTypeCount];

    friend class AnalyserWaveform;
};

#endif // WAVEFORM_H
