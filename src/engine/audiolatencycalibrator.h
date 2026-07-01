#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>
#include <QVector>

#include "engine/engine.h"
#include "util/types.h"

/**
 * AudioLatencyCalibrator implements active latency calibration using a
 * play-and-record loopback approach inspired by the osu! framework's
 * BeatmapOffsetControl and LatencyCertifierScreen.
 *
 * Calibration modes:
 * 1. Per-device: play reference pulse through one output, record via input,
 *    cross-correlate to find that device's absolute latency.
 * 2. Multi-device: play reference pulse through ALL outputs simultaneously,
 *    record the combined audio (e.g. via phone microphone), find multiple
 *    correlation peaks, set the earliest peak as offset 0 and the rest
 *    as relative delays.
 *
 * Usage:
 *   auto* cal = new AudioLatencyCalibrator(this);
 *   connect(cal, &AudioLatencyCalibrator::calibrationComplete,
 *           this, [](const QVector<double>& offsetsMs) {
 *               // offsetsMs[i] = offset for output i, first is 0
 *           });
 *   cal->startCalibration(sampleRate, bufferSize, numOutputs);
 *   // Feed reference frames via generateReferenceFrame() to output(s)
 *   // Feed recorded frames via addRecordedFrame() from input
 *   // calibrator emits calibrationComplete when done
 */
class AudioLatencyCalibrator : public QObject {
    Q_OBJECT
  public:
    enum class State {
        Idle,
        PlayingReference,
        RecordingReference,
        Computing,
    };

    explicit AudioLatencyCalibrator(QObject* parent = nullptr);
    ~AudioLatencyCalibrator() override = default;

    State getState() const {
        return m_state;
    }

    /// Start the calibration process for numOutputs devices.
    /// Plays a reference pulse and records it back.
    void startCalibration(int sampleRate, int bufferSize, int numOutputs = 1);

    /// Stop calibration and reset to idle.
    void stopCalibration();

    /// Get the suggested offsets in milliseconds (size = numOutputs).
    /// offsets[0] is always 0 (the earliest device).
    QVector<double> getSuggestedOffsetsMs() const {
        return m_suggestedOffsetsMs;
    }

    /// Add a recorded sample frame from the input device.
    void addRecordedFrame(CSAMPLE value);

    /// Generate the next reference frame to play on output device index.
    /// @param outputIndex which output this frame is for (0 = clock ref)
    CSAMPLE generateReferenceFrame(int outputIndex = 0);

    /// Called on timeout - stops calibration gracefully.
    void onTimeout();

  signals:
    /// Emitted when calibration completes with per-output offsets in ms.
    /// offsets[0] = 0 (earliest device).
    void calibrationComplete(const QVector<double>& offsetsMs);

    /// Emitted periodically with current status.
    void statusUpdate(const QString& status);

  private:
    void computeOffsets();
    void generateReferenceChirp();

    State m_state;
    QVector<double> m_suggestedOffsetsMs;

    int m_sampleRate;
    int m_bufferSize;
    int m_numOutputs;

    // Reference signal (chirp/pulse)
    QVector<CSAMPLE> m_referenceSignal;
    int m_referencePlayhead;

    // Recorded signal storage
    QVector<CSAMPLE> m_recordedSignal;

    // Timing
    QElapsedTimer m_timer;
    QTimer* m_pTimeoutTimer;
    bool m_referencePlayed;
    qint64 m_recordStartTime;

    static constexpr int kReferenceLength = 4800;     // ~100ms at 48kHz
    static constexpr int kMinReferenceGap = 480;      // ~10ms gap between pulses
    static constexpr int kMaxRecordingFrames = 96000; // 2 seconds at 48kHz
    static constexpr int kCorrelationWindow = 48000;  // Search window (1 second)
};
