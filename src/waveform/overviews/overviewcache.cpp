#include "waveform/overviews/overviewcache.h"

#include <QFutureWatcher>
#include <QPixmapCache>
#include <QSqlDatabase>
#include <QtConcurrentRun>
#include <QtSql>

#include "library/dao/analysisdao.h"
#include "mixer/playermanager.h"
#include "moc_overviewcache.cpp"
#include "preferences/waveformoverviewsettings.h"
#include "track/globaltrackcache.h"
#include "util/db/dbconnectionpooled.h"
#include "util/db/dbconnectionpooler.h"
#include "util/logger.h"
#include "util/timer.h"
#include "waveform/overviews/overviewrenderthread.h"
#include "waveform/waveformfactory.h"

namespace {

mixxx::Logger kLogger("OverviewCache");

/*
QString pixmapCacheKey(TrackId trackId, QSize size) {
    // return QString("Overview_%1").arg(trackId.toInt());
    // return QString("Overview_%1_%2").arg(trackId.toInt()).arg(width);
    return QString("Overview_%1_%2_%3")
            .arg(trackId.toString())
            .arg(size.width())
            .arg(size.height());
}

QString colorToHex(const QColor& color) {
    return QString::number(color.rgb(), 16);
}

QString formCacheKey(TrackId trackId, mixxx::OverviewType waveformType, WaveformSignalColors wsc) {
    switch (waveformType) {
    case mixxx::OverviewType::Filtered:
        return QString("%1_Filtered_%2_%3_%4")
                .arg(trackId.toString())
                .arg(wsc.getLowColor().rgb(), 0, 16)
                .arg(wsc.getMidColor().rgb(), 0, 16)
                .arg(wsc.getHighColor().rgb(), 0, 16);
    case mixxx::OverviewType::HSV:
        return QString("%1_HSV_%2").arg(trackId.toString()).arg(wsc.getLowColor().rgb(), 0, 16);
    case mixxx::OverviewType::RGB:
    default:
        return QString("%1_RGB_%2_%3_%4")
                .arg(trackId.toString())
                .arg(wsc.getRgbLowColor().rgb(), 0, 16)
                .arg(wsc.getRgbMidColor().rgb(), 0, 16)
                .arg(wsc.getRgbHighColor().rgb(), 0, 16);
    }
}
*/

QString formImageCacheKey(mixxx::OverviewType waveformType, WaveformSignalColors wsc) {
    switch (waveformType) {
    case mixxx::OverviewType::Filtered:
        return QString("Filtered_%2_%3_%4")
                .arg(wsc.getLowColor().rgb(), 0, 16)
                .arg(wsc.getMidColor().rgb(), 0, 16)
                .arg(wsc.getHighColor().rgb(), 0, 16);
    case mixxx::OverviewType::HSV:
        return QString("HSV_%2").arg(wsc.getLowColor().rgb(), 0, 16);
    case mixxx::OverviewType::RGB:
    default:
        return QString("RGB_%2_%3_%4")
                .arg(wsc.getRgbLowColor().rgb(), 0, 16)
                .arg(wsc.getRgbMidColor().rgb(), 0, 16)
                .arg(wsc.getRgbHighColor().rgb(), 0, 16);
    }
}

// The transformation mode when scaling images
const Qt::TransformationMode kTransformationMode = Qt::SmoothTransformation;

inline QImage resizeImageSize(const QImage& image, QSize size) {
    return image.scaled(size, Qt::IgnoreAspectRatio, kTransformationMode);
}

} // anonymous namespace

OverviewCache::OverviewCache(UserSettingsPointer pConfig,
        mixxx::DbConnectionPoolPtr pDbConnectionPool,
        std::shared_ptr<PlayerManager> pPlayerManager)
        : m_pConfig(pConfig),
          m_pDbConnectionPool(std::move(pDbConnectionPool)),
          m_renderThread(pDbConnectionPool, pConfig),
          m_overviewCache(50) {
    WaveformOverviewSettings overviewSettings(m_pConfig);
    m_type = overviewSettings.getOverviewType();
    m_overviewNormalized = overviewSettings.isOverviewNormalized();

    connect(pPlayerManager.get(),
            &PlayerManager::trackAnalyzerProgress,
            this,
            &OverviewCache::onTrackAnalyzerProgress);

    connect(&m_renderThread,
            &OverviewRenderThread::overviewRendered,
            this,
            &OverviewCache::overviewRendered);

    m_renderThread.suspend();
    // m_renderThread.resume();
    m_renderThread.start();
}

