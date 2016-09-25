#include "library/overviewcache.h"

#include <QFutureWatcher>
#include <QPixmapCache>
#include <QSqlDatabase>
#include <QtConcurrentRun>
#include <QtSql>

#include "library/dao/analysisdao.h"
#include "moc_overviewcache.cpp"
#include "util/db/dbconnectionpooled.h"
#include "util/db/dbconnectionpooler.h"
#include "util/logger.h"
#include "waveform/overviews/waveformoverviewrenderer.h"
#include "waveform/waveformfactory.h"

namespace {

mixxx::Logger kLogger("OverviewCache");

QString pixmapCacheKey(TrackId trackId, QSize size) {
    // return QString("Overview_%1").arg(trackId.toInt());
    // return QString("Overview_%1_%2").arg(trackId.toInt()).arg(width);
    return QString("Overview_%1_%2_%3")
            .arg(trackId.toString())
            .arg(size.width())
            .arg(size.height());
}

// The transformation mode when scaling images
const Qt::TransformationMode kTransformationMode = Qt::SmoothTransformation;

// Resizes the image (preserving aspect ratio) to width.
inline QImage resizeImageWidth(const QImage& image, int width) {
    return image.scaledToWidth(width, kTransformationMode);
}

inline QImage resizeImageSize(const QImage& image, QSize size) {
    return image.scaled(size, Qt::IgnoreAspectRatio, kTransformationMode);
}
} // anonymous namespace

// OverviewCache::OverviewCache(QObject *parent) : QObject(parent)
OverviewCache::OverviewCache() {
}

// OverviewCache::~OverviewCache() {
//  qDebug()
//}

/*
void OverviewCache::initialize(UserSettingsPointer pConfig) {
    m_database = QSqlDatabase::addDatabase("QSQLITE", "OVERVIEW_CACHE");
    if (!m_database.isOpen()) {
        m_database.setHostName("localhost");
        m_database.setDatabaseName(QDir(pConfig->getSettingsPath()).filePath("mixxxdb.sqlite"));
        m_database.setUserName("mixxx");
        m_database.setPassword("mixxx");

        // Open the database connection in this thread.
        if (!m_database.open()) {
            qDebug() << "Failed to open database from overview cacher thread."
                     << m_database.lastError();
        }
    }

    // m_pAnalysisDao = std::make_unique<AnalysisDao>(pConfig);
}
*/

void OverviewCache::setConfig(UserSettingsPointer pConfig) {
    m_pConfig = pConfig;
}

void OverviewCache::setDbConnectionPool(mixxx::DbConnectionPoolPtr pDbConnectionPool) {
    m_pDbConnectionPool = std::move(pDbConnectionPool);
}

void OverviewCache::onTrackSummaryChanged(TrackId trackId) {
    emit overviewChanged(trackId);
}

