#pragma once

#include <QList>
#include <QObject>
#include <memory>
#include <optional>

#include "stems/stemconverter.h"
#include "track/track.h"

class QThreadPool;

using StemConverterPointer = std::shared_ptr<StemConverter>;

class StemConversionManager : public QObject {
    Q_OBJECT

  public:
    explicit StemConversionManager(QObject* parent = nullptr);
    ~StemConversionManager() override;

    struct ConversionInfo {
        TrackId trackId;
        QString trackTitle;
        float progress;
    };

    struct ConversionStatus {
        TrackId trackId;
        QString trackTitle;
        StemConverter::ConversionState state;
        float progress;
    };

    /// Start converting a track to stems with specified resolution
    void convertTrack(const TrackPointer& pTrack,
            StemConverter::Resolution resolution = StemConverter::Resolution::High);

    /// Get current conversion info
    std::optional<ConversionInfo> getCurrentConversion() const;

    /// Get conversion history
    QList<ConversionStatus> getConversionHistory() const;

    /// Clear conversion history
    void clearConversionHistory();

  signals:
    /// Emitted when a conversion starts
    void conversionStarted(TrackId trackId, const QString& trackTitle);

    /// Emitted when conversion progress updates
    void conversionProgress(TrackId trackId, float progress, const QString& message);

    /// Emitted when a conversion completes successfully
    void conversionCompleted(TrackId trackId, const QString& trackTitle);

    /// Emitted when a conversion fails
    void conversionFailed(TrackId trackId, const QString& trackTitle, const QString& errorMessage);

    /// Emitted when the queue changes
    void queueChanged(int pendingCount);

  private slots:
    void onConversionStarted(TrackId trackId, const QString& trackTitle);
    void onConversionProgress(TrackId trackId, float progress, const QString& message);
    void onConversionCompleted(TrackId trackId);
    void onConversionFailed(TrackId trackId, const QString& errorMessage);

  private:
    void processNextInQueue();
    void loadHistory();
    void saveHistory();

    QThreadPool* m_pThreadPool;
    TrackId m_currentTrackId;
    QList<TrackPointer> m_conversionQueue;
    QList<StemConverter::Resolution> m_resolutionQueue; // Parallel queue for resolutions
    QList<ConversionStatus> m_conversionHistory;
    StemConverterPointer m_pCurrentConverter;
    QString m_historyFilePath;
};

using StemConversionManagerPointer = std::shared_ptr<StemConversionManager>;