void OverviewCache::setOverviewType(mixxx::OverviewType type) {
    kLogger.info() << "setOverviewType" << type;

    if (type == m_type) {
        return;
    }

    m_type = type;
    m_overviewCache.clear();

    emit overviewsChanged(m_overviewCache.keys());
}

void OverviewCache::setOverviewNormalized(bool normalized) {
    kLogger.info() << "setOverviewNormalized" << normalized;

    if (normalized == m_overviewNormalized) {
        return;
    }

    m_overviewNormalized = normalized;
}

void OverviewCache::onTrackAnalyzerProgress(TrackId trackId, AnalyzerProgress analyzerProgress) {
    kLogger.info() << "onTrackAnalyzerProgress" << trackId << analyzerProgress;

    m_tracksWithoutWaveform.remove(trackId);

    /*
    if (m_overviewCache.contains(trackId)) {
        TrackPointer pTrack = GlobalTrackCacheLocker().lookupTrackById(trackId);
        if (pTrack) {
            ConstWaveformPointer pWaveform = pTrack->getWaveform();
            if (pWaveform) {
                OverviewCacheItem* pItem = m_overviewCache[trackId];
                if (pItem != nullptr) {
                    for (auto& item : pItem->images) {
                        OverviewRenderThread::drawNextPixmapPart(item.second,
                                pWaveform,
                                m_type,
                                item.first,
                                pItem->completion);
                    }
                }
            }
        }
    }
    */
}

QImage OverviewCache::getCachedOverviewImage(
        const TrackId trackId, const WaveformSignalColors signalColors) const {
    if (!trackId.isValid()) {
        return QImage();
    }

    OverviewCacheItem* pItem = m_overviewCache[trackId];
    if (pItem != nullptr) {
        QString imageCacheKey = formImageCacheKey(m_type, signalColors);
        if (pItem->images.contains(imageCacheKey)) {
            // TODO: Resize
            // return QPixmap::fromImage();
            return pItem->images[imageCacheKey].second;
        }
    }

    /*
    QImage image =
    if (!image.isNull()) {
        return QPixmap::fromImage(image);
    }
    */

    // QString cacheKey = pixmapCacheKey(trackId, desiredSize);

    /*if (m_tracksCurrentlyAnalyzed.contains(trackId)) {
        QPixmapCache::remove(cacheKey);
        return QPixmap();
    }*/

    /*
    QPixmap pixmap;
    if (!QPixmapCache::find(cacheKey, &pixmap)) {
        return QPixmap();
    }

    return pixmap;
    */

    return QImage();
}

void OverviewCache::requestOverview(const TrackId trackId,
        const WaveformSignalColors signalColors,
        const QObject* pRequester,
        const QSize desiredSize) {
    ScopedTimer t(QStringLiteral("OverviewCache::requestOverview"));

    kLogger.info() << "requestOverview()" << trackId << pRequester << desiredSize;

    if (!trackId.isValid()) {
        return;
    }

    if (m_currentlyRendering.contains(trackId)) {
        return;
    }

    if (m_tracksWithoutWaveform.contains(trackId)) {
        return;
    }

    m_currentlyRendering.insert(trackId);

    // m_renderThread.scheduleRender(trackId, m_type, signalColors);

    // The watcher will be deleted in overviewPrepared()
    QFutureWatcher<FutureResult>* watcher = new QFutureWatcher<FutureResult>(this);
    QFuture<FutureResult> future = QtConcurrent::run(
            &OverviewCache::prepareOverview,
            m_pConfig,
            m_pDbConnectionPool,
            trackId,
            m_type,
            signalColors,
            pRequester,
            desiredSize);
    connect(watcher,
            &QFutureWatcher<FutureResult>::finished,
            this,
            &OverviewCache::overviewPrepared);
    watcher->setFuture(future);
}

