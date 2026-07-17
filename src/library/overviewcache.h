#pragma once

#include <QSqlDatabase>

#include "analyzer/analyzerprogress.h"
#include "analyzer/trackanalysisscheduler.h"
#include "preferences/usersettings.h"
#include "track/trackid.h"
#include "util/db/dbconnectionpool.h"
#include "util/singleton.h"
#include "waveform/overviewtype.h"
#include "waveform/waveform.h"

class TrackDAO;
class WaveformSignalColors;

class OverviewCache : public QObject, public Singleton<OverviewCache> {
    Q_OBJECT
  public:
    void onTrackSummaryChanged(TrackId);

    /// Inject the library's `TrackDAO`. Used by the
    /// `requestWaveformSummary(QString)` overload to resolve a
    /// track file location into a `TrackId` through the library
    /// database. Must be called once after `createInstance`, before
    /// any location-based request is made.
    void setTrackDAO(TrackDAO* pTrackDAO) {
        m_pTrackDAO = pTrackDAO;
    }

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

    /// Variant of `requestWaveformSummary` that resolves the track by
    /// its file location rather than its `TrackId`. The location is
    /// translated to a `TrackId` through the library database (a
    /// single indexed query on the GUI thread) and then the existing
    /// `TrackId`-based load path is reused. This is useful for tracks
    /// that are not in the database (no valid `TrackId`) but whose
    /// file URL is known, e.g. for tracks shown in browse/external
    /// table views. If no track matches `trackLocation` the call is
    /// a no-op.
    void requestWaveformSummary(const QString& trackLocation, const QObject* pRequester);

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

    /// Emitted on every analyzer progress update for `trackId`,
    /// including partial progress while analysis is in flight.
    /// Connect to this signal to repaint waveform overview
    /// visualizations during analysis (the `Track` itself only emits
    /// `waveformSummaryUpdated` once at the start and once at the
    /// end of analysis, so it can't drive progressive drawing on its
    /// own).
    void analyzerProgress(TrackId trackId, AnalyzerProgress analyzerProgress);

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

    /// Borrowed pointer to the library's `TrackDAO` (lives on the GUI
    /// thread, owned by `TrackCollection`). Used to resolve track file
    /// locations to `TrackId`s in `requestWaveformSummary(QString)`.
    TrackDAO* m_pTrackDAO = nullptr;

    QSet<TrackId> m_currentlyLoading;
    QSet<TrackId> m_currentlyLoadingWaveform;
    QSet<TrackId> m_tracksWithoutOverview;
    QMultiHash<TrackId, QString> m_cacheKeysByTrackId;
};
