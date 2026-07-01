#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>
#include <QVector>

#include "engine/engine.h"

/// Measures relative latency between multiple audio output devices.
///
/// Design:
///  1. A reference signal consisting of per-output chirps is played through
///     ALL outputs simultaneously. Each output's chirp occupies a unique
///     frequency band and time slot so they can be distinguished in the
///     recording.
///  2. The microphone records the summed signal.
///  3. Per-output cross-correlation finds each chirp's arrival time.
///  4. The earliest output gets offset 0; others get positive offsets (ms).
///
/// @note All outputs MUST play the same signal — the engine mix is shared.
///       Per-output frequency separation lets us identify peaks even when
///       multiple outputs play simultaneously.
class AudioLatencyCalibrator : public QObject {
    Q_OBJECT
  public:
    explicit AudioLatencyCalibrator(QObject* parent = nullptr);

    enum State {
        Idle,
        PlayingReference,
        RecordingReference,
        Computing,
    };

    /// Start calibration for the given number of outputs.
    /// @param sampleRate sample rate in Hz
    /// @param bufferSize frames per buffer
    /// @param numOutputs number of active Main outputs to calibrate
    void startCalibration(int sampleRate, int bufferSize, int numOutputs);

    /// Stop calibration and reset.
    void stopCalibration();

    /// Returns current state.
    State state() const {
        return m_state;
    }

    /// Called from the audio callback to fill the output buffer.
    /// Returns the next reference sample to play (same value goes to all outputs).
    /// @param outputIndex which output this frame is for (0 = clock ref)
    CSAMPLE generateReferenceFrame(int outputIndex = 0);

    /// Called on timeout — stops calibration gracefully.
    void onTimeout();

  signals:
    /// Emitted when calibration completes with per-output offsets in ms.
    /// offsets[0] = 0 (earliest device).
    void calibrationComplete(QVector<double> offsetsMs);

    /// Status update for the UI.
    void statusUpdate(const QString& message);

  private:
    // Recording cap: 10 seconds at 48kHz
    static constexpr int kMaxRecordingFrames = 10 * 48000;
    // Maximum search window for cross-correlation (5 seconds)
    static constexpr int kCorrelationWindow = 5 * 48000;

    void generateReferenceChirp();
    void computeOffsets();

    State m_state;

    // Audio configuration
    int m_sampleRate;
    int m_bufferSize;
    int m_numOutputs;

    // Per-output calibration results (ms)
    QVector<double> m_perOutputOffsets;

    // Reference signal: concatenation of per-output chirps + gaps
    QVector<CSAMPLE> m_referenceSignal;
    QVector<CSAMPLE> m_recordedSignal;
    int m_currentOutputIndex;
    qint64 m_referencePlayhead;

    // Timing
    QElapsedTimer m_timer;
    QTimer* m_pTimeoutTimer;
    bool m_referencePlayed;
    qint64 m_recordStartTime;
};