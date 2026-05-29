#include "library/youtube/youtubeservice.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <utility>

#include "library/youtube/youtubeaudiocutter.h"
#include "util/logger.h"

namespace mixxx {

namespace {
const Logger kLogger("YouTubeService");

// Per-request budgets. Search is bounded — Piped's /search and yt-dlp's
// page-parse both finish well under a second on a healthy network. Downloads
// can be larger, but bestaudio for a song is typically <10 MB; the ceiling
// is generous so a slow connection or long mix doesn't get killed.
constexpr int kSearchTimeoutMs = 30 * 1000;        // 30 s
constexpr int kDownloadTimeoutMs = 10 * 60 * 1000; // 10 min
// Piped HTTP requests get a tighter ceiling — each instance failure should
// surface fast so we can fail over to the next one without burning too long
// per dead instance. With 10 instances at 10s each, worst case is ~100s.
constexpr int kPipedHttpTimeoutMs = 10 * 1000;
constexpr int kMaxPipedSearchPages = 5;
// SponsorBlock API timeout — prevents indefinite hangs if the service is slow.
constexpr int kSponsorBlockTimeoutMs = 8 * 1000;
// Project default for this fork: the requested first-open YouTube feed is Greek
// top songs, not generic global/United States YouTube trends.
const QString kDefaultRegion = QStringLiteral("GR");

// Hardcoded list of Piped API instances tried in order on per-request
// failure. Picked from the official Piped instance list
// (https://github.com/TeamPiped/documentation/blob/main/content/docs/public-instances/index.md)
// for geographic spread + uptime track record. More instances = better
// resilience when individual community-maintained servers go down.
const QStringList kPipedInstances = {
        QStringLiteral("https://pipedapi.kavin.rocks"),
        QStringLiteral("https://api.piped.private.coffee"),
        QStringLiteral("https://pipedapi.r4fo.com"),
        QStringLiteral("https://api.piped.projectsegfau.lt"),
        QStringLiteral("https://pipedapi.adminforge.de"),
        QStringLiteral("https://pipedapi.darkness.services"),
        QStringLiteral("https://api.piped.yt"),
        QStringLiteral("https://pipedapi.osphost.fi"),
        QStringLiteral("https://pipedapi.smnz.de"),
        QStringLiteral("https://pipedapi.ngn.tf"),
};

// Locations to probe for yt-dlp when it is not on PATH. Order matters:
// the bundled binary next to the Mixxx executable wins, then the user's
// PATH, then common install dirs that GUI launches often miss because
// /opt/homebrew/bin and ~/.local/bin are outside the inherited PATH.
QStringList ytDlpFallbackBins() {
    QStringList bins;
#if defined(Q_OS_WIN)
    const QString exe = QStringLiteral("yt-dlp.exe");
#else
    const QString exe = QStringLiteral("yt-dlp");
#endif
    // 1. Bundled next to the Mixxx executable (Win/Mac/Linux desktop installs
    //    ship the official self-contained PyInstaller binary here — see
    //    cmake/modules/FetchYtDlp.cmake).
    const QString appDir = QCoreApplication::applicationDirPath();
    if (!appDir.isEmpty()) {
        bins << QDir(appDir).filePath(exe);
#if defined(Q_OS_MAC)
        // .app bundle layout: yt-dlp lives in Contents/MacOS alongside the
        // Mixxx binary, but applicationDirPath() already returns that path.
        // Also probe Contents/Resources just in case.
        bins << QDir(appDir).filePath(QStringLiteral("../Resources/") + exe);
#endif
    }
    // 2. Common system locations.
    bins << QStringLiteral("/usr/local/bin/yt-dlp")
         << QStringLiteral("/usr/bin/yt-dlp")
         << QStringLiteral("/opt/homebrew/bin/yt-dlp")
         << QStringLiteral("/opt/local/bin/yt-dlp")
         // Termux-on-Android: only path that ever yields a working yt-dlp
         // on Android, and only if the user has installed it themselves.
         // Piped is the primary Android backend so this is purely a bonus.
         << QStringLiteral("/data/data/com.termux/files/usr/bin/yt-dlp");
    return bins;
}

// Map a Piped audio stream's ext-hint (mimeType) to a sensible file
// extension for the downloaded blob. We name files <id>.<ext> so the rest
// of YouTubeFeature (and the SoundSource picker) can identify them.
QString extFromMime(const QString& mime, const QString& codec) {
    const QString m = mime.toLower();
    if (m.contains(QStringLiteral("webm"))) {
        return QStringLiteral("webm");
    }
    if (m.contains(QStringLiteral("mp4")) || m.contains(QStringLiteral("m4a"))) {
        return QStringLiteral("m4a");
    }
    if (codec.startsWith(QStringLiteral("opus"))) {
        return QStringLiteral("webm");
    }
    if (codec.startsWith(QStringLiteral("mp4a"))) {
        return QStringLiteral("m4a");
    }
    return QStringLiteral("m4a"); // safe default — FFmpeg SoundSource handles it
}

bool isPipedLiveStream(const QJsonObject& obj) {
    if (obj.value(QStringLiteral("isLive")).toBool(false)) {
        return true;
    }
    const QJsonValue duration = obj.value(QStringLiteral("duration"));
    return !duration.isUndefined() && duration.isDouble() && duration.toDouble() <= 0;
}

bool isValidYouTubeVideoId(const QString& videoId) {
    static const QRegularExpression kVideoIdPattern(
            QStringLiteral(R"(^[A-Za-z0-9_-]{11}$)"));
    return kVideoIdPattern.match(videoId).hasMatch();
}

bool isYtDlpLiveStream(const QJsonObject& obj) {
    if (obj.value(QStringLiteral("is_live")).toBool(false)) {
        return true;
    }
    const QString liveStatus =
            obj.value(QStringLiteral("live_status")).toString().toLower();
    if (liveStatus == QStringLiteral("is_live") ||
            liveStatus == QStringLiteral("is_upcoming")) {
        return true;
    }
    const QJsonValue duration = obj.value(QStringLiteral("duration"));
    return !duration.isUndefined() && duration.isDouble() && duration.toDouble() <= 0;
}

QString countryDisplayName(const QString& code) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const QLocale forCountry(QStringLiteral("en_") + code);
    const QString name = QLocale::countryToString(forCountry.country());
#else
    const QLocale forCountry(QLocale::AnyLanguage,
            QLocale::codeToTerritory(code));
    const QString name = QLocale::territoryToString(forCountry.territory());
#endif
    return name.isEmpty() ? code : name;
}

QString countryTopSongsCategoryQuery(const QString& region) {
    // Piped exposes YouTube Music's Songs category via `filter=music_songs`,
    // but the endpoint still expects a short category seed. Use the local music
    // adjective where it matters for this fork's default region so the feed is
    // Greek songs, not English-language videos about Greece.
    if (region.compare(QStringLiteral("GR"), Qt::CaseInsensitive) == 0) {
        return QObject::tr("Greek top songs");
    }
    return QObject::tr("%1 top songs").arg(countryDisplayName(region));
}

void appendUniqueVideos(
        QList<YouTubeVideoInfo>* accumulated,
        const QList<YouTubeVideoInfo>& pageResults,
        int cap) {
    QSet<QString> seen;
    for (const auto& info : std::as_const(*accumulated)) {
        seen.insert(info.id);
    }
    for (const auto& info : pageResults) {
        if (accumulated->size() >= cap) {
            return;
        }
        if (seen.contains(info.id)) {
            continue;
        }
        accumulated->append(info);
        seen.insert(info.id);
    }
}

// Parse a Piped JSON array of stream items (the same `items[]` shape returned
// by /search and the top-level array returned by /trending) into our internal
// YouTubeVideoInfo list, capped at `cap`. Non-stream entries (channels,
// playlists), live/current streams and items with missing id/title are skipped.
// Factored out so the trending and search response paths share one parser.
QList<YouTubeVideoInfo> parsePipedItems(const QJsonArray& items, int cap) {
    QList<YouTubeVideoInfo> results;
    results.reserve(qMin(items.size(), cap));
    for (const QJsonValue& v : items) {
        if (results.size() >= cap) {
            break;
        }
        const QJsonObject obj = v.toObject();
        const QString type = obj.value(QStringLiteral("type")).toString();
        if (!type.isEmpty() && type != QStringLiteral("stream")) {
            continue;
        }
        if (isPipedLiveStream(obj)) {
            continue;
        }
        YouTubeVideoInfo info;
        // "url" is a relative "/watch?v=ID" path. Strip the prefix to recover
        // the bare videoId for /streams/<id> lookup.
        const QString relUrl = obj.value(QStringLiteral("url")).toString();
        const int eq = relUrl.indexOf(QLatin1Char('='));
        if (eq > 0 && eq + 1 < relUrl.size()) {
            info.id = relUrl.mid(eq + 1);
        }
        info.title = obj.value(QStringLiteral("title")).toString();
        info.uploader = obj.value(QStringLiteral("uploaderName")).toString();
        const QJsonValue dur = obj.value(QStringLiteral("duration"));
        if (dur.isDouble()) {
            info.durationSec = static_cast<int>(dur.toDouble());
        }
        info.isLive = false;
        if (isValidYouTubeVideoId(info.id) && !info.title.isEmpty()) {
            results.append(info);
        }
    }
    return results;
}
} // namespace

