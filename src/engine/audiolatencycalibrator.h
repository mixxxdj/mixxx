#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QVector>

#include "engine/engine.h"

/**
 * AudioLatencyCalibrator implements active latency calibration using a
 * play-and-record loopback approach inspired by the osu! framework's
 * BeatmapOffsetControl.
 *
 * How it works:
 * 1. A short reference click/pulse is played through the output device.
 * 2. The same pulse is recorded via a loopback or microphone input.
 * 3. Cross-correlation between played and recorded signals determines
 *    the actual hardware latency (the offset at which correlation peaks).
 * 4. The suggested latency offset is displayed to the user for manual fine-tuning.
 *
 * Manual mode: the user adjusts a spinbox (0-500ms) while listening to
 * a continuous sync click on both devices, tuning until clicks align.
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

    /// Start the calibration process. Plays a reference tone and records it back.
    void startCalibration(int sampleRate, int bufferSize);

    /// Stop calibration and reset to idle.
    void stopCalibration();

    /// Get the suggested offset in milliseconds based on last calibration.
    double getSuggestedOffsetMs() const {
        return m_suggestedOffsetMs;
    }

    /// Get the manual offset from the UI spinbox.
    double getManualOffsetMs() const {
        return m_manualOffsetMs;
    }

    /// Set the manual offset (from UI spinbox).
    void setManualOffsetMs(double offset) {
        m_manualOffsetMs = offset;
    }

    /// Add a recorded sample frame from the input device.
    void addRecordedFrame(CSAMPLE value);

    /// Generate the next reference frame to play. Returns 0 when not playing.
    CSAMPLE generateReferenceFrame();

  signals:
    /// Emitted when calibration completes with the suggested offset in ms.
    void calibrationComplete(double suggestedOffsetMs);

    /// Emitted periodically during calibration with current status.
    void statusUpdate(const QString& status);

  private:
    void computeOffset();
    void generateReferencePulse();

    State m_state;
    double m_suggestedOffsetMs;
    double m_manualOffsetMs;

    int m_sampleRate;
    int m_bufferSize;

    // Reference signal storage
    QVector<CSAMPLE> m_referenceSignal;
    int m_referencePlayhead;

    // Recorded signal storage
    QVector<CSAMPLE> m_recordedSignal;
    qint64 m_recordStartTime;

    // Recording/playback timing
    QElapsedTimer m_timer;
    bool m_referencePlayed;

    static constexpr int kReferencePulseLength = 480; // ~10ms at 48kHz
    static constexpr int kMaxRecordingFrames = 48000; // 1 second at 48kHz
    static constexpr int kCorrelationWindow = 4800;   // Search window for correlation (100ms)
};
