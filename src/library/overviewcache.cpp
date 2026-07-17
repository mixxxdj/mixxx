#include "library/overviewcache.h"

#include <QFutureWatcher>
#include <QPixmapCache>
#include <QPointer>
#include <QSqlDatabase>
#include <QtConcurrentRun>

#include "library/dao/analysisdao.h"
#include "library/dao/trackdao.h"
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
          m_pDbConnectionPool(std::move(pDbConnectionPool)) {
}

void OverviewCache::onTrackAnalysisProgress(TrackId trackId, AnalyzerProgress progress) {
    // Always forward the raw progress so listeners that need to draw
    // the partial waveform during analysis (e.g. QmlWaveformOverview)
    // can repaint on every tick.
    emit analyzerProgress(trackId, progress);
    if (progress < 1.0) {
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

void OverviewCache::requestWaveformSummary(TrackId trackId, const QObject* pRequester) {
    if (!trackId.isValid()) {
        return;
    }
    // Avoid duplicate concurrent loads for the same track. If a load is
    // already pending, the requester will be notified through the
    // existing in-flight job (i.e. it will simply receive an
    // `overviewChanged` since the (re)analysis will update the track).
    if (m_currentlyLoadingWaveform.contains(trackId)) {
        return;
    }

    m_currentlyLoadingWaveform.insert(trackId);

    // The async load carries `pRequester` verbatim and will eventually
    // dispatch the result back to it. If `pRequester` is destroyed while
    // the load is still in flight, its `slotWaveformSummaryReady` is
    // auto-disconnected by Qt, no one adopts the loaded waveform, and
    // `m_currentlyLoadingWaveform` would only be cleared when the
    // background job finishes — meanwhile other requesters interested in
    // the same track are silently blocked. Drop the pending entry as
    // soon as the requester goes away so a new load can be started by
    // any other listener still alive.
    if (pRequester) {
        QObject::connect(pRequester, &QObject::destroyed, this, [this, trackId] {
            m_currentlyLoadingWaveform.remove(trackId);
        });
    }

    QFutureWatcher<FutureWaveformResult>* watcher =
            new QFutureWatcher<FutureWaveformResult>(this);
    QFuture<FutureWaveformResult> future = QtConcurrent::run(
            &OverviewCache::prepareWaveformSummary,
            m_pConfig,
            m_pDbConnectionPool,
            trackId,
            pRequester);
    connect(watcher,
            &QFutureWatcher<FutureWaveformResult>::finished,
            this,
            &OverviewCache::waveformSummaryPrepared);
    watcher->setFuture(future);
}

void OverviewCache::requestWaveformSummary(
        const QString& trackLocation, const QObject* pRequester) {
    if (trackLocation.isEmpty()) {
        return;
    }
    if (!m_pTrackDAO) {
        // Without a TrackDAO we cannot translate a file location into
        // a TrackId. The TrackId-based overload is the only available
        // path in that case.
        return;
    }

    // Resolve the location into a TrackId on the GUI thread. The
    // query is a single indexed lookup on `track_locations.location`,
    // which is essentially instantaneous, so doing it on the GUI
    // thread is preferable to introducing an extra round-trip through
    // the worker pool and lets us reuse the existing TrackId-based
    // dedup logic and async machinery. `TrackDAO` is owned by
    // `TrackCollection` and lives on the GUI thread, so its
    // pre-initialised `QSqlDatabase` connection is valid here.
    const TrackId trackId = m_pTrackDAO->getTrackIdByLocation(trackLocation);
    if (!trackId.isValid()) {
        return;
    }

    // Delegate to the existing TrackId-based load path.
    requestWaveformSummary(trackId, pRequester);
}

// static
OverviewCache::FutureWaveformResult OverviewCache::prepareWaveformSummary(
        const UserSettingsPointer pConfig,
        const mixxx::DbConnectionPoolPtr pDbConnectionPool,
        TrackId trackId,
        const QObject* pRequester) {
    FutureWaveformResult result;
    result.trackId = trackId;
    result.requester = pRequester;

    if (!trackId.isValid()) {
        return result;
    }

    mixxx::DbConnectionPooler dbConnectionPooler(pDbConnectionPool);

    AnalysisDao analysisDao(pConfig);
    analysisDao.initialize(mixxx::DbConnectionPooled(pDbConnectionPool));

    QList<AnalysisDao::AnalysisInfo> analyses =
            analysisDao.getAnalysesForTrackByType(
                    trackId, AnalysisDao::AnalysisType::TYPE_WAVESUMMARY);

    // Only load analyses whose version is compatible with the current
    // format, mirroring the logic in `AnalyzerWaveform::shouldAnalyze`.
    // Loading an incompatible (e.g. `VC_REMOVE`) waveform would yield a
    // `Waveform` with invalid data, producing a garbled overview.
    for (const AnalysisDao::AnalysisInfo& analysis : std::as_const(analyses)) {
        const WaveformFactory::VersionClass vc =
                WaveformFactory::waveformSummaryVersionToVersionClass(analysis.version);
        if (vc == WaveformFactory::VC_USE) {
            result.pWaveform = ConstWaveformPointer(
                    WaveformFactory::loadWaveformFromAnalysis(analysis));
            break;
        }
    }

    return result;
}

void OverviewCache::waveformSummaryPrepared() {
    QFutureWatcher<FutureWaveformResult>* watcher =
            static_cast<QFutureWatcher<FutureWaveformResult>*>(sender());
    FutureWaveformResult res = watcher->result();
    watcher->deleteLater();

    m_currentlyLoadingWaveform.remove(res.trackId);

    // The async job captured `pRequester` as a raw pointer; it may have
    // been deleted on the GUI thread while the load was in flight.
    // Re-check on the GUI thread before handing the (possibly dangling)
    // pointer to listeners, so they don't compare against `this` with
    // a dangling address.
    QPointer<QObject> requesterGuard(const_cast<QObject*>(res.requester));
    if (requesterGuard.isNull()) {
        return;
    }

    emit waveformSummaryReady(res.requester, res.trackId, res.pWaveform);
}