const QString YouTubeService::kTrendingQueryPrefix =
        QStringLiteral("__trending__:");

YouTubeService::YouTubeService(QObject* parent)
        : QObject(parent),
          m_pNam(new QNetworkAccessManager(this)),
          m_ytDlpPath(locateYtDlp()),
          m_pipedInstances(kPipedInstances) {
    if (!m_ytDlpPath.isEmpty()) {
        kLogger.info() << "yt-dlp fallback available at" << m_ytDlpPath;
    } else {
        kLogger.info() << "yt-dlp not found; YouTube tab will rely on Piped only "
                          "(this is normal on Android and minimal desktop installs)";
    }
}

QString YouTubeService::locateYtDlp() {
    // 1. Explicit override — useful for portable installs and for users on
    //    macOS GUI launches where the inherited PATH lacks /opt/homebrew.
    const QByteArray envPath = qgetenv("MIXXX_YTDLP");
    if (!envPath.isEmpty()) {
        const QString p = QString::fromLocal8Bit(envPath);
        if (QFileInfo(p).isExecutable()) {
            return p;
        }
        kLogger.warning() << "MIXXX_YTDLP set to" << p
                          << "but that path is not executable; ignoring";
    }
    // 2. Bundled-next-to-binary + common install dirs.
    const QStringList fallbackBins = ytDlpFallbackBins();
    for (const QString& candidate : std::as_const(fallbackBins)) {
        if (QFileInfo(candidate).isExecutable()) {
            return candidate;
        }
    }
    // 3. PATH lookup — handles "yt-dlp" and "yt-dlp.exe" on Windows.
    const QString fromPath =
            QStandardPaths::findExecutable(QStringLiteral("yt-dlp"));
    if (!fromPath.isEmpty()) {
        return fromPath;
    }
    return QString();
}

