#include "library/overviewcache.h"

#include <QFutureWatcher>
#include <QPixmapCache>
#include <QSqlDatabase>
#include <QtConcurrentRun>

#include "library/dao/analysisdao.h"
#include "moc_overviewcache.cpp"
#include "util/color/color.h"
#include "util/db/dbconnectionpooled.h"
#include "util/db/dbconnectionpooler.h"
#include "util/logger.h"
#include "util/painterscope.h"
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
    QSet<TrackId> affectedIds;
    for (auto it = m_cacheKeysByTrackId.constBegin();
            it != m_cacheKeysByTrackId.constEnd();
            ++it) {
        QPixmapCache::remove(it.value());
        affectedIds.insert(it.key());
    }
    m_cacheKeysByTrackId.clear();
    m_tracksWithoutOverview.clear();
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

    const QString cacheKey = pixmapCacheKey(trackId, desiredSize, type);
    QPixmap pixmap;
    QPixmapCache::find(cacheKey, &pixmap);
    return pixmap;
}

QPixmap OverviewCache::requestUncachedOverview(
        mixxx::OverviewType type,
        const WaveformSignalColors& signalColors,
        TrackId trackId,
        const QList<mixxx::CueInfo>& cueInfos,
        double trackDurationMillis,
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
            cueInfos,
            trackDurationMillis,
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
        const QList<mixxx::CueInfo>& cueInfos,
        double trackDurationMillis,
        const QObject* pRequester,
        QSize desiredSize) {
    FutureResult result;
    result.trackId = trackId;
    result.type = type;
    result.requester = pRequester;
    result.image = QImage();
    result.resizedToSize = desiredSize;

    if (!trackId.isValid() || desiredSize.isEmpty()) {
        return result;
    }

    const bool bDrawMinuteMarkers = pConfig->getValue(
            ConfigKey("[Waveform]", QStringLiteral("draw_library_overview_minute_markers")), true);

    mixxx::DbConnectionPooler dbConnectionPooler(pDbConnectionPool);
    QSqlDatabase database = mixxx::DbConnectionPooled(pDbConnectionPool);

    AnalysisDao analysisDao(pConfig);
    analysisDao.initialize(database);

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
                // draw markers on the resized image for crisp lines
                // at the final display resolution
                if (trackDurationMillis > 0) {
                    if (bDrawMinuteMarkers) {
                        QList<int> markerXPositions;
                        for (double currentMarkerMillis = 60000;
                                currentMarkerMillis < trackDurationMillis;
                                currentMarkerMillis += 60000) {
                            markerXPositions.append(static_cast<int>(
                                    (currentMarkerMillis / trackDurationMillis) *
                                    image.width()));
                        }
                        drawMinuteMarkers(&image, markerXPositions);
                    }
                    if (!cueInfos.isEmpty()) {
                        drawHotcueMarkers(&image, cueInfos, trackDurationMillis);
                    }
                }
            }
            result.image = image;
        }
    }

    return result;
}

// static
void OverviewCache::drawHotcueMarkers(
        QImage* pImage,
        const QList<mixxx::CueInfo>& cueInfos,
        double trackDurationMillis) {
    if (pImage->format() != QImage::Format_ARGB32_Premultiplied) {
        *pImage = pImage->convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }

    QPainter markerPainter(pImage);
    markerPainter.setRenderHint(QPainter::Antialiasing, false);
    const int imageWidth = pImage->width();
    const int imageHeight = pImage->height();
    PainterScope painterScope(&markerPainter);
    for (const auto& cueInfo : cueInfos) {
        if (cueInfo.getType() != mixxx::CueType::HotCue) {
            continue;
        }

        auto positionMillis = cueInfo.getStartPositionMillis();
        if (!positionMillis.has_value()) {
            continue;
        }

        // allow negative positions (preroll)
        if (*positionMillis > trackDurationMillis) {
            continue;
        }

        const int x = static_cast<int>(
                (*positionMillis / trackDurationMillis) * imageWidth);

        if (x < 0 || x >= imageWidth) {
            continue;
        }

        // use same rendering style as deck overview:
        // contrasting border line + bright fill line
        auto color = cueInfo.getColor();
        if (!color.has_value()) {
            continue;
        }

        QColor fillColor = QColor::fromRgb(*color).lighter(110);
        QColor borderColor = Color::chooseContrastColor(
                QColor::fromRgb(*color), 120);

        // draw border/shadow line (1px wide, offset by -1)
        markerPainter.setPen(borderColor);
        markerPainter.drawLine(x - 1, 0, x - 1, imageHeight);

        // draw main bright marker line (1px wide)
        markerPainter.setPen(fillColor);
        markerPainter.drawLine(x, 0, x, imageHeight);
    }
}

// static
void OverviewCache::drawMinuteMarkers(
        QImage* pImage,
        const QList<int>& markerXPositions) {
    if (pImage->format() != QImage::Format_ARGB32_Premultiplied) {
        *pImage = pImage->convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }

    QPainter markerPainter(pImage);
    markerPainter.setRenderHint(QPainter::Antialiasing, false);
    const int imageWidth = pImage->width();
    const int imageHeight = pImage->height();
    const int markerHeight = static_cast<int>(imageHeight * 0.2);
    const int lowerMarkerYPos = static_cast<int>(imageHeight * 0.8);
    const int inset = 1;
    const QColor minuteColor(255, 255, 255, 204); // 80% opacity

    for (int x : markerXPositions) {
        if (x >= 0 && x < imageWidth) {
            markerPainter.fillRect(x, inset, 1, markerHeight - inset, minuteColor);
            markerPainter.fillRect(x, lowerMarkerYPos, 1, imageHeight - lowerMarkerYPos - inset, minuteColor);
        }
    }
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
