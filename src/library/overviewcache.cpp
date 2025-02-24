#include "library/overviewcache.h"

#include <QFutureWatcher>
#include <QPixmapCache>
#include <QSqlDatabase>
#include <QtConcurrentRun>

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

QString pixmapCacheKey(TrackId trackId, QSize size, mixxx::OverviewType type) {
    return QString("Overview_%1_%2_%3_%4")
            .arg(QString::number(static_cast<int>(type)),
                    trackId.toString(),
                    QString::number(size.width()),
                    QString::number(size.height()));
}

// The transformation mode when scaling images
const Qt::TransformationMode kTransformationMode = Qt::SmoothTransformation;

inline QImage resizeImageSize(const QImage& image, QSize size) {
    return image.scaled(size, Qt::IgnoreAspectRatio, kTransformationMode);
}
} // anonymous namespace

OverviewCache::OverviewCache(UserSettingsPointer pConfig,
        mixxx::DbConnectionPoolPtr pDbConnectionPool)
        : m_pConfig(pConfig),
          m_pDbConnectionPool(std::move(pDbConnectionPool)),
          m_clearingCache(false),
          m_stopClearing(false) {
}

void OverviewCache::onNormalizeOrVisualGainChanged() {
    // Clear the cache and emit changed signal so OverviewDelegate requests
    // new pixmaps.
    // Prevent interferences of repeated calls when Normalize or VisualGainAll
    // are changed in quick succession.
    if (m_clearingCache) {
        m_stopClearing = true;
    }
    m_clearingCache = true;

    QMultiHashIterator<TrackId, QString> it(m_cacheKeysByTrackId);
    QSet<TrackId> ids;
    while (it.hasNext()) {
        if (m_stopClearing) {
            m_stopClearing = false;
            return;
        }
        it.next();
        TrackId id = it.key();
        ids.insert(id);
        const auto cacheKey = it.value();
        QPixmapCache::remove(cacheKey);
    }

    m_cacheKeysByTrackId.clear();
    m_clearingCache = false;

    for (const auto trackId : ids) {
        emit overviewChanged(trackId);
    }
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

    const QString cacheKey = pixmapCacheKey(trackId, desiredSize, type);
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

    const QString cacheKey = pixmapCacheKey(trackId, desiredSize, type);
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
    result.resizedToSize = desiredSize;

    if (!trackId.isValid() || desiredSize.isEmpty()) {
        return result;
    }

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
                image = resizeImageSize(image, desiredSize);
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
    if (!pixmap.isNull() && !res.resizedToSize.isEmpty()) {
        // we have to be sure that cacheKey is unique
        // because insert replaces the images with the same key
        const QString cacheKey = pixmapCacheKey(
                res.trackId, res.resizedToSize, res.type);
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
