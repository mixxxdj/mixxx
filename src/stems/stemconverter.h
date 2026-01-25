#pragma once

#include <QObject>
#include <QString>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#ifdef __STEM_CONVERSION__
  #include <onnxruntime_cxx_api.h>
  #include <sndfile.h>
#endif

#include "track/track.h"

/// Converts audio tracks to STEM format using ONNX Runtime and mp4box
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
#ifdef __STEM__
    /// to save number of track original frames
    int m_originalFrames = 0;

    /// Step 1: Load ONNX model from ~/.local/mixxx_models/htdemucs.onnx
    bool loadOnnxModel();

    /// Step 2: Run ONNX inference on audio file
    bool runInference(const std::vector<float>& inputAudio, int sampleRate,
                      std::vector<std::vector<float>>& outputStems);

    /// Step 3: Decode audio file to WAV using ffmpeg
    bool decodeAudioFile(const QString& inputPath, std::vector<float>& audioData,
                      int& sampleRate, int& channels, int& originalFrames);

    /// Step 4: Save stem to WAV file using libsndfile
    bool saveStemToWav(const std::vector<float>& audioData, const QString& outputPath,
                      int sampleRate, int channels, int originalFrames);

    /// Main ONNX separation function
    bool runOnnxSeparation(const QString& trackFilePath, const QString& outputDir);
#endif

    /// Get the directory where stems are created
    QString getStemsDirectory(const QString& trackFilePath);

    /// Convert separated stems (WAV) to M4A format
    bool convertStemsToM4A(const QString& stemsDir);

    /// Create STEM container using ffmpeg and mp4box
    bool createStemContainer(const QString& trackFilePath, const QString& stemsDir);

    /// Create STEM manifest JSON
    QString createStemManifest();

    /// Find MP4Box executable in system
    QString findMP4Box();

    /// Add STEM metadata atom using MP4Box
    bool addStemMetadata(const QString& outputPath);

    /// Convert a single track to M4A format
    bool convertTrackToM4A(const QString& inputPath, const QString& outputPath);

    /// Step 5: Add metadata tags to the STEM file
    bool addMetadataTags(const QString& trackFilePath, const QString& stemsDir);

#ifdef __STEM__
    // ONNX Runtime members
    std::unique_ptr<Ort::Env> m_pOrtEnv;
    std::unique_ptr<Ort::Session> m_pOrtSession;
    Ort::AllocatorWithDefaultOptions m_allocator;
#endif

    TrackPointer m_pTrack;
    Resolution m_resolution;
    ConversionState m_state;
    float m_progress;
    QString m_trackTitle;
};

using StemConverterPointer = std::shared_ptr<StemConverter>;
