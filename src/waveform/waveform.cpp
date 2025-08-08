#include "waveform/waveform.h"

#include <QVariant>
#include <QtDebug>

#include "analyzer/constants.h"
#include "engine/engine.h"
#include "moc_waveform.cpp"
#include "proto/waveform.pb.h"
#include "util/assert.h"

using namespace mixxx::track;

// Return the smallest power of 2 which is greater than the desired size when
// squared.
int computeTextureStride(int size) {
    int stride = 256;
    while (stride * stride < size) {
        stride *= 2;
    }
    return stride;
}

Waveform::Waveform(const QByteArray& data)
        : m_id(-1),
          m_saveState(SaveState::NotSaved),
          m_dataSize(0),
          m_visualSampleRate(0),
          m_audioVisualRatio(0),
          m_textureStride(computeTextureStride(0)),
          m_completion(-1) {
    readByteArray(data);
}

Waveform::Waveform(
        int audioSampleRate,
        SINT frameLength,
        int desiredVisualSampleRate,
        int maxVisualSamples,
        int stemCount,
        Sampling samplingMode)
        : m_id(-1),
          m_saveState(SaveState::NotSaved),
          m_dataSize(0),
          m_visualSampleRate(0),
          m_audioVisualRatio(0),
          m_textureStride(1024),
          m_completion(-1),
          m_stemCount(stemCount),
          m_sampling(samplingMode) {
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
            if (frameLength > maxVisualSamples / mixxx::kAnalysisChannels) {
                m_visualSampleRate = static_cast<double>(audioSampleRate) *
                        maxVisualSamples / mixxx::kAnalysisChannels / frameLength;
            } else {
                m_visualSampleRate = audioSampleRate;
            }
        }
        m_audioVisualRatio = (double)audioSampleRate / (double)m_visualSampleRate;
        numberOfVisualSamples =
                static_cast<int>(frameLength / m_audioVisualRatio *
                        mixxx::kAnalysisChannels) +
                1;
        numberOfVisualSamples += numberOfVisualSamples%2;
    }
    assign(numberOfVisualSamples);
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

    QList<io::Waveform::Signal*> stems;
    for (int i = 0; i < m_stemCount; i++) {
        stems.append(waveform.add_signal_stems());
    }
    // TODO(rryan) get the actual cutoff values from analyzerwaveform.cpp so
    // that if they change we don't have to remember to update these.

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
    auto unit = m_sampling == Sampling::MAX ? io::Waveform::AMPLITUDE : io::Waveform::RMS;
    all->set_units(unit);
    all->set_channels(mixxx::kEngineChannelOutputCount);
    low->set_units(unit);
    low->set_channels(mixxx::kEngineChannelOutputCount);
    mid->set_units(unit);
    mid->set_channels(mixxx::kEngineChannelOutputCount);
    high->set_units(unit);
    high->set_channels(mixxx::kEngineChannelOutputCount);

    int dataSize = getDataSize();
    for (int i = 0; i < dataSize; ++i) {
        const WaveformData& datum = m_data.at(i);
        all->add_value(datum.filtered.all);
        low->add_value(datum.filtered.low);
        mid->add_value(datum.filtered.mid);
        high->add_value(datum.filtered.high);
    }

    int stemIdx = 0;
    for (auto& stem : stems) {
        stem->set_units(unit);
        stem->set_channels(mixxx::kEngineChannelOutputCount);
        for (int i = 0; i < dataSize; ++i) {
            const WaveformData& datum = m_data.at(i);
            stem->add_value(datum.stems[stemIdx]);
        }
        stemIdx++;
    }

    qDebug() << "Writing waveform from byte array:"
             << "dataSize" << dataSize
             << "stemCount" << m_stemCount
             << "allSignalSize" << all->value_size()
             << "visualSampleRate" << waveform.visual_sample_rate()
             << "audioVisualRatio" << waveform.audio_visual_ratio();

    std::string output;
    waveform.SerializeToString(&output);
    return QByteArray(output.data(), static_cast<int>(output.length()));
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
             << "audioVisualRatio" << waveform.audio_visual_ratio()
             << "stemSignalSize" << waveform.signal_stems_size();

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

    m_sampling = low.units() == io::Waveform::RMS ? Sampling::RMS : Sampling::MAX;
    DEBUG_ASSERT(mid.units() == low.units() && mid.units() == high.units());

    for (int i = 0; i < dataSize; ++i) {
        m_data[i].filtered.all = static_cast<unsigned char>(all.value(i));
        m_data[i].filtered.low = i < low.value_size()
                ? static_cast<unsigned char>(low.value(i))
                : 0;
        m_data[i].filtered.mid = i < mid.value_size()
                ? static_cast<unsigned char>(mid.value(i))
                : 0;
        m_data[i].filtered.high = i < high.value_size()
                ? static_cast<unsigned char>(high.value(i))
                : 0;
    }
    m_stemCount = waveform.signal_stems_size();
    for (int stemIdx = 0; stemIdx < m_stemCount; ++stemIdx) {
        const io::Waveform::Signal& stem = waveform.signal_stems(stemIdx);
        if (stem.value_size() > 0) {
            for (int i = 0; i < std::min(dataSize, stem.value_size()); ++i) {
                m_data[i].stems[stemIdx] = static_cast<unsigned char>(stem.value(i));
            }
        }
        if (dataSize > stem.value_size()) {
            for (int i = stem.value_size(); i < dataSize; ++i) {
                m_data[i].stems[stemIdx] = 0;
            }
        }
    }

    m_completion = dataSize;
    m_saveState = SaveState::Saved;
}

void Waveform::resize(int size) {
    m_dataSize = size;
    m_textureStride = computeTextureStride(size);
    m_data.resize(m_textureStride * m_textureStride);
}

void Waveform::assign(int size) {
    m_dataSize = size;
    m_textureStride = computeTextureStride(size);
    m_data.assign(m_textureStride * m_textureStride, {});
    m_saveState = SaveState::SavePending;
}

void Waveform::dump() const {
    qDebug() << "Waveform" << this
             << "size(" + QString::number(getDataSize()) + ")"
             << "sampling(" + QVariant::fromValue(m_sampling).toString() + ")"
             << "stems(" + QString::number(m_stemCount) + ")"
             << "textureStride(" + QString::number(m_textureStride) + ")"
             << "completion(" + QString::number(getCompletion()) + ")"
             << "visualSampleRate(" + QString::number(m_visualSampleRate) + ")"
             << "audioVisualRatio(" + QString::number(m_audioVisualRatio) + ")";
}
