#pragma once

#include <QElapsedTimer>
#include <QHash>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QRandomGenerator>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <functional>

class QNetworkAccessManager;
class QNetworkCookieJar;
class QNetworkReply;
class QNetworkRequest;
class QProcess;

namespace mixxx {

struct SponsorSegment {
    double start;
    double end;
    QString category;
};

struct YouTubeVideoInfo {
    QString id;
    QString title;
    QString uploader;
    int durationSec = 0;
    bool isLive = false;
};

/// YouTube extractor + downloader.
///
/// Uses three cooperating backends so the YouTube tab works out of the box
/// on every platform Mixxx ships on, with no external setup required:
///
///   1. **YouTube InnerTube API** (primary, all platforms incl. Android).
///      The same internal API used by yt-dlp and ytdlnis, hit directly with
///      no third-party proxy. Search/trending POST to
///      `https://www.youtube.com/youtubei/v1/search`; stream resolution POSTs
///      to `.../youtubei/v1/player` with a sequence of mobile/embedded client
///      contexts (ANDROID_VR, iOS, embedded TV), each of which can return
///      non-cipher adaptive stream URLs valid for ~6h. We fail over across
///      clients so a single throttled client does not break things. Because it
///      depends on no community-run servers, this is the most reliable path
///      and is therefore tried first. Pure Qt/HTTPS — no external deps.
///
///   2. **Piped HTTP** (fallback, all platforms incl. Android). Piped is an
///      open-source, federated REST proxy in front of YouTube
///      (https://github.com/TeamPiped/Piped). We round-robin a hardcoded
///      list of public instances; on per-request failure we failover to the
///      next instance. Used when InnerTube is unreachable. Piped community
///      instances are ephemeral, so this is a best-effort secondary path.
///
///   3. **Bundled yt-dlp** (last resort). Desktop: ships the official
///      self-contained PyInstaller binary. Android: bundles the
///      youtubedl-android AAR (Python 3.11 runtime + yt-dlp) and calls it via
///      JNI — no external dependencies, no Termux, no system Python needed.
///      Also works with Termux-installed yt-dlp on Android.
class YouTubeService : public QObject {
    Q_OBJECT
  public:
    explicit YouTubeService(QObject* parent = nullptr);

    /// Run a search; emits searchResultsReady(query, results) on completion.
    /// `cap` is the max number of results returned to the caller.
    void searchVideos(const QString& query, int cap = 25);

    /// Fetch the next page of results for the most recent search() call.
    /// Uses the InnerTube continuation token stored from the last successful
    /// search response. Emits searchMoreReady(query, results) on success, or
    /// does nothing when no continuation is available or bot-flagged.
    /// The feature calls this from YouTubeTrackModel::fetchMore().
    void fetchMoreSearchResults(const QString& emittedQuery, int cap = 25);

    /// True when the last InnerTube search response included a continuation
    /// token, i.e. there are more results the user can scroll to.
    bool hasMoreSearchResults() const {
        return !m_searchContinuationToken.isEmpty();
    }

    /// Fetch country-specific trending music for the given ISO 3166-1 alpha-2
    /// country code (e.g. "US", "DE", "BR"). Results are surfaced via the
    /// existing searchResultsReady signal with `query` set to the sentinel
    /// kTrendingQueryPrefix + region — YouTubeFeature recognizes that
    /// prefix and renders a "Trending in <Country>" header instead of
    /// "Results for: ...". This keeps the signal surface small.
    void fetchTrending(const QString& region, int cap = 25);

    /// Fetch music genres/moods from YouTube Music's browse API for the given
    /// region. Emits genresReady(genres) with a list of genre display names
    /// (localized to the region, e.g. Greek genre names for "GR"). Falls back
    /// to a hardcoded default list on failure.
    void fetchMusicGenres(const QString& region);

    /// Sentinel `query` value prefix used for trending results. Anything
    /// emitted on searchResultsReady whose query starts with this prefix is
    /// the trending feed for the region given after the colon.
    static const QString kTrendingQueryPrefix;

    /// Download `videoId` to `cacheDir`. Picks the best audio-only stream
    /// (typically opus-in-webm or aac-in-m4a), writes it to
    /// `<cacheDir>/<videoId>.<ext>`. Emits downloadFinished(videoId, path)
    /// or downloadFailed(videoId, error).
    void downloadVideo(const QString& videoId, const QString& cacheDir);

