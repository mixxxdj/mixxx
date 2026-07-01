#include "engine/audiolatencycalibrator.h"

#include <QtMath>
#include <algorithm>
#include <cmath>

#include "util/sample.h"

AudioLatencyCalibrator::AudioLatencyCalibrator(QObject* parent)
        : QObject(parent),
          m_state(State::Idle),
          m_sampleRate(48000),
          m_bufferSize(256),
          m_numOutputs(1),
          m_referencePlayhead(0),
          m_referencePlayed(false),
          m_recordStartTime(0) {
}

void AudioLatencyCalibrator::startCalibration(
        int sampleRate, int bufferSize, int numOutputs) {
    m_sampleRate = sampleRate;
    m_bufferSize = bufferSize;
    m_numOutputs = qMax(numOutputs, 1);
    m_referencePlayed = false;
    m_referencePlayhead = 0;
    m_recordedSignal.clear();
    m_recordedSignal.reserve(kMaxRecordingFrames);
    m_referenceSignal.clear();
    m_suggestedOffsetsMs.clear();

    generateReferenceChirp();

    m_state = State::PlayingReference;
    m_timer.start();

    emit statusUpdate(tr("Playing reference pulse through %1 output(s)...")
                    .arg(m_numOutputs));
}

void AudioLatencyCalibrator::stopCalibration() {
    m_state = State::Idle;
    m_referenceSignal.clear();
    m_recordedSignal.clear();
    m_referencePlayhead = 0;
    m_suggestedOffsetsMs.clear();
}

CSAMPLE AudioLatencyCalibrator::generateReferenceFrame(int outputIndex) {
    Q_UNUSED(outputIndex);
    if (m_state != State::PlayingReference) {
        return 0.0;
    }

    if (m_referencePlayhead >= m_referenceSignal.size()) {
        // Done playing reference for all outputs, switch to recording
        m_state = State::RecordingReference;
        emit statusUpdate(
                tr("Now recording (%1 frames)...")
                        .arg(kMaxRecordingFrames));
        return 0.0;
    }

    CSAMPLE value = m_referenceSignal[m_referencePlayhead++];

    // Mark when the pulse begins for timing
    if (!m_referencePlayed && qAbs(value) > 0.01) {
        m_referencePlayed = true;
        m_recordStartTime = m_timer.nsecsElapsed();
    }

    return value;
}

void AudioLatencyCalibrator::addRecordedFrame(CSAMPLE value) {
    if (m_state != State::RecordingReference) {
        return;
    }

    m_recordedSignal.append(value);

    if (m_recordedSignal.size() >= kMaxRecordingFrames) {
        m_state = State::Computing;
        computeOffsets();
    }
}

void AudioLatencyCalibrator::generateReferenceChirp() {
    // Generate a frequency sweep (chirp) from 500Hz to 2000Hz over 100ms.
    // A chirp is easier to detect via cross-correlation than a simple pulse
    // because it has more energy spread over time while maintaining a sharp
    // autocorrelation peak.
    m_referenceSignal.resize(kReferenceLength);
    double freqStart = 500.0;
    double freqEnd = 2000.0;
    for (int i = 0; i < kReferenceLength; ++i) {
        double t = static_cast<double>(i) / m_sampleRate;
        double freq = freqStart +
                (freqEnd - freqStart) *
                        (static_cast<double>(i) / kReferenceLength);
        // Chirp with fade-in/out to avoid clicks
        double envelope = 1.0;
        if (i < 96) {
            envelope = static_cast<double>(i) / 96.0;
        } else if (i > kReferenceLength - 96) {
            envelope = static_cast<double>(kReferenceLength - i) / 96.0;
        }
        m_referenceSignal[i] =
                static_cast<CSAMPLE>(std::sin(2.0 * M_PI * freq * t) * envelope * 0.8);
    }
}

