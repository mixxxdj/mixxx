#pragma once

#include <QObject>
#include <memory>
#include <optional>
#include <QProcess>

#include "track/track.h"

class StemConverter : public QObject {
    Q_OBJECT

  public:
    enum class ConversionState {
        Idle,
        Processing,
        Completed,
        Failed
    };

    explicit StemConverter(QObject* parent = nullptr);
    ~StemConverter() override = default;

    /// Convert a track to stems
    bool convertTrack(const TrackPointer& pTrack);

    /// Stop the current conversion
    void stopConversion();

    /// Get current state
    ConversionState getState() const { return m_state; }

    /// Get current progress (0.0 to 1.0)
    float getProgress() const { return m_progress; }

    /// Get status message
    QString getStatusMessage() const { return m_statusMessage; }

    /// Get track title
    QString getTrackTitle() const { return m_trackTitle; }

  signals:
    /// Emitted when conversion starts
    void conversionStarted(TrackId trackId, const QString& trackTitle);

    /// Emitted when conversion progress updates
    void conversionProgress(TrackId trackId, float progress, const QString& message);

    /// Emitted when conversion completes
    void conversionCompleted(TrackId trackId);

    /// Emitted when conversion fails
    void conversionFailed(TrackId trackId, const QString& errorMessage);

  private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onProcessOutput();

  private:
    QString findStemgenPath();
    void setState(ConversionState state);
    void setProgress(float progress, const QString& message);

    ConversionState m_state;
    float m_progress;
    QString m_statusMessage;
    QString m_trackTitle;
    TrackId m_currentTrackId;
    std::unique_ptr<QProcess> m_pProcess;
};

using StemConverterPointer = std::shared_ptr<StemConverter>;