// =============================================================================
// Public API — picks Piped first, yt-dlp as desktop fallback
// =============================================================================

void YouTubeService::searchVideos(const QString& query, int cap) {
    if (query.trimmed().isEmpty()) {
        Q_EMIT searchResultsReady(query, {});
        return;
    }
    const bool hasYtDlpFallback = !m_ytDlpPath.isEmpty();
    searchViaPiped(query,
            cap,
            /*instanceIdx=*/0,
            [this, query, cap, hasYtDlpFallback](const QString& lastError) {
                if (hasYtDlpFallback) {
                    kLogger.warning() << "All Piped instances failed for search"
                                      << query << ":" << lastError
                                      << "— falling back to yt-dlp";
                    searchViaYtDlp(query, cap);
                } else {
                    kLogger.warning() << "All Piped instances failed for search"
                                      << query << ":" << lastError;
                    Q_EMIT searchFailed(query, lastError);
                }
            });
}

void YouTubeService::downloadVideo(const QString& videoId, const QString& cacheDir) {
    if (!isValidYouTubeVideoId(videoId)) {
        Q_EMIT downloadFailed(videoId, tr("Invalid YouTube video id"));
        return;
    }
    QDir().mkpath(cacheDir);
    const bool hasYtDlpFallback = !m_ytDlpPath.isEmpty();
    downloadViaPiped(videoId,
            cacheDir,
            /*instanceIdx=*/0,
            [this, videoId, cacheDir, hasYtDlpFallback](const QString& lastError) {
                if (hasYtDlpFallback) {
                    kLogger.warning() << "All Piped instances failed for download"
                                      << videoId << ":" << lastError
                                      << "— falling back to yt-dlp";
                    downloadViaYtDlp(videoId, cacheDir);
                } else {
                    kLogger.warning() << "All Piped instances failed for download"
                                      << videoId << ":" << lastError;
                    Q_EMIT downloadFailed(videoId, lastError);
                }
            });
}

void YouTubeService::fetchTrending(const QString& region, int cap) {
    // Empty / malformed region: fall back to the project default ("GR") rather
    // than failing, because an empty pane is the worst possible UX for a
    // freshly-opened YouTube tab.
    QString r = region.toUpper();
    const bool valid = r.size() == 2 && r.at(0).isLetter() && r.at(1).isLetter();
    if (!valid) {
        kLogger.warning() << "fetchTrending: ignoring unsupported region"
                          << region << "— defaulting to GR";
        r = kDefaultRegion;
    }
    fetchTrendingViaPiped(r, cap, /*instanceIdx=*/0);
}

// =============================================================================
// Piped (primary backend, works on every Qt platform incl. Android)
// =============================================================================

