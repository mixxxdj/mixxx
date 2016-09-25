#pragma once

#include <QSqlDatabase>

#include "analyzer/analyzerprogress.h"
#include "preferences/usersettings.h"
#include "track/track.h"
#include "util/db/dbconnectionpool.h"
#include "util/singleton.h"
#include "waveform/overviews/overviewrenderthread.h"

class AnalysisDao;
class PlayerManager;

struct OverviewCacheItem {
    AnalyzerProgress analyzerProgress;
    int completion;
    QMap<QString, QPair<WaveformSignalColors, QImage>> images;
};

class OverviewCache : public QObject, public Singleton<OverviewCache> {
    Q_OBJECT
  public:
    mixxx::OverviewType getOverviewType();
    void setOverviewType(mixxx::OverviewType);

    bool isOverviewNormalized();
    void setOverviewNormalized(bool);

    QPixmap getCachedOverviewPixmap(const TrackId trackId,
            const WaveformSignalColors signalColors,
            const QSize desiredSize) const;

    QImage getCachedOverviewImage(const TrackId trackId,
            const WaveformSignalColors signalColors) const;

    void requestOverview(
            const TrackId trackId,
            const WaveformSignalColors signalColors,
            const QObject* pRequester,
            const QSize desiredSize);

    struct FutureResult {
        FutureResult()
                : requester(nullptr) {
        }

        TrackId trackId;
        mixxx::OverviewType overviewType;
        WaveformSignalColors signalColors;
        QImage image;
        // QSize resizedToSize;
        const QObject* requester;
        int completion;
        float peak;
    };

  public slots:
    void overviewPrepared();

    void overviewRendered(TrackId,
            mixxx::OverviewType,
            WaveformSignalColors,
            QImage,
            int completion);

    void onTrackAnalyzerProgress(TrackId trackId, AnalyzerProgress analyzerProgress);

  signals:
    void overviewReady(const QObject* pRequester, TrackId trackId);

    void overviewChanged(TrackId);

    void overviewsChanged(const QList<TrackId>);

  protected:
    OverviewCache(UserSettingsPointer pConfig,
            mixxx::DbConnectionPoolPtr pDbConnectionPool,
            std::shared_ptr<PlayerManager> pPlayerManager);
    virtual ~OverviewCache() override = default;
    friend class Singleton<OverviewCache>;

    static FutureResult prepareOverview(
            UserSettingsPointer pConfig,
            mixxx::DbConnectionPoolPtr pDbConnectionPool,
            const TrackId trackId,
            const mixxx::OverviewType type,
            const WaveformSignalColors signalColors,
            const QObject* pRequester,
            const QSize desiredSize);

    static ConstWaveformPointer loadWaveform(const UserSettingsPointer pConfig,
            const mixxx::DbConnectionPoolPtr pDbConnectionPool,
            const TrackId trackId);

  private:
    UserSettingsPointer m_pConfig;
    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;

    std::unique_ptr<AnalysisDao> m_pAnalysisDao;

    OverviewRenderThread m_renderThread;

    QSet<TrackId> m_currentlyRendering;

    QSet<TrackId> m_tracksWithoutWaveform;

    QHash<TrackId, int> m_trackToAnalysisId;

    QCache<TrackId, OverviewCacheItem> m_overviewCache;

    mixxx::OverviewType m_type;
    bool m_overviewNormalized;
};
