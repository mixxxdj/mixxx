#pragma once

#include <QSqlDatabase>

#include "analyzer/analyzerprogress.h"
#include "preferences/usersettings.h"
#include "track/track.h"
#include "util/db/dbconnectionpool.h"
#include "util/singleton.h"
#include "widget/woverview.h"

class WaveformSignalColors;

class OverviewCache : public QObject, public Singleton<OverviewCache> {
    Q_OBJECT
  public:
    void onTrackSummaryChanged(TrackId);

    QPixmap requestOverview(
            const WOverview::Type type,
            const WaveformSignalColors& signalColors,
            const TrackId trackId,
            const QObject* pRequester,
            const QSize desiredSize);

    struct FutureResult {
        FutureResult()
                : requester(nullptr) {
        }

        TrackId trackId;
        WOverview::Type type;
        QImage image;
        QSize resizedToSize;
        const QObject* requester;
    };

  public slots:
    void overviewPrepared();
    void onTrackAnalysisProgress(TrackId trackId, AnalyzerProgress analyzerProgress);

  signals:
    void overviewReady(
            const QObject* pRequester,
            const TrackId trackId,
            bool pixmapValid,
            const QSize resizedToSize); // Currently only needed for debugging

    void overviewChanged(TrackId);

  protected:
    OverviewCache(UserSettingsPointer pConfig,
            mixxx::DbConnectionPoolPtr m_pDbConnectionPool);
    virtual ~OverviewCache() override = default;
    friend class Singleton<OverviewCache>;

    static FutureResult prepareOverview(
            UserSettingsPointer pConfig,
            mixxx::DbConnectionPoolPtr pDbConnectionPool,
            const WOverview::Type type,
            const WaveformSignalColors& signalColors,
            const TrackId trackId,
            const QObject* pRequester,
            const QSize desiredSize);

  private:
    UserSettingsPointer m_pConfig;
    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;

    QSet<TrackId> m_currentlyLoading;
    QSet<TrackId> m_tracksWithoutOverview;
};