void YouTubeService::searchViaPiped(const QString& query,
        int cap,
        int instanceIdx,
        const std::function<void(const QString&)>& onAllFailed) {
    // Try music_songs first for higher-quality results, then fall back to
    // music_videos, then all videos if the specific filters find nothing.
    // This ensures any song can be found regardless of YouTube's internal
    // categorization.
    searchViaPipedWithFilter(query,
            query,
            QStringLiteral("music_songs"),
            cap,
            instanceIdx,
            [this, query, cap, instanceIdx, onAllFailed](const QString& /*lastError*/) {
                // music_songs failed on all instances — try music_videos
                kLogger.info() << "music_songs filter failed for" << query
                               << "— trying music_videos";
                searchViaPipedWithFilter(query,
                        query,
                        QStringLiteral("music_videos"),
                        cap,
                        /*instanceIdx=*/0,
                        [this, query, cap, onAllFailed](const QString& /*lastError2*/) {
                            // music_videos also failed — try unfiltered "videos"
                            kLogger.info() << "music_videos filter failed for"
                                           << query << "— trying videos";
                            searchViaPipedWithFilter(query,
                                    query,
                                    QStringLiteral("videos"),
                                    cap,
                                    /*instanceIdx=*/0,
                                    onAllFailed);
                        });
            });
}

void YouTubeService::searchViaPipedWithFilter(const QString& emittedQuery,
        const QString& requestQuery,
        const QString& filter,
        int cap,
        int instanceIdx,
        const std::function<void(const QString&)>& onAllFailed) {
    if (instanceIdx >= m_pipedInstances.size()) {
        onAllFailed(tr("All Piped instances failed (network or upstream YouTube error)"));
        return;
    }
    const QString instance = m_pipedInstances.at(instanceIdx);
    QUrl url(instance + QStringLiteral("/search"));
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("q"), requestQuery);
    q.addQueryItem(QStringLiteral("filter"), filter);
    url.setQuery(q);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "Mixxx/YouTube");
    req.setRawHeader("Accept", "application/json");
    req.setTransferTimeout(kPipedHttpTimeoutMs);

    QNetworkReply* reply = m_pNam->get(req);
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this,
                    reply,
                    emittedQuery,
                    requestQuery,
                    filter,
                    cap,
                    instanceIdx,
                    instance,
                    onAllFailed]() {
                reply->deleteLater();
                if (reply->error() != QNetworkReply::NoError) {
                    kLogger.info() << "Piped search via" << instance << "failed:"
                                   << reply->errorString() << "— trying next";
                    searchViaPipedWithFilter(emittedQuery,
                            requestQuery,
                            filter,
                            cap,
                            instanceIdx + 1,
                            onAllFailed);
                    return;
                }
                const QJsonDocument doc =
                        QJsonDocument::fromJson(reply->readAll());
                const QJsonObject root = doc.object();
                const QJsonArray items = root.value(QStringLiteral("items")).toArray();
                QList<YouTubeVideoInfo> results = parsePipedItems(items, cap);
                if (results.isEmpty()) {
                    // Empty result set is a legitimate "no matches" answer
                    // for a unique query, but for popular queries it almost
                    // always means the instance is degraded. Try the next.
                    kLogger.info() << "Piped instance" << instance
                                   << "returned 0 results for" << requestQuery
                                   << "— trying next";
                    searchViaPipedWithFilter(emittedQuery,
                            requestQuery,
                            filter,
                            cap,
                            instanceIdx + 1,
                            onAllFailed);
                    return;
                }
                const QString nextPage =
                        root.value(QStringLiteral("nextpage")).toString();
                fetchNextPipedSearchPage(emittedQuery,
                        requestQuery,
                        filter,
                        cap,
                        instanceIdx,
                        nextPage,
                        results,
                        1,
                        onAllFailed);
            });
}

