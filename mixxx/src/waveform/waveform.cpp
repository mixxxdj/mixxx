#include <cmath>
#include <QtDebug>

#include "waveform/waveform.h"
#include "proto/waveform.pb.h"

using namespace mixxx::track;

Waveform::Waveform(const QByteArray data)
        : m_id(-1),
          m_bDirty(true),
          m_numChannels(2),
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

QByteArray Waveform::toByteArray() const {
    io::Waveform waveform;
    waveform.set_visual_sample_rate(m_visualSampleRate);
    waveform.set_audio_visual_ratio(m_audioVisualRatio);

    io::Waveform::Signal* all = waveform.mutable_signal_all();
    io::Waveform::FilteredSignal* filtered = waveform.mutable_signal_filtered();
    // TODO(rryan) get the actual cutoff values from analyserwaveform.cpp so
    // that if they change we don't have to remember to update these.

    // Frequency cutoffs for butterworth filters:
    // filtered->set_low_cutoff_frequency(200);
    // filtered->set_mid_low_cutoff_frequency(200);
    // filtered->set_mid_high_cutoff_frequency(2000);
    // filtered->set_high_cutoff_frequency(2000);

    // Frequency cutoff for bessel_lowpass4
    filtered->set_low_cutoff_frequency(600);
    // Frequency cutoff for bessel_bandpass
    filtered->set_mid_low_cutoff_frequency(600);
    filtered->set_mid_high_cutoff_frequency(4000);
    // Frequency cutoff for bessel_highpass4
    filtered->set_high_cutoff_frequency(4000);

    io::Waveform::Signal* low = filtered->mutable_low();
    io::Waveform::Signal* mid = filtered->mutable_mid();
    io::Waveform::Signal* high = filtered->mutable_high();

    // TODO(vrince) set max/min for each signal
    all->set_units(io::Waveform::RMS);
    all->set_channels(m_numChannels);
    low->set_units(io::Waveform::RMS);
    low->set_channels(m_numChannels);
    mid->set_units(io::Waveform::RMS);
    mid->set_channels(m_numChannels);
    high->set_units(io::Waveform::RMS);
    high->set_channels(m_numChannels);

    for (int i = 0; i < m_dataSize; ++i) {
        const WaveformData& datum = m_data.at(i);
        all->add_value(datum.filtered.all);
        low->add_value(datum.filtered.low);
        mid->add_value(datum.filtered.mid);
        high->add_value(datum.filtered.high);
    }

    qDebug() << "Writing waveform from byte array:"
             << "dataSize" << m_dataSize
             << "allSignalSize" << all->value_size()
             << "visualSampleRate" << waveform.visual_sample_rate()
             << "audioVisualRatio" << waveform.audio_visual_ratio();

    std::string output;
    waveform.SerializeToString(&output);
    return QByteArray(output.data(), output.length());
}

void Waveform::readByteArray(const QByteArray data) {
    io::Waveform waveform;

    if (!waveform.ParseFromArray(data.constData(), data.size())) {
        qDebug() << "ERROR: Could not parse Waveform from QByteArray of size"
                 << data.size();
        return;
    }

    if (!waveform.has_visual_sample_rate() ||
        !waveform.has_audio_visual_ratio() ||
        !waveform.has_signal_all() ||
        !waveform.has_signal_filtered() ||
        !waveform.signal_filtered().has_low() ||
        !waveform.signal_filtered().has_mid() ||
        !waveform.signal_filtered().has_high()) {
        qDebug() << "ERROR: Waveform proto is missing key data. Skipping.";
        return;
    }

    const io::Waveform::Signal& all = waveform.signal_all();
    const io::Waveform::Signal& low = waveform.signal_filtered().low();
    const io::Waveform::Signal& mid = waveform.signal_filtered().mid();
    const io::Waveform::Signal& high = waveform.signal_filtered().high();

    qDebug() << "Reading waveform from byte array:"
             << "allSignalSize" << all.value_size()
             << "visualSampleRate" << waveform.visual_sample_rate()
             << "audioVisualRatio" << waveform.audio_visual_ratio();

    resize(all.value_size());

    if (all.value_size() != m_dataSize) {
        qDebug() << "ERROR: Couldn't resize Waveform to" << all.value_size()
                 << "while reading.";
        resize(0);
        return;
    }

    m_visualSampleRate = waveform.visual_sample_rate();
    m_audioVisualRatio = waveform.audio_visual_ratio();
    if (low.value_size() != m_dataSize ||
        mid.value_size() != m_dataSize ||
        high.value_size() != m_dataSize) {
        qDebug() << "WARNING: Filtered data size does not match all-signal size.";
    }

    // TODO(XXX) If non-RMS, convert but since we only save RMS today we can add
    // this later.
    bool low_valid = low.units() == io::Waveform::RMS;
    bool mid_valid = mid.units() == io::Waveform::RMS;
    bool high_valid = high.units() == io::Waveform::RMS;
    for (int i = 0; i < m_dataSize; ++i) {
        m_data[i].filtered.all = static_cast<unsigned char>(all.value(i));
        bool use_low = low_valid && i < low.value_size();
        bool use_mid = mid_valid && i < mid.value_size();
        bool use_high = high_valid && i < high.value_size();
        m_data[i].filtered.low = use_low ? static_cast<unsigned char>(low.value(i)) : 0;
        m_data[i].filtered.mid = use_mid ? static_cast<unsigned char>(mid.value(i)) : 0;
        m_data[i].filtered.high = use_high ? static_cast<unsigned char>(high.value(i)) : 0;
    }
    m_completion = m_dataSize;
    m_bDirty = false;
}

void Waveform::reset() {
    m_dataSize = 0;
    m_textureStride = 1024;
    m_completion = -1;
    m_audioSamplesPerVisualSample = 0;
    m_visualSampleRate = 0;
    m_audioVisualRatio = 0;
    m_data.clear();
    m_bDirty = true;
}

void Waveform::computeBestVisualSampleRate( int audioSampleRate, double desiredVisualSampleRate) {
    m_audioSamplesPerVisualSample = std::floor((double)audioSampleRate / desiredVisualSampleRate);
    const double actualVisualSamplingRate = (double)audioSampleRate / (double)(m_audioSamplesPerVisualSample);

    m_visualSampleRate = actualVisualSamplingRate;
    m_audioVisualRatio = (double)audioSampleRate / (double)m_visualSampleRate;
}

void Waveform::allocateForAudioSamples(int audioSamples) {
    double actualSize = audioSamples / m_audioSamplesPerVisualSample;
    int numberOfVisualSamples = static_cast<int>(actualSize) + 1;
    numberOfVisualSamples += numberOfVisualSamples%2;
    assign(numberOfVisualSamples, 0);
    setCompletion(0);
    m_bDirty = true;
}

void Waveform::resize(int size) {
    m_dataSize = size;
    int textureSize = computeTextureSize(size);
    m_data.resize(textureSize);
    m_bDirty = true;
}

void Waveform::assign(int size, int value) {
    m_dataSize = size;
    int textureSize = computeTextureSize(size);
    m_data.assign(textureSize, value);
    m_bDirty = true;
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