    /// Fetch SponsorBlock segments for the given videoId from the public
    /// SponsorBlock API at sponsor.ajay.app.
    void fetchSponsorSegments(const QString& videoId);

    /// Absolute path to the yt-dlp binary (desktop/Termux), "android-bundled"
    /// marker (Android with HAVE_YTDLP_ANDROID compiled in), or empty string
    /// when no yt-dlp runtime is available. The InnerTube backend is used
    /// regardless of this value; yt-dlp is only a last-resort fallback.
    QString ytDlpPath() const {
        return m_ytDlpPath;
    }

  signals:
    void searchResultsReady(const QString& query, const QList<mixxx::YouTubeVideoInfo>& results);
    /// Emitted when an InnerTube continuation fetch returns more results.
    /// The feature appends these to the existing track table (vs replacing
    /// for a fresh search). The `query` is the same emittedQuery passed to
    /// fetchMoreSearchResults().
    void searchMoreReady(const QString& query, const QList<mixxx::YouTubeVideoInfo>& results);
    void searchFailed(const QString& query, const QString& error);
    void downloadFinished(const QString& videoId, const QString& localPath);
    void downloadFailed(const QString& videoId, const QString& error);
    void sponsorSegmentsFetched(
            const QString& videoId, const QList<mixxx::SponsorSegment>& segments);
    /// Emitted when YouTube returns a response indicating automated access
    /// detection (HTTP 429, LOGIN_REQUIRED, consent challenge). The UI can
    /// surface this as a non-fatal warning so the user knows why results are
    /// degraded or downloads are falling back to yt-dlp.
    void botFlagged(const QString& detail);
    /// Emitted when genre auto-discovery completes. The list contains display
    /// names (potentially localized, e.g. Greek genre names for region "GR").
    void genresReady(const QStringList& genres);

  private:
    /// Locate the bundled (or user-installed) yt-dlp binary. Cached in
    /// m_ytDlpPath at construction.
    static QString locateYtDlp();

    // ----- InnerTube search (primary) -----

    /// Try a search against the YouTube InnerTube /search endpoint using
    /// `m_innerTubeClients[clientIdx]` (indexed into the internal client
    /// table). On network/parse failure — or a 200 with no parseable videos —
    /// recurses to the next client. On success emits searchResultsReady(
    /// emittedQuery, results). When every client is exhausted, calls
    /// `onAllFailed(lastError)` so the caller can fall through to Piped/yt-dlp.
    /// `requestQuery` is the text actually sent to YouTube; `emittedQuery` is
    /// echoed back on searchResultsReady (these differ for trending, where the
    /// emitted value is the kTrendingQueryPrefix sentinel).
    void searchViaInnerTube(const QString& emittedQuery,
            const QString& requestQuery,
            int cap,
            int clientIdx,
            const std::function<void(const QString& lastError)>& onAllFailed,
            const QString& regionOverride = QString());

    // ----- Piped (search/download fallback) -----

    /// Try a Piped search against `m_pipedInstances[instanceIdx]`. On
    /// network/parse failure, recurses to the next instance. When all
    /// instances are exhausted, calls `onAllFailed(lastError)` so the
    /// public searchVideos() can decide whether to fall through to yt-dlp.
    void searchViaPiped(const QString& query,
            int cap,
            int instanceIdx,
            const std::function<void(const QString& lastError)>& onAllFailed);

    void searchViaPipedWithFilter(const QString& emittedQuery,
            const QString& requestQuery,
            const QString& filter,
            int cap,
            int instanceIdx,
            const std::function<void(const QString& lastError)>& onAllFailed);
    void fetchNextPipedSearchPage(const QString& emittedQuery,
            const QString& requestQuery,
            const QString& filter,
            int cap,
            int instanceIdx,
            const QString& nextPage,
            QList<mixxx::YouTubeVideoInfo> accumulated,
            int pageCount,
            const std::function<void(const QString& lastError)>& onAllFailed);

