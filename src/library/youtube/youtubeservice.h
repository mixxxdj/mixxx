#pragma once

#include <QHash>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <functional>

class QNetworkAccessManager;
class QNetworkReply;
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
/// Uses two cooperating backends so the YouTube tab works out of the box
/// on every platform Mixxx ships on, with no external setup required:
///
///   1. **Piped HTTP** (primary, all platforms incl. Android). Piped is an
///      open-source, federated REST proxy in front of YouTube
///      (https://github.com/TeamPiped/Piped). We round-robin a hardcoded
///      list of known-good public instances; on per-request failure we
///      automatically failover to the next instance. Search is `/search`,
///      stream resolution is `/streams/<id>` which returns direct unsigned
///      googlevideo URLs that we then pull with QNetworkAccessManager.
///      Pure Qt over HTTPS — no native dependency, no bundled binary, no
///      JNI bridge.
///
2. **Bundled yt-dlp** (defense in depth on all platforms). On Linux/macOS
     /Windows the install layout ships the official self-contained yt-dlp
     PyInstaller binary. On Android we bundle the youtubedl-android AAR
     (Python 3.11 runtime + yt-dlp) and call it via JNI — no external
     dependencies, no Termux, no system Python needed. Used when Piped
     instances are unreachable or return no audio streams.
class YouTubeService : public QObject {
    Q_OBJECT
  public:
    explicit YouTubeService(QObject* parent = nullptr);

    /// Run a search; emits searchResultsReady(query, results) on completion.
    /// `cap` is the max number of results returned to the caller.
    void searchVideos(const QString& query, int cap = 25);

    /// Fetch country-specific trending music for the given ISO 3166-1 alpha-2
    /// country code (e.g. "US", "DE", "BR"). Results are surfaced via the
    /// existing searchResultsReady signal with `query` set to the sentinel
    /// kTrendingQueryPrefix + region — YouTubeFeature recognizes that
    /// prefix and renders a "Trending in <Country>" header instead of
    /// "Results for: ...". This keeps the signal surface small.
    void fetchTrending(const QString& region, int cap = 25);

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

    /// Absolute path to the yt-dlp binary (desktop) or "android-bundled"
    /// marker (Android, indicating the youtubedl-android JNI runtime).
    /// Empty if no runtime was found (desktop without bundled yt-dlp only).
    QString ytDlpPath() const {
        return m_ytDlpPath;
    }

  signals:
    void searchResultsReady(const QString& query, const QList<mixxx::YouTubeVideoInfo>& results);
    void searchFailed(const QString& query, const QString& error);
    void downloadFinished(const QString& videoId, const QString& localPath);
    void downloadFailed(const QString& videoId, const QString& error);
    void sponsorSegmentsFetched(
            const QString& videoId, const QList<mixxx::SponsorSegment>& segments);

  private:
    /// Locate the bundled (or user-installed) yt-dlp binary. Cached in
    /// m_ytDlpPath at construction.
    static QString locateYtDlp();

    // ----- Piped (primary) -----

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
            const std::function<void(const QString&)>& onFailure);

    // ----- yt-dlp (desktop fallback) -----

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

    QNetworkAccessManager* m_pNam;
    QString m_ytDlpPath;
    /// Hardcoded fallback list of Piped API instances. Tried in order on
    /// per-request failure. The list is intentionally small — going through
    /// every public instance on every search would amplify load and make
    /// failures slow to surface to the user.
    QStringList m_pipedInstances;
};

} // namespace mixxx
