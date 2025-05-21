#pragma once

#include <QSqlDatabase>

#include "analyzer/analyzerprogress.h"
#include "preferences/usersettings.h"
#include "track/trackid.h"
#include "util/db/dbconnectionpool.h"
#include "util/singleton.h"
#include "waveform/overviewtype.h"

class WaveformSignalColors;

class OverviewCache : public QObject, public Singleton<OverviewCache> {
    Q_OBJECT
  public:
    void onTrackSummaryChanged(TrackId);

    QPixmap requestCachedOverview(
            mixxx::OverviewType type,
            TrackId trackId,
            const QObject* pRequester,
            QSize desiredSize);
    QPixmap requestUncachedOverview(
            mixxx::OverviewType type,
            const WaveformSignalColors& signalColors,
            TrackId trackId,
            const QObject* pRequester,
            QSize desiredSize);

    struct FutureResult {
        FutureResult()
                : requester(nullptr) {
        }

        TrackId trackId;
        mixxx::OverviewType type;
        QImage image;
        QSize resizedToSize;
        const QObject* requester;
    };

  public slots:
    void onNormalizeOrVisualGainChanged();
    void overviewPrepared();
    void onTrackAnalysisProgress(TrackId trackId, AnalyzerProgress analyzerProgress);

  signals:
    void overviewReady(
            const QObject* pRequester,
            TrackId trackId,
            bool pixmapValid);

    void overviewChanged(TrackId);

  protected:
    OverviewCache(UserSettingsPointer pConfig,
            mixxx::DbConnectionPoolPtr m_pDbConnectionPool);
    virtual ~OverviewCache() override = default;
    friend class Singleton<OverviewCache>;

    static FutureResult prepareOverview(
            UserSettingsPointer pConfig,
            mixxx::DbConnectionPoolPtr pDbConnectionPool,
            mixxx::OverviewType type,
            const WaveformSignalColors& signalColors,
            TrackId trackId,
            const QObject* pRequester,
            QSize desiredSize);

  private:
    UserSettingsPointer m_pConfig;
    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;

    QSet<TrackId> m_currentlyLoading;
    QSet<TrackId> m_tracksWithoutOverview;
    QMultiHash<TrackId, QString> m_cacheKeysByTrackId;
    bool m_clearingCache;
    bool m_stopClearing;
};