    /// Try YouTube Music's Songs category for the selected country against
    /// `m_pipedInstances[instanceIdx]`. On per-instance failure, recurses to
    /// the next one. Emits searchResultsReady(kTrendingQueryPrefix + region,
    /// results) on success; emits searchFailed(...) once every instance is
    /// exhausted.
    void fetchTrendingViaPiped(const QString& region, int cap, int instanceIdx);

    /// Try a Piped resolve+download against `m_pipedInstances[instanceIdx]`.
    /// Same failover semantics as searchViaPiped.
    void downloadViaPiped(const QString& videoId,
            const QString& cacheDir,
            int instanceIdx,
            const std::function<void(const QString& lastError)>& onAllFailed);

    /// Pick the best audio stream from a Piped /streams response and start
    /// streaming it to `<cacheDir>/<videoId>.<ext>`. Emits downloadFinished
    /// (after the SponsorBlock chain) or invokes `onFailure` on error so
    /// the caller can try the next Piped instance.
    void downloadAudioStream(const QString& videoId,
            const QString& cacheDir,
            const QJsonArray& audioStreams,
            const std::function<void(const QString&)>& onFailure,
            const QString& streamUserAgent = QString());

    /// Try the YouTube InnerTube player API as a secondary download backend.
    /// POSTs to /youtubei/v1/player with a sequence of mobile/embedded client
    /// contexts (Android, iOS, ...), which return non-cipher stream URLs.
    /// Works on all platforms including Android. Called when all Piped
    /// instances fail; invokes onAllFailed when every client fails to produce
    /// a usable stream.
    void downloadViaInnerTube(const QString& videoId,
            const QString& cacheDir,
            const std::function<void(const QString& lastError)>& onAllFailed);

    /// Attempt a single InnerTube client (indexed into the internal client
    /// table). On per-client failure — request error, unplayable status, no
    /// audio streams, or a failed stream download — recurses to the next
    /// client. When every client is exhausted, calls onAllFailed(lastError).
    void downloadViaInnerTubeClient(const QString& videoId,
            const QString& cacheDir,
            int clientIdx,
            const std::function<void(const QString& lastError)>& onAllFailed);

    // ----- yt-dlp (binary fallback — desktop and Termux-on-Android) -----

    /// Spawn yt-dlp with `args`; on completion call `onSuccess(stdout)` or
    /// `onFailure(message)`. `timeoutMs` is enforced via QTimer; processes
    /// that exceed it are killed and surfaced as a failure. The QProcess is
    /// parented to `this` and deletes itself on finish.
    void runYtDlp(const QStringList& args,
            int timeoutMs,
            const std::function<void(const QByteArray&)>& onSuccess,
            const std::function<void(const QString&)>& onFailure);

    void searchViaYtDlp(const QString& query, int cap);
    void downloadViaYtDlp(const QString& videoId, const QString& cacheDir);

    /// Attempt to self-update the bundled yt-dlp binary on desktop. This runs
    /// `yt-dlp --update-to stable` in the background at most once per process
    /// lifetime. A failed update is non-fatal (we proceed with the existing
    /// binary). Called lazily on first yt-dlp download attempt.
    void maybeUpdateDesktopYtDlp();

#if defined(Q_OS_ANDROID) && defined(HAVE_YTDLP_ANDROID)
    // ----- bundled youtubedl-android (Android only, no external deps) -----

    /// Download using the bundled youtubedl-android runtime via JNI.
    /// This is the primary fallback on Android when Piped fails.
    /// No external Python, Termux, or system dependency needed.
    void downloadViaAndroidBundled(const QString& videoId, const QString& cacheDir);
#endif

    // ----- shared -----

    /// Internal SponsorBlock fetch used to chain "download → fetch → cut".
    /// The callback receives segments (possibly empty on failure).
    void fetchSponsorSegmentsInternal(
            const QString& videoId,
            const std::function<void(const QList<SponsorSegment>&)>& cb);

    /// After a downloaded file is on disk, run SponsorBlock fetch, optionally
    /// cut segments in-place, then emit downloadFinished(videoId, outPath).
    void finalizeDownload(const QString& videoId, const QString& outPath);

    // ----- bot-detection & rate limiting -----

