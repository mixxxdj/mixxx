#include "library/overviewcache.h"

#include <QFutureWatcher>
#include <QPixmapCache>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtConcurrentRun>
#include <algorithm>

#include "library/dao/analysisdao.h"
#include "moc_overviewcache.cpp"
#include "util/db/dbconnectionpooled.h"
#include "util/db/dbconnectionpooler.h"
#include "util/logger.h"
#include "waveform/renderers/waveformoverviewrenderer.h"
#include "waveform/renderers/waveformsignalcolors.h"
#include "waveform/waveformfactory.h"

namespace {

mixxx::Logger kLogger("OverviewCache");

// The cache key includes the uniform time base mode flag so that toggling
// the preference invalidates stale pixmaps.
QString pixmapCacheKeyUniform(TrackId trackId,
        QSize size,
        mixxx::OverviewType type,
        bool uniformTimeBase) {
    return QString("Overview_%1_%2_%3_%4_%5")
            .arg(QString::number(static_cast<int>(type)),
                    trackId.toString(),
                    QString::number(size.width()),
                    QString::number(size.height()),
                    uniformTimeBase ? QLatin1String("u") : QLatin1String("s"));
}

// The transformation mode when scaling images
const Qt::TransformationMode kTransformationMode = Qt::SmoothTransformation;

inline QImage resizeImageSize(const QImage& image, QSize size) {
    return image.scaled(size, Qt::IgnoreAspectRatio, kTransformationMode);
}

// Redraw minute markers at exact pixel positions on a uniform-time-base
// overview image. The renderer draws markers proportionally on a fixed-width
// image; scaling introduces rounding that shifts markers by 1-2px between
// tracks. This redraws them at markerSeconds * pixelsPerSecond so they
// align across all tracks regardless of duration.
void redrawMinuteMarkersUniform(QImage* image, double durationSeconds, double pixelsPerSecond) {
    if (image->format() != QImage::Format_ARGB32_Premultiplied) {
        *image = image->convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }
    QPainter painter(image);
    painter.setRenderHint(QPainter::Antialiasing, false);
    const int markerHeight = static_cast<int>(image->height() * 0.2);
    const int lowerMarkerYPos = static_cast<int>(image->height() * 0.8);
    const QColor minuteColor(255, 255, 255, 255);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    for (double markerSeconds = 60.0;
            markerSeconds < durationSeconds;
            markerSeconds += 60.0) {
        const int x = static_cast<int>(markerSeconds * pixelsPerSecond);
        if (x >= 0 && x < image->width()) {
            painter.fillRect(x, 0, 2, markerHeight, minuteColor);
            painter.fillRect(x, lowerMarkerYPos, 2, image->height() - lowerMarkerYPos, minuteColor);
        }
    }
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
}
} // anonymous namespace

OverviewCache::OverviewCache(UserSettingsPointer pConfig,
        mixxx::DbConnectionPoolPtr pDbConnectionPool)
        : m_pConfig(pConfig),
          m_pDbConnectionPool(std::move(pDbConnectionPool)) {
}

void OverviewCache::onTrackAnalysisProgress(TrackId trackId, AnalyzerProgress analyzerProgress) {
    if (analyzerProgress < 1.0) {
        return;
    }
    m_tracksWithoutOverview.remove(trackId);
    // request update independent from paint events
    emit overviewChanged(trackId);
}

void OverviewCache::onTrackSummaryChanged(TrackId trackId) {
    // kLogger.warning() << "onTrackSummaryChanged" << trackId;
    // The waveform has been removed, created or changed.
    // Find all cache keys for this id and remove the entries from the pixmap cache
    while (m_cacheKeysByTrackId.contains(trackId)) {
        const auto cacheKey = m_cacheKeysByTrackId.take(trackId);
        DEBUG_ASSERT(!cacheKey.isEmpty());
        QPixmapCache::remove(cacheKey);
    }
    // try remove the id from the ignore list
    m_tracksWithoutOverview.remove(trackId);
    // then let users request an update independent from paint events
    emit overviewChanged(trackId);
}

