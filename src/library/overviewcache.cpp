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
#include "waveform/overviews/waveformoverviewrenderer.h"
#include "waveform/waveformfactory.h"

namespace {

mixxx::Logger kLogger("OverviewCache");

QString pixmapCacheKey(TrackId trackId, QSize size, WOverview::Type type) {
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

void OverviewCache::onTrackAnalysisProgress(TrackId trackId, AnalyzerProgress analyzerProgress) {
    if (analyzerProgress < 1.0) {
        return;
    }
    m_tracksWithoutOverview.remove(trackId);
    // request update independent from paint events
    emit overviewChanged(trackId);
}

void OverviewCache::onTrackSummaryChanged(TrackId trackId) {
    emit overviewChanged(trackId);
}

QPixmap OverviewCache::requestOverview(
        WOverview::Type type,
        const TrackId trackId,
        const QObject* pRequester,
        const QSize desiredSize) {
    if (!trackId.isValid()) {
        return QPixmap();
    }

    if (m_currentlyLoading.contains(trackId)) {
        return QPixmap();
    }

    if (m_tracksWithoutOverview.contains(trackId)) {
        return QPixmap();
    }

    kLogger.info() << "requestOverview()" << trackId << pRequester << desiredSize;

    // request overview
    const QString cacheKey = pixmapCacheKey(trackId, desiredSize, type);
    QPixmap pixmap;
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
        const WOverview::Type type,
        const TrackId trackId,
        const QObject* pRequester,
        const QSize desiredSize) {
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
            QImage image = WaveformOverviewRenderer::instance()->render(
                    pLoadedTrackWaveformSummary, type);

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

    // Create pixmap, GUI thread only
    QPixmap pixmap = QPixmap::fromImage(res.image);
    if (!pixmap.isNull() && !res.resizedToSize.isEmpty()) {
        // we have to be sure that cacheKey is unique
        // because insert replaces the images with the same key
        const QString cacheKey = pixmapCacheKey(
                res.trackId, res.resizedToSize, res.type);
        QPixmapCache::insert(cacheKey, pixmap);
    }

    if (pixmap.isNull()) {
        // Avoid (too many) repeated lookups.
        // (there may still be identical request be processed due to
        // asynchronous processing)
        m_tracksWithoutOverview.insert(res.trackId);
    }
    m_currentlyLoading.remove(res.trackId);

    emit overviewReady(res.requester, res.trackId, !pixmap.isNull(), res.resizedToSize);
}
