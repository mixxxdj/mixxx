#pragma once

#include <QObject>
#include <QString>
#include <memory>

#include "track/track.h"

/// Converts audio tracks to STEM format using demucs and mp4box
class StemConverter : public QObject {
    Q_OBJECT

  public:
    enum class Resolution {
        High,   // 44.1 kHz (htdemucs_ft)
        Low,    // 16 kHz (htdemucs)
    };

    enum class ConversionState {
        Idle,
        Processing,
        Completed,
        Failed,
    };

    explicit StemConverter(QObject* parent = nullptr);
    ~StemConverter() override = default;

    /// Convert a track to STEM format
    /// @param pTrack Track to convert
    /// @param resolution Output resolution (High or Low)
    void convertTrack(const TrackPointer& pTrack, Resolution resolution = Resolution::High);

    /// Get current conversion progress (0.0 - 1.0)
    float getProgress() const;

    /// Get the title of the track being converted
    QString getTrackTitle() const;

    /// Get the current conversion state
    ConversionState getState() const;

  signals:
    /// Emitted when conversion starts
    void conversionStarted(TrackId trackId, const QString& trackTitle);

    /// Emitted when conversion progress updates
    void conversionProgress(TrackId trackId, float progress, const QString& message);

    /// Emitted when conversion completes successfully
    void conversionCompleted(TrackId trackId);

    /// Emitted when conversion fails
    void conversionFailed(TrackId trackId, const QString& errorMessage);

  private:
    /// Step 1: Run demucs to separate stems
    bool runDemucs(const QString& trackFilePath);

    /// Get the directory where demucs creates stems
    QString getStemsDirectory(const QString& trackFilePath);

    /// Step 2: Convert separated stems (MP3) to M4A format
    bool convertStemsToM4A(const QString& stemsDir);

    /// Step 3: Create STEM container using mp4box
    bool createStemContainer(const QString& trackFilePath, const QString& stemsDir);

    /// Create metadata JSON for STEM container
    QString createMetadataJson();

    /// Create inline Python script for STEM container creation
    QString createPythonScript();

    /// Convert a single track to M4A format
    bool convertTrackToM4A(const QString& inputPath, const QString& outputPath);

    /// Step 4: Add metadata tags to the STEM file
    bool addMetadataTags(const QString& trackFilePath, const QString& stemsDir);

    TrackPointer m_pTrack;
    Resolution m_resolution;
    ConversionState m_state;
    float m_progress;
    QString m_trackTitle;
};

using StemConverterPointer = std::shared_ptr<StemConverter>;