void OverviewCache::invalidateAll() {
    // Clear all cached overview pixmaps so they are re-rendered with the
    // current uniform time base settings.
    QSet<TrackId> affectedIds;
    for (auto it = m_cacheKeysByTrackId.constBegin();
            it != m_cacheKeysByTrackId.constEnd();
            ++it) {
        QPixmapCache::remove(it.value());
        affectedIds.insert(it.key());
    }
    m_cacheKeysByTrackId.clear();
    m_tracksWithoutOverview.clear();
    // Notify delegates for each affected track
    for (const TrackId& trackId : std::as_const(affectedIds)) {
        emit overviewChanged(trackId);
    }
}

QPixmap OverviewCache::requestCachedOverview(
        mixxx::OverviewType type,
        TrackId trackId,
        const QObject* pRequester,
        QSize desiredSize) {
    Q_UNUSED(pRequester);
    if (!trackId.isValid()) {
        return QPixmap();
    }

    if (m_currentlyLoading.contains(trackId)) {
        return QPixmap();
    }

    if (m_tracksWithoutOverview.contains(trackId)) {
        return QPixmap();
    }

    // kLogger.info() << "requestCachedOverview()" << trackId << pRequester << desiredSize;

    const bool uniform = m_pConfig->getValue(
            ConfigKey("[Waveform]", QStringLiteral("overview_uniform_time_base")), false);
    const QString cacheKey = pixmapCacheKeyUniform(trackId, desiredSize, type, uniform);
    QPixmap pixmap;
    QPixmapCache::find(cacheKey, &pixmap);
    return pixmap;
}

QPixmap OverviewCache::requestUncachedOverview(
        mixxx::OverviewType type,
        const WaveformSignalColors& signalColors,
        TrackId trackId,
        const QObject* pRequester,
        QSize desiredSize) {
    if (!trackId.isValid()) {
        return QPixmap();
    }

    if (m_currentlyLoading.contains(trackId)) {
        return QPixmap();
    }

    if (m_tracksWithoutOverview.contains(trackId)) {
        return QPixmap();
    }

    // kLogger.info() << "requestUncachedOverview()" << trackId << pRequester << desiredSize;

    const bool uniform = m_pConfig->getValue(
            ConfigKey("[Waveform]", QStringLiteral("overview_uniform_time_base")), false);
    const QString cacheKey = pixmapCacheKeyUniform(trackId, desiredSize, type, uniform);
    QPixmap pixmap;
    // Maybe it has been cached since the request for cached image?
    if (QPixmapCache::find(cacheKey, &pixmap)) {
        return pixmap;
    }

    // no cached overview, request preparation
    m_currentlyLoading.insert(trackId);

    QFutureWatcher<FutureResult>* watcher = new QFutureWatcher<FutureResult>(this);
    QFuture<FutureResult> future = QtConcurrent::run(
            &OverviewCache::prepareOverview,
            m_pConfig,
            m_pDbConnectionPool,
            type,
            signalColors,
            trackId,
            pRequester,
            desiredSize);
    connect(watcher,
            &QFutureWatcher<FutureResult>::finished,
            this,
            &OverviewCache::overviewPrepared);
    watcher->setFuture(future);

    return QPixmap();
}

