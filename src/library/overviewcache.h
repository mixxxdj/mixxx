#pragma once

#include <QSqlDatabase>

#include "analyzer/analyzerprogress.h"
#include "preferences/usersettings.h"
#include "track/trackid.h"
#include "util/db/dbconnectionpool.h"
#include "util/singleton.h"
#include "waveform/overviewtype.h"
#include "waveform/waveform.h"

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

    /// Asynchronously load the waveform summary for `trackId` from the
    /// analysis database. When finished, `waveformSummaryReady` is
    /// emitted on the GUI thread, carrying the loaded
    /// `ConstWaveformPointer` (or a null pointer if no usable analysis
    /// was stored). This is a no-op if `trackId` is invalid or if a
    /// load for that track is already pending.
    void requestWaveformSummary(TrackId trackId, const QObject* pRequester);

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

    struct FutureWaveformResult {
        FutureWaveformResult()
                : requester(nullptr) {
        }

        TrackId trackId;
        ConstWaveformPointer pWaveform;
        const QObject* requester;
    };

  public slots:
    void overviewPrepared();
    void waveformSummaryPrepared();
    void onTrackAnalysisProgress(TrackId trackId, AnalyzerProgress analyzerProgress);

  signals:
    void overviewReady(
            const QObject* pRequester,
            TrackId trackId,
            bool pixmapValid);

    void overviewChanged(TrackId);

    void waveformSummaryReady(
            const QObject* pRequester,
            TrackId trackId,
            ConstWaveformPointer pWaveform);

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

    static FutureWaveformResult prepareWaveformSummary(
            UserSettingsPointer pConfig,
            mixxx::DbConnectionPoolPtr pDbConnectionPool,
            TrackId trackId,
            const QObject* pRequester);

  private:
    UserSettingsPointer m_pConfig;
    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;

    QSet<TrackId> m_currentlyLoading;
    QSet<TrackId> m_currentlyLoadingWaveform;
    QSet<TrackId> m_tracksWithoutOverview;
    QMultiHash<TrackId, QString> m_cacheKeysByTrackId;
};
