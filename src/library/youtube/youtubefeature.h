#pragma once

#include <QHash>
#include <QNetworkAccessManager>
#include <QPointer>
#include <QSet>
#include <QSharedPointer>
#include <QTimer>

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
class WSearchLineEdit;
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

    /// Connect the search box's returnPressed signal to our searchNow() slot
    /// so that YouTube searches only fire on Enter, not on keystroke debounce.
    void bindSearchboxWidget(WSearchLineEdit* pSearchboxWidget);

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
    void onRightClick(const QPoint& globalPos) override;

  private slots:
    void onSearchResultsReady(
            const QString& query, const QList<mixxx::YouTubeVideoInfo>& results);
    void onSearchFailed(const QString& query, const QString& error);
    void onDownloadFinished(const QString& videoId, const QString& localPath);
    void onDownloadFailed(const QString& videoId, const QString& error);
    void onTrackAnalysisProgress(TrackId trackId, AnalyzerProgress analyzerProgress);
    void slotCleanCache();
    /// Dispatch clicks on links rendered in the home pane HTML.
    /// `ytplay:VIDEOID`     → download/cache/analyze the track.
    /// `ytcached:LOCALPATH` → refresh the already-downloaded row.
    void onHomeAnchorClicked(const QUrl& url);
    /// Append additional results to the track table (used when the user
    /// scrolls to the bottom and fetchMore() fetches the next InnerTube page).
    void onSearchMoreReady(
            const QString& query, const QList<mixxx::YouTubeVideoInfo>& results);

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
    /// `attempt` is used internally to bound the deferred retries triggered
    /// when the write loses the SQLite lock to the library scanner.
    void replaceTrackTable(
            const QList<mixxx::YouTubeVideoInfo>& videos, int attempt = 0);
    /// Append rows for `videos` to `youtube_library` without first wiping
    /// the table. Used when InnerTube returns a continuation page so the
    /// user sees the new batch below the existing results instead of the
    /// whole list being replaced.
    void appendToTrackTable(const QList<mixxx::YouTubeVideoInfo>& videos);
    /// Append a single downloaded entry (or update its row to point at the
    /// real file path) so the "Downloaded" column reflects the new file
    /// without a full table rebuild.
    void upsertDownloadedRow(const QString& videoId,
            const QString& localPath,
            const QString& title,
            const QString& uploader,
            int durationSec);

    // ----- Cover art (YouTube thumbnails) -----

    /// Subdirectory of cacheDir() used for per-video thumbnail images.
    QString thumbnailDir() const;
    /// For each video in `videos`, download `hqdefault.jpg` from YouTube's
    /// image CDN to `thumbnailDir()/<videoId>.jpg` if not already present.
    /// On completion, fires a dataChanged() on the model so the cover-art
    /// delegate repaints the newly-available thumbnail.
    void fetchThumbnails(const QList<mixxx::YouTubeVideoInfo>& videos);
    /// Set of videoIds whose thumbnails are currently being downloaded.
    QSet<QString> m_thumbnailsDownloading;

    parented_ptr<TreeItemModel> m_pSidebarModel;
    QPointer<WLibraryTextBrowser> m_pHomeView;
    QPointer<WSearchLineEdit> m_pSearchbox;
    mixxx::YouTubeService m_service;
    /// Debounce timer for rebuildSidebar() + rebuildHomeHtml() — coalesces
    /// rapid-fire calls (e.g. during batch auto-analyze downloads) so the UI
    /// thread is not churning HTML on every individual download completion.
    QTimer* m_rebuildTimer = nullptr;
    /// When true, the next rebuildTimer fire also runs buildIndex()+select()
    /// to pick up rows updated by upsertDownloadedRow() or analysis completion.
    /// Set by those paths instead of calling buildIndex()+select() inline so N
    /// rapid completions share a single rebuild instead of N.
    bool m_pendingModelUpdate = false;
    /// Schedule a deferred rebuildSidebar() + rebuildHomeHtml(). Calling this
    /// multiple times within the debounce window triggers only one rebuild.
    void scheduleRebuild();
    /// Debounce timer for thumbnail dataChanged() notifications. Coalesces N
    /// simultaneous thumbnail arrivals into a single model repaint.
    QTimer* m_thumbnailTimer = nullptr;
    /// Set when at least one thumbnail arrived since the last repaint.
    bool m_thumbnailsDirty = false;
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
    /// Auto-discovered music genres from YouTube Music's browse API. Populated
    /// asynchronously on first activate(); falls back to kDefaultGenres when
    /// empty (fetch failed or hasn't completed yet).
    QStringList m_discoveredGenres;
    /// True while a trending fetch issued from activate() is in flight, so
    /// repeated activations don't fire duplicate trending requests. Cleared
    /// when results arrive, the fetch fails, or a user search supersedes it.
    bool m_trendingFetchInFlight = false;
    /// True when the most recent searchAndActivate() call came from the
    /// sidebar Samples section (kSampleQueryPrefix). When set, the first
    /// result from onSearchResultsReady() is auto-downloaded and loaded into
    /// the next available sampler slot rather than just displayed in the table.
    bool m_samplerTargetSearch = false;

    // ----- Samples section (DJ Tools + Greek Memes from myinstants.com) -----

    /// A sound scraped from myinstants.com (name + CDN MP3 URL).
    struct MyInstantSound {
        QString name;
        QString mp3Url; // full CDN URL: https://www.myinstants.com/media/sounds/…
    };

    /// Fetched list of Greek meme sounds. Populated async on first expand.
    QList<MyInstantSound> m_myInstantSounds;
    /// True while a myinstants.com fetch is in-flight.
    bool m_myInstantsFetchInFlight = false;
    /// QNetworkAccessManager used only for the Samples section (myinstants fetches + MP3
    /// downloads). Kept separate from YouTubeService's QNetworkAccessManager so cookie jars
    /// and rate-limiting for YouTube are not shared with an unrelated host.
    QNetworkAccessManager* m_pSamplesNam = nullptr;
    /// Set of myinstants CDN URLs currently being downloaded (prevent dupes).
    QSet<QString> m_myInstantsDownloading;

    /// Fetch the Greek myinstants.com page and populate m_myInstantSounds.
    void fetchMyInstantsSounds();
    /// Download a single myinstants MP3 to myInstantsCacheDir() and load it.
    void downloadMyInstant(const QString& mp3Url, const QString& displayName);
    /// Subdirectory of cacheDir() used for myinstants MP3 files.
    QString myInstantsCacheDir() const;
    /// Find the next empty sampler slot and emit loadTrackToPlayer() for it.
    /// Falls back to loadTrack() (next available deck) when all 32 slots are full.
    void loadTrackToNextSampler(const TrackPointer& pTrack);
};