    /// Inspect an InnerTube player/search HTTP response for signs that YouTube
    /// has flagged us as a bot. Returns true (and emits botFlagged) when any
    /// of the following are detected:
    ///   - HTTP 429 (Too Many Requests)
    ///   - playabilityStatus.status == "LOGIN_REQUIRED"
    ///   - playabilityStatus.reason contains "Sign in" / "bot" / "unusual"
    ///   - Response body contains a consent/captcha challenge page
    /// When true, callers should skip directly to yt-dlp rather than cycling
    /// through remaining InnerTube/Piped clients (which will all be flagged on
    /// the same IP).
    bool detectBotFlagging(int httpStatus,
            const QJsonObject& root,
            const QByteArray& rawBody);

    /// Per-request rate limiter: enforces a minimum inter-request delay to
    /// YouTube endpoints to reduce the probability of triggering bot detection.
    /// Returns true if the caller should NOT proceed (throttled).
    bool shouldThrottleRequest();

    /// Record that a request was just sent (updates the rate-limiter state).
    void recordRequest();

    /// Randomized delay (jitter) to add between requests. Returns a random
    /// value in [minMs, maxMs] to make request timing look more human.
    int jitterDelayMs() const;

    /// Called when a bot flag is detected. Sets the flag, starts the cooldown
    /// timer, and emits the signal. Subsequent calls extend the backoff.
    void activateBotFlag(const QString& detail);

    /// Checks if the bot-flag cooldown has expired and resets the flag.
    /// Called before each request attempt to allow auto-recovery.
    void maybeRecoverFromBotFlag();

    /// Persist visitor data + session state to QSettings so it survives restarts.
    void saveSessionState();
    /// Load persisted session state from QSettings on construction.
    void loadSessionState();

    /// Build yt-dlp args with anti-bot options (cookies, extractor-args, etc.)
    QStringList ytDlpAntiBotArgs() const;

    /// Get a randomized starting client index for client rotation.
    int randomClientStartIndex(int clientCount) const;

    /// Apply realistic browser-like headers to a request (beyond the basic UA).
    void applyBrowserFingerprint(QNetworkRequest* req,
            const char* clientNameId) const;

    /// Initialize and persist a cookie jar that outlives individual requests.
    void setupCookieJar();

    QNetworkAccessManager* m_pNam;
    QString m_ytDlpPath;
    /// Continuation token from the most recent InnerTube /search response.
    /// Non-empty when there are more results to fetch via fetchMoreSearchResults().
    /// Cleared at the start of every new searchVideos() call.
    QString m_searchContinuationToken;
    /// Hardcoded fallback list of Piped API instances. Tried in order on
    /// per-request failure. The expanded list provides better resilience
    /// against community-maintained instances going offline.
    QStringList m_pipedInstances;

    // Rate-limiting state: track timestamps of recent InnerTube requests to
    // self-throttle and avoid triggering YouTube's bot detection. The window
    // and burst limits are conservative defaults tuned to avoid 429s while
    // still allowing snappy search/download workflows.
    QMutex m_rateLimitMutex;
    QList<qint64> m_requestTimestamps; // msecsSinceEpoch of recent requests
    QElapsedTimer m_rateLimitTimer;

    // Bot-flag state with exponential backoff cooldown.
    bool m_botFlagActive = false;
    int m_botFlagBackoffMs = 0;    // current backoff duration
    qint64 m_botFlagTimestamp = 0; // when the flag was last set (elapsed ms)
    int m_botFlagCount = 0;        // consecutive bot-flag events this session

    /// Visitor data token obtained from InnerTube responses. YouTube returns a
    /// visitorData field in some responses; echoing it back reduces bot
    /// detection likelihood by maintaining session continuity.
    QString m_visitorData;

    /// Persistent cookie jar path for YouTube session cookies.
    QString m_cookieJarPath;

    // ----- SponsorBlock prefetch cache -----

    /// Segments that were prefetched while the audio download was in flight.
    /// Keyed by videoId. Entries are consumed once by finalizeDownload() and
    /// then removed so the map stays small.
    QHash<QString, QList<SponsorSegment>> m_sponsorPrefetchCache;

    /// Callbacks registered by finalizeDownload() that are waiting for a
    /// SponsorBlock prefetch to complete. When the prefetch callback fires it
    /// calls all registered waiters and removes the entry.
    QHash<QString, QList<std::function<void(const QList<SponsorSegment>&)>>>
            m_sponsorPrefetchWaiters;
};

} // namespace mixxx
