#pragma once

#include <QHash>
#include <QPointer>
#include <QSet>
#include <QSharedPointer>

#include "analyzer/analyzerprogress.h"
#include "library/baseexternallibraryfeature.h"
#include "library/dao/playlistdao.h"
#include "library/youtube/youtubeservice.h"
#include "track/trackid.h"
#include "util/parented_ptr.h"

class BaseTrackCache;
class KeyboardEventFilter;
class QSqlDatabase;
class TreeItem;
class WLibrary;
class WLibraryTextBrowser;
class YouTubeTrackModel;

class YouTubeFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
  public:
    YouTubeFeature(Library* pLibrary, UserSettingsPointer pConfig);
    ~YouTubeFeature() override = default;

    QVariant title() override {
        return tr("YouTube");
    }
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    TreeItemModel* sidebarModel() const override;
    void bindLibraryWidget(WLibrary* pLibraryWidget,
            KeyboardEventFilter* pKeyboard) override;

    void searchAndActivate(const QString& query);

    void requestDownloadToPlayer(
            const QString& videoId, const QString& group, bool play);
    void requestDownload(const QString& videoId);
    void requestDownloadToAutoDJ(
            const QString& videoId, PlaylistDAO::AutoDJSendLoc loc);

    /// Absolute path to the per-user yt-dlp cache directory. Created on demand.
    QString cacheDir() const;

    /// Resolve the ISO 3166-1 alpha-2 region used for the YouTube top-songs
    /// feed. Resolution order:
    ///   1. `[YouTube]/trending_region` user override (if non-empty)
    ///   2. The literal "GR" (Greece) — explicit project default. Stale geo-IP
    ///      cache and en_US locales must not switch this fork to United States.
    /// Always returns a non-empty 2-letter uppercase code.
    QString resolvedTrendingRegion() const;

  protected:
    void appendTrackIdsFromRightClickIndex(
            QList<TrackId>* trackIds, QString* pPlaylist) override;

  private slots:
    void onSearchResultsReady(
            const QString& query, const QList<mixxx::YouTubeVideoInfo>& results);
    void onSearchFailed(const QString& query, const QString& error);
    void onDownloadFinished(const QString& videoId, const QString& localPath);
    void onDownloadFailed(const QString& videoId, const QString& error);
    void onTrackAnalysisProgress(TrackId trackId, AnalyzerProgress analyzerProgress);
    /// Dispatch clicks on links rendered in the home pane HTML.
    /// `ytplay:VIDEOID`     → download/cache/analyze the track.
    /// `ytcached:LOCALPATH` → refresh the already-downloaded row.
    void onHomeAnchorClicked(const QUrl& url);

  private:
    /// Rebuild the sidebar tree from the current search-result and
    /// downloaded-track caches.
    void rebuildSidebar();
    /// Rebuild the HTML for the main YOUTUBE_HOME pane (the right-hand area
    /// the user sees when YouTube is selected). Mirrors the sidebar listing
    /// so the user has feedback that a search returned results, even before
    /// they unfold the tree node.
    void rebuildHomeHtml();
    void requestDownloadFile(const QString& videoId);
    /// Background repair of missing AutoDJ-queued tracks.
    void requestPrefetch(const QString& videoId);
    bool autoAnalyzeResultsEnabled() const;
    void setAutoAnalyzeResultsEnabled(bool enabled);
    void autoAnalyzeCurrentResults();
    /// If `pTrack` was downloaded by us and is no longer loaded on any deck,
    /// delete its cached audio file and purge it from the library DB so the
    /// disk doesn't grow unbounded. Tracks referenced by any playlist or
    /// crate (incl. the AutoDJ queue, which is itself a hidden playlist) are
    /// preserved.
    void maybeReleaseCachedTrack(const TrackPointer& pTrack);
    /// If `pTrack` is a YouTube-cache track whose file is missing, kick off
    /// a background re-download so the next play attempt succeeds. No-op for
    /// non-YouTube tracks or already-present files.
    void ensureDownloaded(const TrackPointer& pTrack);
    /// Walk the AutoDJ queue at startup and ensure every YouTube-cache track
    /// in it is present on disk — re-downloading any that have been swept.
    void prefetchAutoDjQueue();
    void syncAnalyzedTrackMetadata(const TrackPointer& pTrack);
    /// Persistent track table backing the right-hand pane. See
    /// YouTubeTrackModel for the placeholder/downloaded row model.
    QSharedPointer<BaseTrackCache> m_pTrackCache;
    YouTubeTrackModel* m_pTrackModel = nullptr;
    /// Replace all rows in `youtube_library` with the supplied YouTube
    /// videos, then refresh the model so the view picks up the new rows.
    /// `videos` may include both downloaded (file present in cacheDir())
    /// and not-yet-downloaded entries; we resolve which is which row by
    /// row when building the INSERT.
    void replaceTrackTable(const QList<mixxx::YouTubeVideoInfo>& videos);
    /// Append a single downloaded entry (or update its row to point at the
    /// real file path) so the "Downloaded" column reflects the new file
    /// without a full table rebuild.
    void upsertDownloadedRow(const QString& videoId,
            const QString& localPath,
            const QString& title,
            const QString& uploader,
            int durationSec);

    parented_ptr<TreeItemModel> m_pSidebarModel;
    QPointer<WLibraryTextBrowser> m_pHomeView;
    mixxx::YouTubeService m_service;
    QString m_lastQuery;
    QList<mixxx::YouTubeVideoInfo> m_lastResults;
    // videoId -> human-readable label (used for the "Downloaded" branch).
    QHash<QString, QString> m_downloadedTracks;
    struct PendingPlayerLoad {
        QString group;
        bool play = false;
    };
    QHash<QString, QList<PendingPlayerLoad>> m_pendingPlayerLoads;
    QHash<QString, QList<PlaylistDAO::AutoDJSendLoc>> m_pendingAutoDjLoads;
    QSet<QString> m_videoIdsDownloading;
    /// Track download retry attempts per video id. After kMaxDownloadRetries
    /// we give up and surface the error to the user.
    QHash<QString, int> m_downloadRetryCount;
    static constexpr int kMaxDownloadRetries = 2;
    /// Last error message reported by the underlying YouTubeService for the
    /// current `m_lastQuery`. Cleared when a new search is started or when
    /// results arrive successfully. When non-empty, the home pane shows the
    /// error in place of the perpetual "Searching…" placeholder.
    QString m_lastSearchError;
};
