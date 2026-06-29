#include "engine/audiolatencycalibrator.h"

#include <QtMath>
#include <algorithm>
#include <cmath>

#include "util/sample.h"

AudioLatencyCalibrator::AudioLatencyCalibrator(QObject* parent)
        : QObject(parent),
          m_state(State::Idle),
          m_suggestedOffsetMs(0.0),
          m_manualOffsetMs(0.0),
          m_sampleRate(48000),
          m_bufferSize(256),
          m_referencePlayhead(0),
          m_recordStartTime(0),
          m_referencePlayed(false) {
}

void AudioLatencyCalibrator::startCalibration(int sampleRate, int bufferSize) {
    m_sampleRate = sampleRate;
    m_bufferSize = bufferSize;
    m_referencePlayed = false;
    m_referencePlayhead = 0;
    m_recordedSignal.clear();
    m_recordedSignal.reserve(kMaxRecordingFrames);
    m_referenceSignal.clear();

    generateReferencePulse();

    m_state = State::PlayingReference;
    m_timer.start();

    emit statusUpdate("Playing reference tone...");
}

void AudioLatencyCalibrator::stopCalibration() {
    m_state = State::Idle;
    m_referenceSignal.clear();
    m_recordedSignal.clear();
    m_referencePlayhead = 0;
}

CSAMPLE AudioLatencyCalibrator::generateReferenceFrame() {
    if (m_state != State::PlayingReference) {
        return 0.0;
    }

    if (m_referencePlayhead >= m_referenceSignal.size()) {
        // Done playing reference, switch to recording
        m_state = State::RecordingReference;
        emit statusUpdate("Now recording - listen for the click return.");
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
        computeOffset();
    }
}

void AudioLatencyCalibrator::generateReferencePulse() {
    // Generate a short click (pulse of 1.0 values followed by zeros)
    // This is easy to detect via cross-correlation
    m_referenceSignal.resize(kReferencePulseLength);
    for (int i = 0; i < kReferencePulseLength; ++i) {
        // A single-sample impulse is ideal for cross-correlation
        // Use a short burst for robustness
        if (i < 48) { // 1ms at 48kHz
            m_referenceSignal[i] = 1.0f;
        } else {
            m_referenceSignal[i] = 0.0f;
        }
    }
}

void AudioLatencyCalibrator::computeOffset() {
    if (m_recordedSignal.isEmpty() || m_referenceSignal.isEmpty()) {
        emit statusUpdate("Calibration failed: no signal recorded");
        m_state = State::Idle;
        return;
    }

    // Normalize recorded signal
    CSAMPLE maxVal = 0.0;
    for (const CSAMPLE& s : std::as_const(m_recordedSignal)) {
        maxVal = qMax(maxVal, qAbs(s));
    }

    if (maxVal < 0.001) {
        emit statusUpdate("Calibration failed: recorded signal too weak");
        m_state = State::Idle;
        return;
    }

    // Cross-correlation: find the offset where reference best matches recorded
    int searchStart = 0; // Start searching from beginning
    int searchEnd = qMin(kCorrelationWindow, m_recordedSignal.size() - kReferencePulseLength);

    CSAMPLE bestCorrelation = -1.0;
    int bestOffset = 0;

    for (int offset = searchStart; offset < searchEnd; ++offset) {
        CSAMPLE correlation = 0.0;
        int compareLength = qMin(kReferencePulseLength, m_recordedSignal.size() - offset);

        for (int j = 0; j < compareLength; ++j) {
            correlation += m_recordedSignal[offset + j] * m_referenceSignal[j];
        }

        // Normalize by reference energy
        correlation /= static_cast<CSAMPLE>(compareLength);

        if (correlation > bestCorrelation) {
            bestCorrelation = correlation;
            bestOffset = offset;
        }
    }

    // Convert offset from samples to milliseconds
    m_suggestedOffsetMs = (static_cast<double>(bestOffset) /
                                  static_cast<double>(m_sampleRate)) *
            1000.0;

    m_state = State::Idle;
    emit calibrationComplete(m_suggestedOffsetMs);
    emit statusUpdate(QString("Calibration complete. Suggested offset: %1 ms")
                    .arg(m_suggestedOffsetMs, 0, 'f', 1));
}

#include "moc_audiolatencycalibrator.cpp"
