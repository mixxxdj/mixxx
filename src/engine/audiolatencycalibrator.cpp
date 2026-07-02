#include "engine/audiolatencycalibrator.h"

#include <QDebug>
#include <QtMath>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "util/sample.h"

AudioLatencyCalibrator::AudioLatencyCalibrator(QObject* parent)
        : QObject(parent),
          m_state(State::Idle),
          m_sampleRate(48000),
          m_bufferSize(256),
          m_numOutputs(1),
          m_currentOutputIndex(0),
          m_referencePlayhead(0),
          m_pTimeoutTimer(nullptr),
          m_referencePlayed(false),
          m_recordStartTime(0) {
}

void AudioLatencyCalibrator::startCalibration(
        int sampleRate, int bufferSize, int numOutputs) {
    qDebug() << "Calibrator: startCalibration sampleRate=" << sampleRate
             << "bufferSize=" << bufferSize << "numOutputs=" << numOutputs;
    m_sampleRate = sampleRate;
    m_bufferSize = bufferSize;
    m_numOutputs = qMax(numOutputs, 1);
    m_currentOutputIndex = 0;
    m_referencePlayed = false;
    m_referencePlayhead = 0;
    m_recordedSignal.clear();
    m_recordedSignal.reserve(kMaxRecordingFrames);
    m_referenceSignal.clear();
    m_perOutputOffsets.clear();

    generateReferenceChirp();

    m_state = State::PlayingReference;
    m_timer.start();

    // Timeout: 5 seconds per output + 2 seconds safety margin
    int timeoutMs = m_numOutputs * 5000 + 2000;
    if (m_pTimeoutTimer) {
        m_pTimeoutTimer->stop();
    } else {
        m_pTimeoutTimer = new QTimer(this);
        connect(m_pTimeoutTimer, &QTimer::timeout, this, &AudioLatencyCalibrator::onTimeout);
    }
    m_pTimeoutTimer->setSingleShot(true);
    m_pTimeoutTimer->start(timeoutMs);

    emit statusUpdate(tr("Calibrating %1 output(s)...").arg(m_numOutputs));
}

void AudioLatencyCalibrator::stopCalibration() {
    m_state = State::Idle;
    if (m_pTimeoutTimer) {
        m_pTimeoutTimer->stop();
    }
    m_referenceSignal.clear();
    m_recordedSignal.clear();
    m_referencePlayhead = 0;
    m_perOutputOffsets.clear();
}

void AudioLatencyCalibrator::onTimeout() {
    qDebug() << "Calibrator: onTimeout state=" << m_state
             << "recorded=" << m_recordedSignal.size();
    if (m_state == State::RecordingReference && !m_recordedSignal.isEmpty()) {
        // Partial data — compute with what we have
        m_state = State::Computing;
        QTimer::singleShot(0, this, &AudioLatencyCalibrator::computeOffsets);
        return;
    }
    QString msg;
    if (m_recordedSignal.isEmpty()) {
        msg = tr(
                "Calibration timed out: no microphone input received.\n"
                "Make sure a microphone input is configured, audio is running\n"
                "(click Apply), and the phone's mic can hear the outputs.");
    } else {
        msg = tr("Calibration timed out: insufficient data collected.");
    }
    emit statusUpdate(msg);
    m_state = State::Idle;
    m_referenceSignal.clear();
    m_recordedSignal.clear();
    emit calibrationComplete(QVector<double>());
}