void YouTubeService::fetchNextPipedSearchPage(const QString& emittedQuery,
        const QString& requestQuery,
        const QString& filter,
        int cap,
        int instanceIdx,
        const QString& nextPage,
        QList<mixxx::YouTubeVideoInfo> accumulated,
        int pageCount,
        const std::function<void(const QString&)>& onAllFailed) {
    if (accumulated.size() >= cap || nextPage.isEmpty() ||
            pageCount >= kMaxPipedSearchPages) {
        kLogger.info() << "Piped returned" << accumulated.size()
                       << "results for" << requestQuery << "after"
                       << pageCount << "page(s)";
        Q_EMIT searchResultsReady(emittedQuery, accumulated);
        return;
    }

    if (instanceIdx >= m_pipedInstances.size()) {
        if (!accumulated.isEmpty()) {
            Q_EMIT searchResultsReady(emittedQuery, accumulated);
        } else {
            onAllFailed(tr("All Piped instances failed (network or upstream YouTube error)"));
        }
        return;
    }

    const QString instance = m_pipedInstances.at(instanceIdx);
    QUrl url(instance + QStringLiteral("/nextpage/search"));
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("q"), requestQuery);
    q.addQueryItem(QStringLiteral("filter"), filter);
    q.addQueryItem(QStringLiteral("nextpage"), nextPage);
    url.setQuery(q);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "Mixxx/YouTube");
    req.setRawHeader("Accept", "application/json");
    req.setTransferTimeout(kPipedHttpTimeoutMs);

    QNetworkReply* reply = m_pNam->get(req);
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this,
                    reply,
                    emittedQuery,
                    requestQuery,
                    filter,
                    cap,
                    instanceIdx,
                    accumulated,
                    pageCount,
                    onAllFailed]() mutable {
                reply->deleteLater();
                if (reply->error() != QNetworkReply::NoError) {
                    kLogger.info()
                            << "Piped next search page failed for" << requestQuery
                            << ":" << reply->errorString()
                            << "— using accumulated results";
                    Q_EMIT searchResultsReady(emittedQuery, accumulated);
                    return;
                }
                const QJsonObject root =
                        QJsonDocument::fromJson(reply->readAll()).object();
                appendUniqueVideos(&accumulated,
                        parsePipedItems(
                                root.value(QStringLiteral("items")).toArray(),
                                cap),
                        cap);
                fetchNextPipedSearchPage(emittedQuery,
                        requestQuery,
                        filter,
                        cap,
                        instanceIdx,
                        root.value(QStringLiteral("nextpage")).toString(),
                        accumulated,
                        pageCount + 1,
                        onAllFailed);
            });
}

void YouTubeService::fetchTrendingViaPiped(
        const QString& region, int cap, int instanceIdx) {
    const QString sentinelQuery = kTrendingQueryPrefix + region;
    const QString requestQuery = countryTopSongsCategoryQuery(region);
    searchViaPipedWithFilter(sentinelQuery,
            requestQuery,
            QStringLiteral("music_songs"),
            cap,
            instanceIdx,
            [this, sentinelQuery, region](const QString& lastError) {
                Q_EMIT searchFailed(sentinelQuery,
                        tr("Could not load trending music for %1: %2")
                                .arg(countryDisplayName(region), lastError));
            });
}

void YouTubeService::downloadViaPiped(const QString& videoId,
        const QString& cacheDir,
        int instanceIdx,
        const std::function<void(const QString&)>& onAllFailed) {
    if (instanceIdx >= m_pipedInstances.size()) {
        onAllFailed(tr("All Piped instances failed for video %1").arg(videoId));
        return;
    }
    const QString instance = m_pipedInstances.at(instanceIdx);
    const QUrl url(instance + QStringLiteral("/streams/") + videoId);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "Mixxx/YouTube");
    req.setRawHeader("Accept", "application/json");
    req.setTransferTimeout(kPipedHttpTimeoutMs);

    QNetworkReply* reply = m_pNam->get(req);
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, videoId, cacheDir, instanceIdx, instance, onAllFailed]() {
                reply->deleteLater();
                if (reply->error() != QNetworkReply::NoError) {
                    kLogger.info() << "Piped /streams via" << instance << "failed:"
                                   << reply->errorString() << "— trying next";
                    downloadViaPiped(
                            videoId, cacheDir, instanceIdx + 1, onAllFailed);
                    return;
                }
                const QJsonObject root =
                        QJsonDocument::fromJson(reply->readAll()).object();
                const QJsonArray audio =
                        root.value(QStringLiteral("audioStreams")).toArray();
                if (audio.isEmpty()) {
                    kLogger.info() << "Piped instance" << instance
                                   << "returned no audioStreams for" << videoId
                                   << "— trying next";
                    downloadViaPiped(
                            videoId, cacheDir, instanceIdx + 1, onAllFailed);
                    return;
                }
                downloadAudioStream(videoId,
                        cacheDir,
                        audio,
                        [this, videoId, cacheDir, instanceIdx, onAllFailed](
                                const QString& err) {
                            kLogger.info() << "Piped audio download failed:"
                                           << err << "— trying next instance";
                            downloadViaPiped(videoId,
                                    cacheDir,
                                    instanceIdx + 1,
                                    onAllFailed);
                        });
            });
}

