#pragma once

#include <QSqlDatabase>

#include "preferences/usersettings.h"
#include "track/track.h"
#include "util/db/dbconnectionpool.h"
#include "util/singleton.h"

class AnalysisDao;

/*
struct CacheItem{
    TrackId trackId;
    bool isLoading;
    QImage image;
};
*/

class OverviewCache : public QObject, public Singleton<OverviewCache> {
    Q_OBJECT
  public:
    void setConfig(UserSettingsPointer pConfig);

    void setDbConnectionPool(mixxx::DbConnectionPoolPtr pDbConnectionPool);

    void onTrackSummaryChanged(TrackId);

    QPixmap requestOverview(
            const TrackId trackId,
            const QObject* pRequester,
            const QSize desiredSize);

    struct FutureResult {
        FutureResult()
                : /*resToWidth(0),*/
                  requester(nullptr) {
        }

        TrackId trackId;
        QImage image;
        QSize resizedToSize;
        const QObject* requester;
    };

  public slots:
    void overviewPrepared();

  signals:
    void overviewReady(
            const QObject* pRequester,
            TrackId trackId,
            QPixmap pixmap,
            QSize resizedToSize);

    void overviewChanged(TrackId);

  protected:
    OverviewCache();
    virtual ~OverviewCache() override = default;
    friend class Singleton<OverviewCache>;

    static FutureResult prepareOverview(
            UserSettingsPointer pConfig,
            mixxx::DbConnectionPoolPtr pDbConnectionPool,
            const TrackId trackId,
            const QObject* pRequester,
            const QSize desiredSize);

  private:
    UserSettingsPointer m_pConfig;
    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;

    // QSqlDatabase m_database;
    // std::unique_ptr<AnalysisDao> m_pAnalysisDao;

    QSet<TrackId> m_currentlyLoading;

    QHash<TrackId, QImage> m_cache;
};