void AudioLatencyCalibrator::computeOffsets() {
    if (m_recordedSignal.isEmpty() || m_referenceSignal.isEmpty()) {
        emit statusUpdate(tr("Calibration failed: no signal recorded"));
        m_state = State::Idle;
        return;
    }

    // Normalize recorded signal
    CSAMPLE maxVal = 0.0;
    for (const CSAMPLE& s : std::as_const(m_recordedSignal)) {
        maxVal = qMax(maxVal, qAbs(s));
    }

    if (maxVal < 0.001) {
        emit statusUpdate(
                tr("Calibration failed: recorded signal too weak. "
                   "Make sure the microphone can hear the output."));
        m_state = State::Idle;
        return;
    }

    // Normalize
    QVector<CSAMPLE> normalized(m_recordedSignal.size());
    for (int i = 0; i < m_recordedSignal.size(); ++i) {
        normalized[i] = m_recordedSignal[i] / maxVal;
    }

    // Cross-correlation: find peaks where reference matches recorded
    int searchEnd = qMin(kCorrelationWindow,
            m_recordedSignal.size() - m_referenceSignal.size());

    // Compute correlation for each offset
    struct Peak {
        int offset;
        double correlation;
    };
    QVector<Peak> peaks;

    for (int offset = 0; offset < searchEnd; ++offset) {
        double correlation = 0.0;
        int compareLength = qMin(m_referenceSignal.size(),
                m_recordedSignal.size() - offset);

        for (int j = 0; j < compareLength; ++j) {
            correlation += normalized[offset + j] * m_referenceSignal[j];
        }

        // Normalize by comparison length
        correlation /= static_cast<double>(compareLength);

        // Track the best peak
        if (peaks.isEmpty() || correlation > peaks[0].correlation) {
            peaks.clear();
            peaks.append({offset, correlation});
        } else if (qAbs(correlation) > 0.3) {
            // Also track significant secondary peaks (from other output devices)
            // A peak is valid if it's separated by at least 10ms from existing peaks
            bool isNew = true;
            for (const auto& p : std::as_const(peaks)) {
                if (qAbs(offset - p.offset) < m_sampleRate / 100) { // 10ms min separation
                    isNew = false;
                    break;
                }
            }
            if (isNew && correlation > 0.5) {
                peaks.append({offset, correlation});
            }
        }
    }

    // Sort peaks by offset (earliest first)
    std::sort(peaks.begin(), peaks.end(), [](const Peak& a, const Peak& b) {
        return a.offset < b.offset;
    });

    if (peaks.isEmpty()) {
        emit statusUpdate(
                tr("Calibration failed: no correlation peaks found"));
        m_state = State::Idle;
        return;
    }

    // Convert offsets from samples to milliseconds
    // peaks[0] is the earliest arrival (lowest latency device) → offset 0
    m_suggestedOffsetsMs.clear();
    for (int i = 0; i < peaks.size(); ++i) {
        if (i == 0) {
            m_suggestedOffsetsMs.append(0.0);
        } else {
            double relativeMs = (static_cast<double>(peaks[i].offset - peaks[0].offset) /
                                        m_sampleRate) *
                    1000.0;
            m_suggestedOffsetsMs.append(relativeMs);
        }
    }

    // If we found fewer peaks than outputs, pad with zeros
    while (m_suggestedOffsetsMs.size() < m_numOutputs) {
        m_suggestedOffsetsMs.append(0.0);
    }

    m_state = State::Idle;
    emit calibrationComplete(m_suggestedOffsetsMs);

    QString result = tr("Calibration complete: found %1 device(s)")
                             .arg(m_suggestedOffsetsMs.size());
    for (int i = 0; i < m_suggestedOffsetsMs.size(); ++i) {
        result += QStringLiteral("\n  Output %1: %2 ms offset")
                          .arg(i)
                          .arg(m_suggestedOffsetsMs[i], 0, 'f', 1);
    }
    emit statusUpdate(result);
}

#include "moc_audiolatencycalibrator.cpp"