void YouTubeService::downloadAudioStream(const QString& videoId,
        const QString& cacheDir,
        const QJsonArray& audioStreams,
        const std::function<void(const QString&)>& onFailure) {
    // Pick the highest-bitrate M4A/AAC stream first because Mixxx already
    // supports this container everywhere the YouTube feature is built. Fall
    // back to WebM/Opus only if Piped does not expose an M4A stream.
    QJsonObject best;
    int bestBitrate = -1;
    bool bestIsM4a = false;
    for (const QJsonValue& v : audioStreams) {
        const QJsonObject s = v.toObject();
        const int br = s.value(QStringLiteral("bitrate")).toInt();
        const QString codec =
                s.value(QStringLiteral("codec")).toString().toLower();
        const QString ext = extFromMime(
                s.value(QStringLiteral("mimeType")).toString(), codec);
        const bool isM4a = ext == QStringLiteral("m4a");
        // Prefer M4A/AAC over WebM/Opus for playback compatibility; within
        // the chosen family prefer higher bitrate.
        if (best.isEmpty() || (isM4a && !bestIsM4a) ||
                (isM4a == bestIsM4a && br > bestBitrate)) {
            best = s;
            bestBitrate = br;
            bestIsM4a = isM4a;
        }
    }
    const QString streamUrl = best.value(QStringLiteral("url")).toString();
    if (streamUrl.isEmpty()) {
        onFailure(tr("No usable audio stream URL"));
        return;
    }
    const QString ext = extFromMime(
            best.value(QStringLiteral("mimeType")).toString(),
            best.value(QStringLiteral("codec")).toString().toLower());
    const QString outPath =
            QDir(cacheDir).filePath(videoId + QLatin1Char('.') + ext);

    // Stream straight to disk via a temp file we rename on completion. A
    // partial file left behind from a kill/crash would otherwise look like
    // a complete download to the next launch's library scanner.
    auto* outFile = new QFile(outPath + QStringLiteral(".part"));
    if (!outFile->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        const QString err = outFile->errorString();
        delete outFile;
        onFailure(tr("Cannot open %1: %2").arg(outPath, err));
        return;
    }

    QNetworkRequest req((QUrl(streamUrl)));
    req.setRawHeader("User-Agent", "Mixxx/YouTube");
    // googlevideo CDN is generally fast; a transfer timeout of 10 min mirrors
    // the yt-dlp watchdog and matches kDownloadTimeoutMs.
    req.setTransferTimeout(kDownloadTimeoutMs);

    QNetworkReply* reply = m_pNam->get(req);

    // Stream the body to disk as it arrives — important on Android where
    // RAM-buffering a full audio file can OOM on large mixes.
    connect(reply, &QNetworkReply::readyRead, this, [reply, outFile]() {
        outFile->write(reply->readAll());
    });
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, outFile, outPath, videoId, onFailure]() {
                reply->deleteLater();
                outFile->write(reply->readAll());
                outFile->close();
                if (reply->error() != QNetworkReply::NoError) {
                    QFile::remove(outFile->fileName());
                    delete outFile;
                    onFailure(reply->errorString());
                    return;
                }
                // Atomic rename .part → final path. If a previous run left a
                // stale final file behind (rare — we only get here on
                // success), QFile::rename refuses to overwrite, so unlink
                // first.
                QFile::remove(outPath);
                if (!outFile->rename(outPath)) {
                    const QString err = outFile->errorString();
                    QFile::remove(outFile->fileName());
                    delete outFile;
                    onFailure(tr("Cannot finalize %1: %2").arg(outPath, err));
                    return;
                }
                delete outFile;
                kLogger.info() << "Downloaded" << videoId << "→" << outPath
                               << "via Piped";
                finalizeDownload(videoId, outPath);
            });
}

// =============================================================================
// yt-dlp (desktop fallback only)
// =============================================================================

void YouTubeService::runYtDlp(const QStringList& args,
        int timeoutMs,
        const std::function<void(const QByteArray&)>& onSuccess,
        const std::function<void(const QString&)>& onFailure) {
    if (m_ytDlpPath.isEmpty()) {
        onFailure(tr("yt-dlp not available"));
        return;
    }

    auto* proc = new QProcess(this);
    proc->setProcessChannelMode(QProcess::SeparateChannels);

    // Watchdog timer parented to `proc`; the connect target is also `proc`,
    // so when proc is destroyed both the timer and its lambda are torn down
    // before any post-cleanup fire is possible.
    auto* watchdog = new QTimer(proc);
    watchdog->setSingleShot(true);
    watchdog->setInterval(timeoutMs);
    connect(watchdog, &QTimer::timeout, proc, [proc]() {
        kLogger.warning() << "yt-dlp watchdog timeout — killing process";
        proc->kill();
    });

    // Cleanup is centralized: `finished` is the single place that calls
    // deleteLater(). FailedToStart is the one error case where `finished`
    // does *not* fire (Qt skips it because the process never started), so
    // the errorOccurred handler synthesizes a failure for that one case.
    connect(proc,
            &QProcess::finished,
            this,
            [proc, watchdog, onSuccess, onFailure](
                    int exitCode, QProcess::ExitStatus status) {
                watchdog->stop();
                proc->deleteLater();
                const QByteArray out = proc->readAllStandardOutput();
                const QByteArray err = proc->readAllStandardError();
                if (status != QProcess::NormalExit || exitCode != 0) {
                    QString msg = QString::fromLocal8Bit(err).trimmed();
                    if (msg.isEmpty()) {
                        msg = tr("yt-dlp exited with code %1").arg(exitCode);
                    }
                    onFailure(msg);
                    return;
                }
                onSuccess(out);
            });
    connect(proc,
            &QProcess::errorOccurred,
            this,
            [proc, watchdog, onFailure](QProcess::ProcessError error) {
                if (error != QProcess::FailedToStart) {
                    return; // `finished` will run cleanup
                }
                watchdog->stop();
                proc->deleteLater();
                onFailure(tr("Failed to launch yt-dlp"));
            });

    watchdog->start();
    proc->start(m_ytDlpPath, args);
}