// static
OverviewCache::FutureResult OverviewCache::prepareOverview(
        const UserSettingsPointer pConfig,
        const mixxx::DbConnectionPoolPtr pDbConnectionPool,
        mixxx::OverviewType type,
        const WaveformSignalColors& signalColors,
        TrackId trackId,
        const QObject* pRequester,
        QSize desiredSize) {
    // kLogger.warning() << "prepareOverview" << trackId;
    FutureResult result;
    result.trackId = trackId;
    result.type = type;
    result.requester = pRequester;
    result.image = QImage();
    result.requestedSize = desiredSize;

    if (!trackId.isValid() || desiredSize.isEmpty()) {
        return result;
    }

    const bool uniformTimeBase = pConfig->getValue(
            ConfigKey("[Waveform]", QStringLiteral("overview_uniform_time_base")), false);
    result.uniformTimeBase = uniformTimeBase;

    mixxx::DbConnectionPooler dbConnectionPooler(pDbConnectionPool);

    AnalysisDao analysisDao(pConfig);
    analysisDao.initialize(mixxx::DbConnectionPooled(pDbConnectionPool));

    QList<AnalysisDao::AnalysisInfo> analyses =
            analysisDao.getAnalysesForTrackByType(
                    trackId, AnalysisDao::AnalysisType::TYPE_WAVESUMMARY);

    if (!analyses.isEmpty()) {
        ConstWaveformPointer pLoadedTrackWaveformSummary = ConstWaveformPointer(
                WaveformFactory::loadWaveformFromAnalysis(analyses.first()));

        if (!pLoadedTrackWaveformSummary.isNull()) {
            QImage image = waveformOverviewRenderer::render(
                    pLoadedTrackWaveformSummary,
                    type,
                    signalColors,
                    true /* mono, bottom-aligned */);

            if (!image.isNull()) {
                if (uniformTimeBase) {
                    // Render the waveform at a width proportional to the
                    // track duration so that all overviews share the same
                    // pixels-per-second ratio. Short tracks occupy less
                    // than the full column; long tracks are clipped.
                    const double timeBaseMinutes = pConfig->getValue(
                            ConfigKey("[Waveform]",
                                    QStringLiteral("overview_time_base_minutes")),
                            6.0);
                    const double pixelsPerSecond =
                            static_cast<double>(desiredSize.width()) /
                            (timeBaseMinutes * 60.0);

                    // Query track duration directly from the library table.
                    double durationSeconds = 0.0;
                    {
                        QSqlDatabase dbConnection =
                                mixxx::DbConnectionPooled(pDbConnectionPool);
                        QSqlQuery query(dbConnection);
                        query.prepare(QStringLiteral(
                                "SELECT duration FROM library WHERE id = :id"));
                        query.bindValue(QStringLiteral(":id"), trackId.toVariant());
                        if (query.exec() && query.next()) {
                            durationSeconds = query.value(0).toDouble();
                        }
                    }

                    if (durationSeconds > 0.0) {
                        int uniformWidth = static_cast<int>(
                                durationSeconds * pixelsPerSecond);
                        uniformWidth = std::clamp(uniformWidth, 1, desiredSize.width());
                        QSize uniformSize(uniformWidth, desiredSize.height());
                        image = resizeImageSize(image, uniformSize);
                        redrawMinuteMarkersUniform(&image,
                                durationSeconds,
                                pixelsPerSecond);
                    } else {
                        image = resizeImageSize(image, desiredSize);
                    }
                } else {
                    image = resizeImageSize(image, desiredSize);
                }
            }
            result.image = image;
        }
    }

    return result;
}

// watcher
void OverviewCache::overviewPrepared() {
    QFutureWatcher<FutureResult>* watcher = static_cast<QFutureWatcher<FutureResult>*>(sender());
    FutureResult res = watcher->result();
    watcher->deleteLater();
    // kLogger.warning() << "overviewPrepared" << res.trackId;

    // Create pixmap, GUI thread only
    QPixmap pixmap = QPixmap::fromImage(res.image);
    if (!pixmap.isNull() && !res.requestedSize.isEmpty()) {
        // we have to be sure that cacheKey is unique
        // because insert replaces the images with the same key
        // Use requestedSize (the cell size) so the key matches the lookup
        // in requestCachedOverview/requestUncachedOverview. The actual image
        // may be narrower in uniform mode, but the delegate handles that.
        const QString cacheKey = pixmapCacheKeyUniform(
                res.trackId, res.requestedSize, res.type, res.uniformTimeBase);
        QPixmapCache::insert(cacheKey, pixmap);
        // Store the cached track id so we can clear ALL pixmaps of a track
        // in case the waveform has been cleared/updated.
        // This is a QMultiHash because we want to store pixmap keys of all
        // OverviewDelegates with different widths in various library features.
        m_cacheKeysByTrackId.insert(res.trackId, cacheKey);
    }

    if (pixmap.isNull()) {
        // Avoid (too many) repeated lookups.
        // (there may still be identical request be processed due to
        // asynchronous processing)
        // kLogger.warning() << "--> empty pixmap, add to ignore list";
        m_tracksWithoutOverview.insert(res.trackId);
    }
    m_currentlyLoading.remove(res.trackId);

    emit overviewReady(res.requester, res.trackId, !pixmap.isNull());
}