// static
ConstWaveformPointer OverviewCache::loadWaveform(const UserSettingsPointer pConfig,
        const mixxx::DbConnectionPoolPtr pDbConnectionPool,
        const TrackId trackId) {
    TrackPointer pTrack = GlobalTrackCacheLocker().lookupTrackById(trackId);
    if (pTrack) {
        ConstWaveformPointer pWaveform = pTrack->getWaveform();
        if (pWaveform) {
            return pWaveform;
        }
    }

    mixxx::DbConnectionPooler dbConnectionPooler(pDbConnectionPool);

    AnalysisDao analysisDao(pConfig);
    analysisDao.initialize(mixxx::DbConnectionPooled(pDbConnectionPool));

    QList<AnalysisDao::AnalysisInfo> analyses = analysisDao.getAnalysesForTrackByType(
            trackId, AnalysisDao::AnalysisType::TYPE_WAVESUMMARY);
    if (analyses.isEmpty()) {
        kLogger.info() << "Track" << trackId << "has no WAVESUMMARY analysis";
        return ConstWaveformPointer();
    }

    return ConstWaveformPointer(WaveformFactory::loadWaveformFromAnalysis(analyses.first()));
}

// static
OverviewCache::FutureResult OverviewCache::prepareOverview(
        const UserSettingsPointer pConfig,
        const mixxx::DbConnectionPoolPtr pDbConnectionPool,
        const TrackId trackId,
        const mixxx::OverviewType type,
        const WaveformSignalColors signalColors,
        const QObject* pRequester,
        const QSize desiredSize) {
    ScopedTimer t(QStringLiteral("OverviewCache::prepareOverview"));

    kLogger.info() << "prepareOverview()" << trackId << type << pRequester << desiredSize;

    RenderResult res;

    ConstWaveformPointer pLoadedTrackWaveformSummary =
            loadWaveform(pConfig, pDbConnectionPool, trackId);
    if (pLoadedTrackWaveformSummary.isNull()) {
        kLogger.info() << "Analysis for track" << trackId << "could not be loaded";
    } else {
        res = OverviewRenderThread::render(
                pLoadedTrackWaveformSummary, type, signalColors);
    }

    FutureResult result;
    result.trackId = trackId;
    result.overviewType = type;
    result.signalColors = signalColors;
    result.requester = pRequester;
    result.image = res.image;
    // result.resizedToSize = desiredSize;
    result.completion = res.completion;
    result.peak = res.waveformPeak;
    return result;
}

// watcher
void OverviewCache::overviewPrepared() {
    FutureResult res;
    {
        QFutureWatcher<FutureResult>* pFutureWatcher =
                static_cast<QFutureWatcher<FutureResult>*>(sender());
        VERIFY_OR_DEBUG_ASSERT(pFutureWatcher) {
            return;
        }
        res = pFutureWatcher->result();
        pFutureWatcher->deleteLater();
    }

    kLogger.info() << "overviewPrepared" << res.trackId << res.image;

    if (res.image.isNull()) {
        m_tracksWithoutWaveform.insert(res.trackId);
    } else {
        if (m_overviewCache.contains(res.trackId)) {
            OverviewCacheItem* pItem = m_overviewCache[res.trackId];
            pItem->completion = res.completion;
            QString imageCacheKey = formImageCacheKey(res.overviewType, res.signalColors);
            pItem->images[imageCacheKey] = QPair(res.signalColors, res.image);
        } else {
            OverviewCacheItem* pItem = new OverviewCacheItem;
            pItem->completion = res.completion;
            QString imageCacheKey = formImageCacheKey(res.overviewType, res.signalColors);
            pItem->images[imageCacheKey] = QPair(res.signalColors, res.image);
            m_overviewCache.insert(res.trackId, pItem);
        }

        kLogger.info()
                << "Emitting overviewReady() signal" << res.requester << res.trackId;
        emit overviewReady(res.requester, res.trackId);
    }

    m_currentlyRendering.remove(res.trackId);
}

void OverviewCache::overviewRendered(TrackId trackId,
        mixxx::OverviewType overviewType,
        WaveformSignalColors signalColors,
        QImage image,
        int completion) {
    if (m_overviewCache.contains(trackId)) {
        OverviewCacheItem* pItem = m_overviewCache[trackId];
        pItem->completion = completion;
        QString imageCacheKey = formImageCacheKey(overviewType, signalColors);
        pItem->images[imageCacheKey] = QPair(signalColors, image);
    } else {
        OverviewCacheItem* pItem = new OverviewCacheItem;
        pItem->completion = completion;
        QString imageCacheKey = formImageCacheKey(overviewType, signalColors);
        pItem->images[imageCacheKey] = QPair(signalColors, image);
        m_overviewCache.insert(trackId, pItem);
    }

    kLogger.info() << "Emitting overviewReady() signal" << trackId << overviewType << completion;
    emit overviewChanged(trackId);
}