void YouTubeService::searchViaYtDlp(const QString& query, int cap) {
    const QStringList args = {
            QStringLiteral("--flat-playlist"),
            QStringLiteral("--skip-download"),
            QStringLiteral("--dump-single-json"),
            QStringLiteral("--no-warnings"),
            QStringLiteral("--no-cache-dir"),
            QStringLiteral("--ignore-config"),
            QStringLiteral("--match-filter"),
            QStringLiteral("!is_live & live_status != is_live & live_status != is_upcoming"),
            QStringLiteral("--default-search"),
            QStringLiteral("ytsearch"),
            QStringLiteral("ytsearch%1:%2").arg(cap).arg(query),
    };
    runYtDlp(
            args,
            kSearchTimeoutMs,
            [this, query, cap](const QByteArray& stdoutBytes) {
                const QJsonObject root =
                        QJsonDocument::fromJson(stdoutBytes).object();
                const QJsonArray entries =
                        root.value(QStringLiteral("entries")).toArray();
                QList<YouTubeVideoInfo> results;
                results.reserve(entries.size());
                for (const QJsonValue& v : entries) {
                    if (results.size() >= cap) {
                        break;
                    }
                    const QJsonObject entry = v.toObject();
                    YouTubeVideoInfo info;
                    info.id = entry.value(QStringLiteral("id")).toString();
                    info.title = entry.value(QStringLiteral("title")).toString();
                    info.uploader =
                            entry.value(QStringLiteral("channel")).toString();
                    if (info.uploader.isEmpty()) {
                        info.uploader = entry.value(QStringLiteral("uploader"))
                                                .toString();
                    }
                    const QJsonValue dur = entry.value(QStringLiteral("duration"));
                    if (dur.isDouble()) {
                        info.durationSec = static_cast<int>(dur.toDouble());
                    }
                    info.isLive = isYtDlpLiveStream(entry);
                    if (!info.isLive && isValidYouTubeVideoId(info.id) &&
                            !info.title.isEmpty()) {
                        results.append(info);
                    }
                }
                kLogger.info() << "yt-dlp returned" << results.size()
                               << "results for" << query;
                Q_EMIT searchResultsReady(query, results);
            },
            [this, query](const QString& err) {
                kLogger.warning() << "yt-dlp search failed:" << err;
                Q_EMIT searchFailed(query, err);
            });
}

void YouTubeService::downloadViaYtDlp(const QString& videoId, const QString& cacheDir) {
    const QString outTemplate =
            QDir(cacheDir).filePath(QStringLiteral("%(id)s.%(ext)s"));
    const QStringList args = {
            QStringLiteral("-f"),
            QStringLiteral("bestaudio"),
            QStringLiteral("--no-playlist"),
            QStringLiteral("--no-warnings"),
            QStringLiteral("--no-progress"),
            QStringLiteral("--no-cache-dir"),
            QStringLiteral("--ignore-config"),
            QStringLiteral("--no-mtime"),
            QStringLiteral("-o"),
            outTemplate,
            QStringLiteral("--print"),
            QStringLiteral("after_move:filepath"),
            QStringLiteral("--"),
            QStringLiteral("https://www.youtube.com/watch?v=") + videoId,
    };
    runYtDlp(
            args,
            kDownloadTimeoutMs,
            [this, videoId, cacheDir](const QByteArray& stdoutBytes) {
                // Last non-empty stdout line is the final filepath.
                QString outPath;
                const QList<QByteArray> lines = stdoutBytes.split('\n');
                for (auto it = lines.crbegin(); it != lines.crend(); ++it) {
                    const QString line = QString::fromLocal8Bit(*it).trimmed();
                    if (!line.isEmpty() && QFileInfo::exists(line)) {
                        outPath = line;
                        break;
                    }
                }
                if (outPath.isEmpty()) {
                    // Fallback: scan the cache dir for any file with our id.
                    const QDir dir(cacheDir);
                    const QStringList existing =
                            dir.entryList({videoId + QStringLiteral(".*")},
                                    QDir::Files | QDir::NoDotAndDotDot);
                    for (const QString& f : existing) {
                        if (f.endsWith(QStringLiteral(".info.json")) ||
                                f.endsWith(QStringLiteral(".sponsor.json")) ||
                                f.endsWith(QStringLiteral(".part"))) {
                            continue;
                        }
                        outPath = dir.filePath(f);
                        break;
                    }
                }
                if (outPath.isEmpty()) {
                    Q_EMIT downloadFailed(videoId,
                            tr("yt-dlp finished but no output file was found"));
                    return;
                }
                kLogger.info() << "Downloaded" << videoId << "→" << outPath
                               << "via yt-dlp";
                finalizeDownload(videoId, outPath);
            },
            [this, videoId](const QString& err) {
                kLogger.warning() << "yt-dlp download failed:" << err;
                Q_EMIT downloadFailed(videoId, err);
            });
}