CSAMPLE AudioLatencyCalibrator::generateReferenceFrame(int outputIndex) {
    Q_UNUSED(outputIndex);
    if (m_state != State::PlayingReference) {
        return 0.0;
    }

    // The reference signal contains chirps for each output BACK-TO-BACK:
    // [chirp_0][silence_gap][chirp_1][silence_gap]...
    // ALL outputs play the SAME signal, but the recording has
    // time-separated chirps for each output's time slot.
    if (m_referencePlayhead >= m_referenceSignal.size()) {
        // Done playing full sequence — switch to recording
        qDebug() << "Calibrator: reference finished at playhead"
                 << m_referencePlayhead << "of" << m_referenceSignal.size();
        m_state = State::RecordingReference;
        emit statusUpdate(
                tr("Recording microphone input (%1 frames)...")
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

    if (m_recordedSignal.isEmpty()) {
        qDebug() << "Calibrator: first recorded frame received";
    }

    m_recordedSignal.append(value);

    // Log recording progress every 100k frames
    if (m_recordedSignal.size() % 100000 == 0) {
        qDebug() << "Calibrator: recorded" << m_recordedSignal.size()
                 << "frames, max recording =" << kMaxRecordingFrames;
    }

    if (m_recordedSignal.size() >= kMaxRecordingFrames) {
        m_state = State::Computing;
        // Run computeOffsets on the main thread via event loop — never block
        // the audio callback with the expensive cross-correlation.
        QTimer::singleShot(0, this, &AudioLatencyCalibrator::computeOffsets);
    }
}

void AudioLatencyCalibrator::generateReferenceChirp() {
    // Reference signal: a sequence of chirps separated by silence gaps.
    // Each chirp is 400ms, each gap is 200ms.
    // Total duration = numOutputs * (chirpLen + gapLen)
    // This lets us identify which output corresponds to which time window.
    //
    // Each output's chirp sweeps a DIFFERENT frequency band so we can
    // also distinguish them by frequency content even within the same window:
    //   Output 0: 200–800 Hz
    //   Output 1: 800–1400 Hz
    //   Output 2: 1400–2000 Hz
    //   Output 3: 2000–2600 Hz (if more)

    const int chirpLen = m_sampleRate / 2;         // 500ms chirp
    const int gapLen = m_sampleRate / 4;           // 250ms silence
    const int outputLen = chirpLen + gapLen;       // 750ms per output
    const int totalLen = m_numOutputs * outputLen; // total playback time

    m_referenceSignal.resize(totalLen);
    m_referenceSignal.fill(0.0);

    for (int o = 0; o < m_numOutputs; ++o) {
        int startOffset = o * outputLen;
        double freqStart = 200.0 + o * 600.0;
        double freqEnd = freqStart + 600.0;

        for (int i = 0; i < chirpLen; ++i) {
            double t = static_cast<double>(i) / m_sampleRate;
            double freq = freqStart +
                    (freqEnd - freqStart) *
                            (static_cast<double>(i) / chirpLen);
            // Fade-in/out to avoid clicks
            double envelope = 1.0;
            if (i < 128) {
                envelope = static_cast<double>(i) / 128.0;
            } else if (i > chirpLen - 128) {
                envelope = static_cast<double>(chirpLen - i) / 128.0;
            }
            m_referenceSignal[startOffset + i] =
                    static_cast<CSAMPLE>(
                            std::sin(2.0 * M_PI * freq * t) * envelope * 0.7);
        }
        // The gap (startOffset + chirpLen ... startOffset + outputLen) is already 0
    }

    emit statusUpdate(tr("Playing %1s reference through all outputs...")
                    .arg(totalLen / static_cast<double>(m_sampleRate), 0, 'f', 1));
}

void AudioLatencyCalibrator::computeOffsets() {
    qDebug() << "Calibrator: computeOffsets recorded="
             << m_recordedSignal.size() << "reference="
             << m_referenceSignal.size() << "numOutputs=" << m_numOutputs;

    if (m_recordedSignal.isEmpty() || m_referenceSignal.isEmpty()) {
        qWarning() << "Calibrator: no signal recorded — recorded empty?"
                   << m_recordedSignal.isEmpty() << "ref empty?"
                   << m_referenceSignal.isEmpty();
        emit statusUpdate(tr("Calibration failed: no signal recorded"));
        m_state = State::Idle;
        return;
    }

    // Check signal energy: if the recorded signal is silence, the mic
    // stream is not capturing any audio.
    double peakEnergy = 0.0;
    double sumEnergy = 0.0;
    for (int i = 0; i < m_recordedSignal.size(); ++i) {
        double s = static_cast<double>(m_recordedSignal[i]);
        double e = s * s;
        peakEnergy = qMax(peakEnergy, e);
        sumEnergy += e;
    }
    double avgEnergy = sumEnergy / m_recordedSignal.size();
    qDebug() << "Calibrator: recorded signal peak=" << peakEnergy
             << "avg=" << avgEnergy << "sum=" << sumEnergy;
    if (peakEnergy < 1e-6) {
        qWarning() << "Calibrator: recorded signal is effectively silence"
                   << "(peak energy" << peakEnergy
                   << "). Check mic input routing.";
        emit statusUpdate(
                tr("Calibration failed: microphone recorded silence. "
                   "Make sure audio is running and the mic can hear the outputs."));
        m_state = State::Idle;
        emit calibrationComplete(QVector<double>());
        return;
    }

    // Build per-output reference chirps (just the chirp sections, no gaps)
    struct OutputChirp {
        int chirpOffsetInSignal; // sample offset in m_referenceSignal
        int chirpLen;
        QVector<CSAMPLE> samples;
    };

    const int chirpLen = m_sampleRate / 2;
    const int gapLen = m_sampleRate / 4;
    const int outputLen = chirpLen + gapLen;

    QVector<OutputChirp> outputChirps;
    for (int o = 0; o < m_numOutputs; ++o) {
        int startOffset = o * outputLen;
        outputChirps.append({startOffset, chirpLen, m_referenceSignal.mid(startOffset, chirpLen)});
    }

    // Cross-correlate the recorded signal with each output's reference chirp
    struct Peak {
        int offset;
        double correlation;
        int outputIndex;
    };
    QVector<Peak> peaks;

    int recordLen = m_recordedSignal.size();

    for (int o = 0; o < m_numOutputs; ++o) {
        const auto& chirp = outputChirps[o].samples;
        if (chirp.isEmpty()) {
            continue;
        }

        // Search range: the chirp could start anywhere from offset 0 to
        // the end of recording minus the chirp length, plus a bit extra
        // for the output's known position in the signal.
        int searchStart = 0;
        int searchEnd = qMin(recordLen - chirp.size(), kCorrelationWindow);

        double bestCorrelation = 0.0;
        int bestOffset = 0;

        for (int offset = searchStart; offset < searchEnd; ++offset) {
            double correlation = 0.0;
            int compareLen = qMin(chirp.size(), recordLen - offset);

            for (int j = 0; j < compareLen; ++j) {
                correlation += static_cast<double>(
                                       m_recordedSignal[offset + j]) *
                        chirp[j];
            }

            // Normalize by length
            correlation /= static_cast<double>(compareLen);

            if (correlation > bestCorrelation) {
                bestCorrelation = correlation;
                bestOffset = offset;
            }
        }

        if (bestCorrelation > 0.1) {
            peaks.append({bestOffset, bestCorrelation, o});
            qDebug() << "Calibrator: output" << o << "peak at offset"
                     << bestOffset << "correlation=" << bestCorrelation;
        } else {
            qDebug() << "Calibrator: output" << o << "NO peak (best="
                     << bestCorrelation << ")";
        }
    }

    if (peaks.isEmpty()) {
        qWarning() << "Calibrator: no peaks found for any of"
                   << m_numOutputs << "outputs";
        emit statusUpdate(
                tr("Calibration failed: no peaks found. "
                   "Make sure outputs are audible to the microphone."));
        m_state = State::Idle;
        emit calibrationComplete(QVector<double>());
        return;
    }

    // Sort peaks by offset (earliest = lowest latency)
    std::sort(peaks.begin(), peaks.end(), [](const Peak& a, const Peak& b) {
        return a.offset < b.offset;
    });

    // The earliest peak's device offset = 0. All others are relative to it.
    int baseOffset = peaks[0].offset;

    m_perOutputOffsets.clear();
    m_perOutputOffsets.resize(m_numOutputs, 0.0);

    for (const auto& peak : std::as_const(peaks)) {
        double relativeMs = (static_cast<double>(peak.offset - baseOffset) /
                                    m_sampleRate) *
                1000.0;
        m_perOutputOffsets[peak.outputIndex] = relativeMs;
    }

    m_state = State::Idle;

    QString result = tr("Calibration complete!\n");
    for (int i = 0; i < m_numOutputs; ++i) {
        result += tr("  Output %1: %2 ms offset\n")
                          .arg(i + 1)
                          .arg(m_perOutputOffsets[i], 0, 'f', 1);
    }
    result += tr("(Offset relative to earliest output)");
    emit statusUpdate(result);

    emit calibrationComplete(m_perOutputOffsets);
}

#include "moc_audiolatencycalibrator.cpp"
