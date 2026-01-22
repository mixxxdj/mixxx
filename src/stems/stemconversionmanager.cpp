#include "stems/stemconversionmanager.h"

#include <QThreadPool>
#include <QRunnable>
#include <QDebug>

#include "util/logger.h"

namespace {
const mixxx::Logger kLogger("StemConversionManager");
}

class StemConversionTask : public QRunnable {
  public:
    StemConversionTask(StemConversionManager* pManager, 
                      StemConverterPointer pConverter, 
                      TrackPointer pTrack,
                      StemConverter::Resolution resolution)
            : m_pManager(pManager),
              m_pConverter(pConverter),
              m_pTrack(pTrack),
              m_resolution(resolution) {
    }

    void run() override {
        if (!m_pManager || !m_pConverter || !m_pTrack) {
            return;
        }

        // Execute the conversion in the thread pool with the specified resolution
        m_pConverter->convertTrack(m_pTrack, m_resolution);
    }

  private:
    StemConversionManager* m_pManager;
    StemConverterPointer m_pConverter;
    TrackPointer m_pTrack;
    StemConverter::Resolution m_resolution;
};

StemConversionManager::StemConversionManager(QObject* parent)
        : QObject(parent),
          m_currentTrackId(TrackId()),
          m_conversionQueue(QList<TrackPointer>()) {
    // Create thread pool for asynchronous conversions
    m_pThreadPool = new QThreadPool(this);
    m_pThreadPool->setMaxThreadCount(1);  // One conversion at a time

    kLogger.info() << "StemConversionManager initialized";
}

void StemConversionManager::convertTrack(const TrackPointer& pTrack, 
                                         StemConverter::Resolution resolution) {
    if (!pTrack) {
        kLogger.warning() << "Cannot convert null track";
        return;
    }

    // Add to queue
    m_conversionQueue.append(pTrack);
    m_resolutionQueue.append(resolution);
    emit queueChanged(m_conversionQueue.size());

    // If no conversion is in progress, start the next one
    if (m_currentTrackId.isValid() == false) {
        processNextInQueue();
    }
}

void StemConversionManager::processNextInQueue() {
    if (m_conversionQueue.isEmpty()) {
        m_currentTrackId = TrackId();
        emit queueChanged(0);
        return;
    }

    // Get the first track from the queue
    TrackPointer pTrack = m_conversionQueue.takeFirst();
    StemConverter::Resolution resolution = m_resolutionQueue.takeFirst();
    m_currentTrackId = pTrack->getId();

    // Create the converter
    StemConverterPointer pConverter = std::make_shared<StemConverter>();

    // Connect the converter signals
    connect(pConverter.get(), &StemConverter::conversionStarted,
            this, &StemConversionManager::onConversionStarted);
    connect(pConverter.get(), &StemConverter::conversionProgress,
            this, &StemConversionManager::onConversionProgress);
    connect(pConverter.get(), &StemConverter::conversionCompleted,
            this, &StemConversionManager::onConversionCompleted);
    connect(pConverter.get(), &StemConverter::conversionFailed,
            this, &StemConversionManager::onConversionFailed);

    // Emit that conversion has started
    emit conversionStarted(pTrack->getId(), pTrack->getTitle());

    // Execute the conversion in a separate thread (asynchronous)
    StemConversionTask* pTask = new StemConversionTask(this, pConverter, pTrack, resolution);
    m_pThreadPool->start(pTask);

    // Store the converter to access signals
    m_pCurrentConverter = pConverter;

    emit queueChanged(m_conversionQueue.size());
}

void StemConversionManager::onConversionStarted(TrackId trackId, const QString& trackTitle) {
    Q_UNUSED(trackTitle);
    m_currentTrackId = trackId;
    emit conversionStarted(trackId, trackTitle);
}

void StemConversionManager::onConversionProgress(TrackId trackId, float progress, const QString& message) {
    emit conversionProgress(trackId, progress, message);
}

void StemConversionManager::onConversionCompleted(TrackId trackId) {
    kLogger.info() << "Conversion completed for track:" << trackId;

    // Add to history
    ConversionStatus status;
    status.trackId = trackId;
    status.trackTitle = m_pCurrentConverter ? m_pCurrentConverter->getTrackTitle() : "Unknown";
    status.state = StemConverter::ConversionState::Completed;
    status.progress = 1.0f;
    m_conversionHistory.prepend(status);

    // Limit history to 100 items
    if (m_conversionHistory.size() > 100) {
        m_conversionHistory.removeLast();
    }

    emit conversionCompleted(trackId);

    // Process next in queue
    processNextInQueue();
}

void StemConversionManager::onConversionFailed(TrackId trackId, const QString& errorMessage) {
    kLogger.warning() << "Conversion failed for track:" << trackId << "Error:" << errorMessage;

    // Add to history
    ConversionStatus status;
    status.trackId = trackId;
    status.trackTitle = m_pCurrentConverter ? m_pCurrentConverter->getTrackTitle() : "Unknown";
    status.state = StemConverter::ConversionState::Failed;
    status.progress = 0.0f;
    m_conversionHistory.prepend(status);

    // Limit history to 100 items
    if (m_conversionHistory.size() > 100) {
        m_conversionHistory.removeLast();
    }

    emit conversionFailed(trackId, errorMessage);

    // Process next in queue
    processNextInQueue();
}

std::optional<StemConversionManager::ConversionInfo> StemConversionManager::getCurrentConversion() const {
    if (!m_currentTrackId.isValid()) {
        return std::nullopt;
    }

    if (!m_pCurrentConverter) {
        return std::nullopt;
    }

    ConversionInfo info;
    info.trackId = m_currentTrackId;
    info.trackTitle = m_pCurrentConverter->getTrackTitle();
    info.progress = m_pCurrentConverter->getProgress();

    return info;
}

QList<StemConversionManager::ConversionStatus> StemConversionManager::getConversionHistory() const {
    return m_conversionHistory;
}

void StemConversionManager::clearConversionHistory() {
    m_conversionHistory.clear();
}

#include "moc_stemconversionmanager.cpp"