QPixmap OverviewCache::requestOverview(const TrackId trackId,
        const QObject* pRequester,
        const QSize desiredSize) {
    kLogger.info() << "requestOverview()" << trackId << pRequester << desiredSize;

    if (!trackId.isValid()) {
        return QPixmap();
    }

    if (m_currentlyLoading.contains(trackId)) {
        return QPixmap();
    }

    QString cacheKey = pixmapCacheKey(trackId, desiredSize);

    QPixmap pixmap;
    if (QPixmapCache::find(cacheKey, &pixmap)) {
        return pixmap;
    }

    m_currentlyLoading.insert(trackId);

    QFutureWatcher<FutureResult>* watcher = new QFutureWatcher<FutureResult>(this);
    QFuture<FutureResult> future = QtConcurrent::run(
            &OverviewCache::prepareOverview,
            m_pConfig,
            m_pDbConnectionPool,
            trackId,
            pRequester,
            desiredSize);
    connect(watcher,
            &QFutureWatcher<FutureResult>::finished,
            this,
            &OverviewCache::overviewPrepared);
    watcher->setFuture(future);

    return QPixmap();

    /*
    ConstWaveformPointer pLoadedTrackWaveformSummary;
    QList<AnalysisDao::AnalysisInfo> analyses =
            m_pAnalysisDao->getAnalysesForTrackByType(trackId,
    AnalysisDao::AnalysisType::TYPE_WAVESUMMARY);

    if (analyses.size() != 1) {
        return QPixmap();
    }
    */

    /*
    QListIterator<AnalysisDao::AnalysisInfo> it(analyses);
    while (it.hasNext()) {
        const AnalysisDao::AnalysisInfo& analysis = it.next();
        pLoadedTrackWaveformSummary = ConstWaveformPointer(
                WaveformFactory::loadWaveformFromAnalysis(analysis));
    }
    */
    /*
    pLoadedTrackWaveformSummary = ConstWaveformPointer(
                WaveformFactory::loadWaveformFromAnalysis(analyses[0]));

    if (!pLoadedTrackWaveformSummary.isNull() && !pLoadedTrackWaveformSummary->isValid()) {
        return QPixmap();
    }

    QImage image = pLoadedTrackWaveformSummary->renderToImage();

    if (!image.isNull() && desiredWidth > 0) {
        image = resizeImageWidth(image, desiredWidth);
    }
    */

    // Create pixmap, GUI thread only
    /*QPixmap*/
    /*pixmap = QPixmap::fromImage(image);
if (!pixmap.isNull() && desiredWidth != 0) {
// we have to be sure that res.cover.hash is unique
// because insert replaces the images with the same key
QString cacheKey = pixmapCacheKey(
    trackId, desiredWidth);
QPixmapCache::insert(cacheKey, pixmap);
}

return pixmap;*/
}

// static
OverviewCache::FutureResult OverviewCache::prepareOverview(
        const UserSettingsPointer pConfig,
        const mixxx::DbConnectionPoolPtr pDbConnectionPool,
        const TrackId trackId,
        const QObject* pRequester,
        const QSize desiredSize) {
    mixxx::DbConnectionPooler dbConnectionPooler(pDbConnectionPool);

    AnalysisDao analysisDao(pConfig);
    analysisDao.initialize(mixxx::DbConnectionPooled(pDbConnectionPool));

    QList<AnalysisDao::AnalysisInfo> analyses =
            analysisDao.getAnalysesForTrackByType(
                    trackId, AnalysisDao::AnalysisType::TYPE_WAVESUMMARY);

    /*
    QListIterator<AnalysisDao::AnalysisInfo> it(analyses);
    if (it.hasNext()) {
        const AnalysisDao::AnalysisInfo& analysis = it.next();
    }
    */

    QImage image;

    if (!analyses.isEmpty()) {
        ConstWaveformPointer pLoadedTrackWaveformSummary = ConstWaveformPointer(
                WaveformFactory::loadWaveformFromAnalysis(analyses.first()));

        /*&& pLoadedTrackWaveformSummary->isValid()*/
        if (!pLoadedTrackWaveformSummary.isNull()) {
            image = WaveformOverviewRenderer::instance()->render(pLoadedTrackWaveformSummary);

            if (!image.isNull() && !desiredSize.isEmpty()) {
                // image = resizeImageWidth(image, desiredWidth);
                image = resizeImageSize(image, desiredSize);
            }
        }
    }

    FutureResult result;
    result.trackId = trackId;
    result.requester = pRequester;
    result.image = image; // QImage();
    result.resizedToSize = desiredSize;
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
        // we have to be sure that res.cover.hash is unique
        // because insert replaces the images with the same key
        QString cacheKey = pixmapCacheKey(
                res.trackId, res.resizedToSize);
        QPixmapCache::insert(cacheKey, pixmap);
    }

    m_currentlyLoading.remove(res.trackId);

    emit overviewReady(res.requester, res.trackId, pixmap, res.resizedToSize);
}