// =============================================================================
// Shared post-download chain: SponsorBlock fetch → in-place cut → emit
// =============================================================================

void YouTubeService::finalizeDownload(const QString& videoId, const QString& outPath) {
    // Fetch SponsorBlock segments, physically cut them out of the file (so
    // duration / BPM / waveform all reflect the music-only length), THEN
    // tell consumers the file is ready. We deliberately do this ourselves
    // rather than via yt-dlp's --sponsorblock-remove because the latter
    // requires a re-encode pass through ffmpeg and can drop quality.
    fetchSponsorSegmentsInternal(videoId,
            [this, videoId, outPath](const QList<SponsorSegment>& segments) {
                if (!segments.isEmpty()) {
                    const bool cut = cutAudioRanges(outPath, segments);
                    if (!cut) {
                        // Cutting failed (e.g. mid-file in the middle of a
                        // packet boundary on opus). Write the sidecar so
                        // SponsorBlockController can fall back to skip-at-
                        // playback during deck use.
                        QFile sidecar(outPath +
                                QStringLiteral(".sponsor.json"));
                        if (sidecar.open(QIODevice::WriteOnly |
                                    QIODevice::Truncate)) {
                            QJsonArray arr;
                            for (const auto& s : segments) {
                                QJsonObject o;
                                o.insert(QStringLiteral("start"), s.start);
                                o.insert(QStringLiteral("end"), s.end);
                                o.insert(QStringLiteral("category"),
                                        s.category);
                                arr.append(o);
                            }
                            sidecar.write(QJsonDocument(arr).toJson(
                                    QJsonDocument::Compact));
                        }
                    }
                }
                Q_EMIT downloadFinished(videoId, outPath);
            });
}

void YouTubeService::fetchSponsorSegments(const QString& videoId) {
    fetchSponsorSegmentsInternal(videoId,
            [this, videoId](const QList<SponsorSegment>& segments) {
                Q_EMIT sponsorSegmentsFetched(videoId, segments);
            });
}

void YouTubeService::fetchSponsorSegmentsInternal(
        const QString& videoId,
        const std::function<void(const QList<SponsorSegment>&)>& cb) {
    // SponsorBlock public API: https://wiki.sponsor.ajay.app/w/API_Docs
    // Categories include third-party sponsorships, creator self-promo,
    // intros/outros, viewer-interaction reminders, content previews, and
    // non-music sections within music videos. We download audio-only
    // streams from Piped/yt-dlp, so YouTube's pre/mid/post-roll ads are
    // already absent — SponsorBlock only needs to handle the in-content
    // creator-inserted breaks.
    const QUrl url(
            QStringLiteral("https://sponsor.ajay.app/api/skipSegments?videoID=%1"
                           "&categories=[\"sponsor\",\"selfpromo\",\"interaction\","
                           "\"intro\",\"outro\",\"preview\",\"music_offtopic\"]")
                    .arg(videoId));
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "Mixxx/SponsorBlock");
    request.setTransferTimeout(kSponsorBlockTimeoutMs);
    QNetworkReply* reply = m_pNam->get(request);
    connect(reply, &QNetworkReply::finished, this, [reply, cb]() {
        reply->deleteLater();
        QList<SponsorSegment> segments;
        if (reply->error() == QNetworkReply::NoError) {
            const QJsonArray array =
                    QJsonDocument::fromJson(reply->readAll()).array();
            for (const QJsonValue& value : array) {
                const QJsonObject obj = value.toObject();
                const QJsonArray seg =
                        obj.value(QStringLiteral("segment")).toArray();
                if (seg.size() < 2) {
                    continue;
                }
                segments.append({seg[0].toDouble(),
                        seg[1].toDouble(),
                        obj.value(QStringLiteral("category")).toString()});
            }
        }
        // 404 just means "no segments for this video" — not an error.
        // Timeout or network failure also yields empty segments — we don't
        // block track loading on SponsorBlock availability.
        cb(segments);
    });
}

} // namespace mixxx

#include "moc_youtubeservice.cpp"
