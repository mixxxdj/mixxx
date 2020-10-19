#include <QtDebug>

#include "waveform/waveform.h"
#include "proto/waveform.pb.h"

using namespace mixxx::track;

const int kNumChannels = 2;

// Return the smallest power of 2 which is greater than the desired size when
// squared.
int computeTextureStride(int size) {
    int stride = 256;
    while (stride * stride < size) {
        stride *= 2;
    }
    return stride;
}

Waveform::Waveform(const QByteArray data)
        : m_id(-1),
          m_saveState(SaveState::NotSaved),
          m_dataSize(0),
          m_visualSampleRate(0),
          m_audioVisualRatio(0),
          m_textureStride(computeTextureStride(0)),
          m_completion(-1) {
    readByteArray(data);
}

Waveform::Waveform(int audioSampleRate, int audioSamples,
                   int desiredVisualSampleRate, int maxVisualSamples)
        : m_id(-1),
          m_saveState(SaveState::NotSaved),
          m_dataSize(0),
          m_visualSampleRate(0),
          m_audioVisualRatio(0),
          m_textureStride(1024),
          m_completion(-1) {
    int numberOfVisualSamples = 0;
    if (audioSampleRate > 0) {
        if (maxVisualSamples == -1) {
            // Waveform
            if (desiredVisualSampleRate < audioSampleRate) {
                m_visualSampleRate =
                        static_cast<double>(desiredVisualSampleRate);
            } else {
                m_visualSampleRate = static_cast<double>(audioSampleRate);
            }
        } else {
            // Waveform Summary (Overview)
            if (audioSamples > maxVisualSamples) {
                m_visualSampleRate = (double)maxVisualSamples *
                        (double)audioSampleRate / (double)audioSamples;
            } else {
                m_visualSampleRate = audioSampleRate;
            }
        }
        m_audioVisualRatio = (double)audioSampleRate / (double)m_visualSampleRate;
        numberOfVisualSamples = static_cast<int>(audioSamples / m_audioVisualRatio) + 1;
        numberOfVisualSamples += numberOfVisualSamples%2;
    }
    assign(numberOfVisualSamples, 0);
    setCompletion(0);
}

Waveform::~Waveform() {
}

QByteArray Waveform::toByteArray() const {
    io::Waveform waveform;
    waveform.set_visual_sample_rate(m_visualSampleRate);
    waveform.set_audio_visual_ratio(m_audioVisualRatio);

    io::Waveform::Signal* all = waveform.mutable_signal_all();
    io::Waveform::FilteredSignal* filtered = waveform.mutable_signal_filtered();
    // TODO(rryan) get the actual cutoff values from analyzerwaveform.cpp so
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
    int numChannels = kNumChannels;
    all->set_units(io::Waveform::RMS);
    all->set_channels(numChannels);
    low->set_units(io::Waveform::RMS);
    low->set_channels(numChannels);
    mid->set_units(io::Waveform::RMS);
    mid->set_channels(numChannels);
    high->set_units(io::Waveform::RMS);
    high->set_channels(numChannels);

    int dataSize = getDataSize();
    for (int i = 0; i < dataSize; ++i) {
        const WaveformData& datum = m_data.at(i);
        all->add_value(datum.filtered.all);
        low->add_value(datum.filtered.low);
        mid->add_value(datum.filtered.mid);
        high->add_value(datum.filtered.high);
    }

    qDebug() << "Writing waveform from byte array:"
             << "dataSize" << dataSize
             << "allSignalSize" << all->value_size()
             << "visualSampleRate" << waveform.visual_sample_rate()
             << "audioVisualRatio" << waveform.audio_visual_ratio();

    std::string output;
    waveform.SerializeToString(&output);
    return QByteArray(output.data(), output.length());
}

void Waveform::readByteArray(const QByteArray& data) {
    if (data.isNull()) {
        return;
    }

    io::Waveform waveform;

    if (!waveform.ParseFromArray(data.constData(), data.size())) {
        qDebug() << "ERROR: Could not parse Waveform from QByteArray of size "
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

    int dataSize = getDataSize();
    if (all.value_size() != dataSize) {
        qDebug() << "ERROR: Couldn't resize Waveform to" << all.value_size()
                 << "while reading.";
        resize(0);
        m_saveState = SaveState::NotSaved;
        return;
    }

    m_visualSampleRate = waveform.visual_sample_rate();
    m_audioVisualRatio = waveform.audio_visual_ratio();
    if (low.value_size() != dataSize ||
        mid.value_size() != dataSize ||
        high.value_size() != dataSize) {
        qDebug() << "WARNING: Filtered data size does not match all-signal size.";
    }

    // TODO(XXX) If non-RMS, convert but since we only save RMS today we can add
    // this later.
    bool low_valid = low.units() == io::Waveform::RMS;
    bool mid_valid = mid.units() == io::Waveform::RMS;
    bool high_valid = high.units() == io::Waveform::RMS;
    for (int i = 0; i < dataSize; ++i) {
        m_data[i].filtered.all = static_cast<unsigned char>(all.value(i));
        bool use_low = low_valid && i < low.value_size();
        bool use_mid = mid_valid && i < mid.value_size();
        bool use_high = high_valid && i < high.value_size();
        m_data[i].filtered.low = use_low ? static_cast<unsigned char>(low.value(i)) : 0;
        m_data[i].filtered.mid = use_mid ? static_cast<unsigned char>(mid.value(i)) : 0;
        m_data[i].filtered.high = use_high ? static_cast<unsigned char>(high.value(i)) : 0;
    }
    m_completion = dataSize;
    m_saveState = SaveState::Saved;
}

void Waveform::resize(int size) {
    m_dataSize = size;
    m_textureStride = computeTextureStride(size);
    m_data.resize(m_textureStride * m_textureStride);
}

void Waveform::assign(int size, int value) {
    m_dataSize = size;
    m_textureStride = computeTextureStride(size);
    m_data.assign(m_textureStride * m_textureStride, value);
    m_saveState = SaveState::SavePending;
}

void Waveform::dump() const {
    qDebug() << "Waveform" << this
             << "size("+QString::number(getDataSize())+")"
             << "textureStride("+QString::number(m_textureStride)+")"
             << "completion("+QString::number(getCompletion())+")"
             << "visualSampleRate("+QString::number(m_visualSampleRate)+")"
             << "audioVisualRatio("+QString::number(m_audioVisualRatio)+")";
}
