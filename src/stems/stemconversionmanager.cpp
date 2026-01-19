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
    StemConversionTask(StemConversionManager* pManager, StemConverterPointer pConverter, TrackPointer pTrack)
            : m_pManager(pManager),
              m_pConverter(pConverter),
              m_pTrack(pTrack) {
    }

    void run() override {
        if (!m_pManager || !m_pConverter || !m_pTrack) {
            return;
        }

        // Executar la conversió en el thread pool
        m_pConverter->convertTrack(m_pTrack);
    }

  private:
    StemConversionManager* m_pManager;
    StemConverterPointer m_pConverter;
    TrackPointer m_pTrack;
};

StemConversionManager::StemConversionManager(QObject* parent)
        : QObject(parent),
          m_currentTrackId(TrackId()),
          m_conversionQueue(QList<TrackPointer>()) {
    // Crear el thread pool per a conversions asíncrones
    m_pThreadPool = new QThreadPool(this);
    m_pThreadPool->setMaxThreadCount(1);  // Una conversió a la vegada

    kLogger.info() << "StemConversionManager initialized";
}

void StemConversionManager::convertTrack(const TrackPointer& pTrack) {
    if (!pTrack) {
        kLogger.warning() << "Cannot convert null track";
        return;
    }

    // Afegir a la cua
    m_conversionQueue.append(pTrack);
    emit queueChanged(m_conversionQueue.size());

    // Si no hi ha cap conversió en curs, iniciar la següent
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

    // Agafar la primera pista de la cua
    TrackPointer pTrack = m_conversionQueue.takeFirst();
    m_currentTrackId = pTrack->getId();

    // Crear el conversor
    StemConverterPointer pConverter = std::make_shared<StemConverter>();

    // Connectar els senyals del conversor
    connect(pConverter.get(), &StemConverter::conversionStarted,
            this, &StemConversionManager::onConversionStarted);
    connect(pConverter.get(), &StemConverter::conversionProgress,
            this, &StemConversionManager::onConversionProgress);
    connect(pConverter.get(), &StemConverter::conversionCompleted,
            this, &StemConversionManager::onConversionCompleted);
    connect(pConverter.get(), &StemConverter::conversionFailed,
            this, &StemConversionManager::onConversionFailed);

    // Emetre que ha començat
    emit conversionStarted(pTrack->getId(), pTrack->getTitle());

    // Executar la conversió en un thread separat (asíncrona)
    StemConversionTask* pTask = new StemConversionTask(this, pConverter, pTrack);
    m_pThreadPool->start(pTask);

    // Guardar el conversor per poder accedir als senyals
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

    // Afegir a l'historial
    ConversionStatus status;
    status.trackId = trackId;
    status.trackTitle = m_pCurrentConverter ? m_pCurrentConverter->getTrackTitle() : "Unknown";
    status.state = StemConverter::ConversionState::Completed;
    status.progress = 1.0f;
    m_conversionHistory.prepend(status);

    // Limitar l'historial a 100 elements
    if (m_conversionHistory.size() > 100) {
        m_conversionHistory.removeLast();
    }

    emit conversionCompleted(trackId);

    // Processar la següent de la cua
    processNextInQueue();
}

void StemConversionManager::onConversionFailed(TrackId trackId, const QString& errorMessage) {
    kLogger.warning() << "Conversion failed for track:" << trackId << "Error:" << errorMessage;

    // Afegir a l'historial
    ConversionStatus status;
    status.trackId = trackId;
    status.trackTitle = m_pCurrentConverter ? m_pCurrentConverter->getTrackTitle() : "Unknown";
    status.state = StemConverter::ConversionState::Failed;
    status.progress = 0.0f;
    m_conversionHistory.prepend(status);

    // Limitar l'historial a 100 elements
    if (m_conversionHistory.size() > 100) {
        m_conversionHistory.removeLast();
    }

    emit conversionFailed(trackId, errorMessage);

    // Processar la següent de la cua
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
