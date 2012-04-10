#include "waveform/waveform.h"
#include <cmath>
#include <QtDebug>

Waveform::Waveform(const QByteArray data)
        : m_id(-1),
          m_actualSize(0.0),
          m_dataSize(0),
          m_audioSamplesPerVisualSample(0),
          m_visualSampleRate(0),
          m_audioVisualRatio(0),
          m_textureStride(1024),
          m_completion(-1),
          m_mutex(new QMutex()) {
    if (!data.isNull()) {
        readByteArray(data);
    }
}

Waveform::~Waveform() {
    delete m_mutex;
}

struct SerializedWaveformHeader {
    double actualSize;
    double dataSize;
    double visualSampleRate;
    double audioVisualRatio;
};

QByteArray Waveform::toByteArray() const {
    SerializedWaveformHeader header;
    header.actualSize = m_actualSize;
    header.dataSize = m_dataSize;
    header.visualSampleRate = m_visualSampleRate;
    header.audioVisualRatio = m_audioVisualRatio;
    QByteArray data;
    data.append(reinterpret_cast<const char*>(&header), sizeof(header));
    data.append(reinterpret_cast<const char*>(&m_data[0]),
                sizeof(m_data[0])*m_dataSize);
    return data;
}

void Waveform::readByteArray(const QByteArray data) {
    int headerBytes = sizeof(SerializedWaveformHeader);
    int dataBytes = data.size() - headerBytes;
    if (dataBytes < 0) {
        qDebug() << "Error reading waveform, not enough bytes to parse:"
                 << data.size();
        return;
    }
    const SerializedWaveformHeader* header =
            reinterpret_cast<const SerializedWaveformHeader*>(data.constData());

    if (int(header->dataSize) * sizeof(WaveformData) != dataBytes) {
        qDebug() << "Parse failure -- did not get number of bytes expected"
                 << m_dataSize * sizeof(WaveformData) << "vs" << dataBytes;
        return;
    }

    qDebug() << "Reading waveform from byte array:"
             << "actualSize" << header->actualSize
             << "dataSize" << header->dataSize
             << "visualSampleRate" << header->visualSampleRate
             << "audioVisualRatio" << header->audioVisualRatio;
    m_actualSize = header->actualSize;
    m_dataSize = int(header->dataSize);
    m_visualSampleRate = header->visualSampleRate;
    m_audioVisualRatio = header->audioVisualRatio;
    resize(m_dataSize);
    memcpy(&m_data[0], data.constData() + headerBytes, m_dataSize * sizeof(m_data[0]));
    m_completion = m_dataSize;
}

void Waveform::reset() {
    m_actualSize = 0.0;
    m_dataSize = 0;
    m_textureStride = 1024;
    m_completion = -1;
    m_audioSamplesPerVisualSample = 0;
    m_visualSampleRate = 0;
    m_audioVisualRatio = 0;
    m_data.clear();
}

void Waveform::computeBestVisualSampleRate( int audioSampleRate, double desiredVisualSampleRate) {
    m_audioSamplesPerVisualSample = std::floor((double)audioSampleRate / desiredVisualSampleRate);
    const double actualVisualSamplingRate = (double)audioSampleRate / (double)(m_audioSamplesPerVisualSample);

    m_visualSampleRate = actualVisualSamplingRate;
    m_audioVisualRatio = (double)audioSampleRate / (double)m_visualSampleRate;
}

void Waveform::allocateForAudioSamples(int audioSamples) {
    m_actualSize = audioSamples / m_audioSamplesPerVisualSample;
    int numberOfVisualSamples = static_cast<int>(m_actualSize) + 1;
    numberOfVisualSamples += numberOfVisualSamples%2;
    assign(numberOfVisualSamples, 0);
    setCompletion(0);
}

void Waveform::resize(int size) {
    m_dataSize = size;
    int textureSize = computeTextureSize(size);
    m_data.resize(textureSize);
}

void Waveform::assign(int size, int value) {
    m_dataSize = size;
    int textureSize = computeTextureSize(size);
    m_data.assign(textureSize, value);
}

int Waveform::computeTextureSize(int size) {

    //find the best match
    //NOTE vRince : I know 'what about actual coding ? ...'

    if( size <= 256*256) { //~1min @441Hz stereo
        m_textureStride = 256;
        return 256*256;
    } else if( size <= 512*512) { //~9min @441Hz stereo
        m_textureStride = 512;
        return 512*512;
    } else if( size <= 1024*1024) { //~19min @441Hz stereo
        m_textureStride = 1024;
        return 1024*1024;
    } else if( size <= 2048*2048) { //~79min @441Hz stereo
        m_textureStride = 2048;
        return 2048*2048;
    } else {  //~317min @441Hz stereo

        if( size > 4096*4096)
            qDebug() << "Waveform::computeTextureSize - this look like a really big song ...";

        m_textureStride = 4096;
        return 4096*4096;
    }
}

void Waveform::dump() const {
    qDebug() << "Waveform" << this
             << "size("+QString::number(m_dataSize)+")"
             << "textureStride("+QString::number(m_textureStride)+")"
             << "completion("+QString::number(m_completion)+")"
             << "audioSamplesPerVisualSample("+QString::number(m_audioSamplesPerVisualSample)+")"
             << "visualSampleRate("+QString::number(m_visualSampleRate)+")"
             << "audioVisualRatio("+QString::number(m_audioVisualRatio)+")";
}
