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
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QProcess>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSet>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QThread>
#include <QThreadPool>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QVector>
#include <atomic>
#include <utility>

#if defined(Q_OS_ANDROID) && defined(HAVE_YTDLP_ANDROID)
#include <QJniEnvironment>
#include <QJniObject>
#include <QNativeInterface/QAndroidApplication>
#endif

#include "library/youtube/youtubeaudiocutter.h"
#include "util/logger.h"

namespace mixxx {

namespace {
const Logger kLogger("YouTubeService");

// Per-request budgets. Search is bounded — Piped's /search and yt-dlp's
// page-parse both finish well under a second on a healthy network. Downloads
// can be larger, but bestaudio for a song is typically <10 MB; the ceiling
// is generous so a slow connection or long mix doesn't get killed.
// 15 s is enough for InnerTube (typically <1s); a faster fail means the
// yt-dlp fallback kicks in sooner on flaky connections.
constexpr int kSearchTimeoutMs = 15 * 1000;
constexpr int kDownloadTimeoutMs = 10 * 60 * 1000; // 10 min
// Piped HTTP requests get a tighter ceiling — each instance failure should
// surface fast so we can fail over to the next one without burning too long
// per dead instance. With 10 instances at 10s each, worst case is ~100s.
constexpr int kPipedHttpTimeoutMs = 10 * 1000;
// Minimum file size before we switch from a single-connection download to 4
// parallel Range requests. Below this threshold the connection-setup overhead
// outweighs the gain; above it, 4 TCP streams typically deliver 2-4x higher
// throughput on cellular links where a single window saturates slowly.
constexpr qint64 kChunkedDownloadThresholdBytes = 2LL * 1024 * 1024; // 2 MB
constexpr int kParallelDownloadChunks = 4;
constexpr int kMaxPipedSearchPages = 5;
// SponsorBlock API timeout — prevents indefinite hangs if the service is slow.
// 4 s is generous for a single HTTP GET backed by a CDN; most responses arrive
// in < 500 ms. Keeping it short prevents blocking track load on a slow or
// unreachable SponsorBlock server.
constexpr int kSponsorBlockTimeoutMs = 4 * 1000;
// Project default for this fork: the requested first-open YouTube feed is Greek
// top songs, not generic global/United States YouTube trends.
const QString kDefaultRegion = QStringLiteral("GR");

// Rate-limiting constants for InnerTube requests.
// We allow at most kRateLimitBurst requests within any kRateLimitWindowMs
// sliding window. When the limit is hit, the request is deferred (or the
// caller skips straight to yt-dlp). These values are conservative — YouTube's
// actual threshold is unknown but empirical testing shows 10+ rapid requests
// within a few seconds can trigger a 429.
constexpr int kRateLimitWindowMs = 10 * 1000; // 10 second window
constexpr int kRateLimitBurst = 8;            // max 8 requests per window
// Minimum inter-request gap in milliseconds. Even under burst we space
// requests by this much to avoid looking like a bot.
constexpr int kMinRequestGapMs = 300;
// Jitter range: random delay added to minimum gap to look more human.
// Each request gets a random delay in [kMinRequestGapMs, kMaxJitterMs].
// Capped at 800 ms (was 1500 ms) so normal user interactions — quick search
// then scroll — are not unnecessarily delayed. The bot-detection benefit of
// larger jitter values is marginal for normal single-user usage patterns.
constexpr int kMaxJitterMs = 800;
// Desktop yt-dlp self-update timeout — generous because it downloads a ~40 MB
// binary from GitHub Releases.
constexpr int kYtDlpUpdateTimeoutMs = 120 * 1000; // 2 min
// Bot-flag exponential backoff: initial, max, and multiplier.
// After first flag: wait 30s. Each subsequent flag doubles the wait up to 10min.
constexpr int kBotFlagInitialBackoffMs = 30 * 1000;  // 30 seconds
constexpr int kBotFlagMaxBackoffMs = 10 * 60 * 1000; // 10 minutes
constexpr int kBotFlagBackoffMultiplier = 2;
// QSettings keys for persisted session state.
const QString kSettingsGroupYouTube = QStringLiteral("YouTube");
const QString kSettingsVisitorData = QStringLiteral("visitorData");
const QString kSettingsBotFlagCount = QStringLiteral("botFlagCount");

// Apply the request attributes every YouTube/Piped/SponsorBlock call needs.
//
// The critical one is disabling HTTP/2 on Android: Qt's HTTP/2 stack is
// unreliable against YouTube's (HTTP/2 + QUIC) endpoints on Android — POST and
// sometimes GET requests stall until the transfer timeout fires and then fail,
// which is exactly the "clicking YouTube lags for ~10s and then shows no
// results" symptom (the request never completes, so search/trending fall all
// the way through every fallback and surface nothing). Forcing HTTP/1.1 makes
// these requests complete reliably. Desktop platforms keep HTTP/2 (where it
// works fine), so behaviour there is unchanged. See QTBUG-100016.
//
// We also opt into safe redirect following so a 30x from any endpoint is
// transparently chased instead of being parsed as an empty body.
void applyYouTubeRequestAttributes(QNetworkRequest* req) {
#if defined(Q_OS_ANDROID)
    req->setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
#endif
    req->setAttribute(QNetworkRequest::RedirectPolicyAttribute,
            QNetworkRequest::NoLessSafeRedirectPolicy);
}

// Hardcoded list of Piped API instances tried in order on per-request
// failure. Instances verified as working (both /search and /streams return
// 200) as of 2026-05-29. Piped community instances are ephemeral — the
// official registry at https://piped-instances.kavin.rocks/ currently lists
// only private.coffee as active. Periodically re-verify and update this list.
// When all Piped instances fail, the youtubedl-android bundled runtime
// (JNI-based, no external Python needed) serves as the Android fallback.
const QStringList kPipedInstances = {
        QStringLiteral("https://api.piped.private.coffee"),
        QStringLiteral("https://api.piped.projectsegfau.lt"),
        QStringLiteral("https://pipedapi.orangenet.cc"),
        QStringLiteral("https://pi.ggtyler.dev"),
        QStringLiteral("https://pipedapi.kavin.rocks"),
};

// InnerTube client contexts tried in order by downloadViaInnerTubeClient().
// Each entry mimics an official YouTube app/device so the player endpoint
// returns plain (non-cipher) adaptive stream URLs that need no JS signature
// deciphering. YouTube periodically tightens individual clients (e.g. the
// ANDROID client now sometimes 403s the actual stream without a poToken), so
// we fail over across several clients to maximise the chance one still yields
// a directly-downloadable audio stream. The iOS client is currently the most
// reliable for unsigned URLs and is therefore tried first.
struct InnerTubeClient {
    const char* clientName;
    const char* clientVersion;
    const char* clientNameId; // numeric X-YouTube-Client-Name
    const char* apiKey;       // optional, empty when not needed
    const char* userAgent;
    const char* deviceMake;  // optional, empty when not applicable
    const char* deviceModel; // optional, empty when not applicable
    int androidSdkVersion;   // 0 = omit
    const char* osName;      // optional, empty when not applicable
    const char* osVersion;   // optional, empty when not applicable
};

const QVector<InnerTubeClient>& innerTubeClients() {
    static const QVector<InnerTubeClient> kClients = {
            // ANDROID_VR (Oculus Quest) — as of 2026 this is the only widely
            // reliable client that still returns plain, directly-downloadable
            // adaptive audio URLs WITHOUT a poToken. It mirrors yt-dlp's
            // current default `android_vr` client context exactly. The other
            // mobile clients (IOS/ANDROID) now require a GVS poToken and reply
            // with HTTP 400 / 403, so this MUST be tried first. Caveat: it
            // cannot access "made for kids" videos, hence the fallbacks below.
            {"ANDROID_VR",
                    "1.65.10",
                    "28",
                    "", // no API key needed; the client context is sufficient
                    "com.google.android.apps.youtube.vr.oculus/1.65.10 (Linux; "
                    "U; Android 12L; eureka-user Build/SQ3A.220605.009.A1) gzip",
                    "Oculus",
                    "Quest 3",
                    32,
                    "Android",
                    "12L"},
            // iOS YouTube app — kept as a fallback for videos ANDROID_VR can't
            // serve. May require a poToken (and then fail over), but still
            // succeeds for many videos.
            {"IOS",
                    "21.02.3",
                    "5",
                    "",
                    "com.google.ios.youtube/21.02.3 (iPhone16,2; U; CPU iOS "
                    "18_3_2 like Mac OS X;)",
                    "Apple",
                    "iPhone16,2",
                    0,
                    "iPhone",
                    "18.3.2.22D82"},
            // TV HTML5 client — last-resort unsigned-URL path.
            {"TVHTML5_SIMPLY_EMBEDDED_PLAYER",
                    "2.0",
                    "85",
                    "AIzaSyAO_FJ2SlqU8Q4STEHLGCilw_Y9_11qcW8",
                    "Mozilla/5.0 (PlayStation; PlayStation 4/12.00) AppleWebKit/"
                    "605.1.15 (KHTML, like Gecko) Version/16.0 Safari/605.1.15",
                    "",
                    "",
                    0,
                    "",
                    ""},
    };
    return kClients;
}

// Client order used specifically for the InnerTube /search endpoint.
//
// This is deliberately NOT innerTubeClients(): the download client list has
// the iOS client second, but for *search* the iOS client replies with a huge
// (~5 MB) `elementRenderer` payload that collectInnerTubeVideos() cannot parse
// — so it always counts as "no usable results" and fails over anyway, after
// wasting bandwidth and time downloading megabytes on mobile. We skip it here.
//
// ANDROID_VR and TVHTML5 both return the compactVideoRenderer shape we parse;
// WEB returns videoRenderer (also parsed) and is a very reliable last resort
// that needs no poToken for search. All three were verified live (2026-06) to
// return HTTP 200 with parseable results. Ordering puts the lightest responses
// first (ANDROID_VR/TVHTML5 are ~130 KB / ~400 KB; WEB is ~1.3 MB).
const QVector<InnerTubeClient>& innerTubeSearchClients() {
    static const QVector<InnerTubeClient> kClients = {
            {"ANDROID_VR",
                    "1.65.10",
                    "28",
                    "",
                    "com.google.android.apps.youtube.vr.oculus/1.65.10 (Linux; "
                    "U; Android 12L; eureka-user Build/SQ3A.220605.009.A1) gzip",
                    "Oculus",
                    "Quest 3",
                    32,
                    "Android",
                    "12L"},
            {"TVHTML5_SIMPLY_EMBEDDED_PLAYER",
                    "2.0",
                    "85",
                    "AIzaSyAO_FJ2SlqU8Q4STEHLGCilw_Y9_11qcW8",
                    "Mozilla/5.0 (PlayStation; PlayStation 4/12.00) AppleWebKit/"
                    "605.1.15 (KHTML, like Gecko) Version/16.0 Safari/605.1.15",
                    "",
                    "",
                    0,
                    "",
                    ""},
            {"WEB",
                    "2.20240620.05.00",
                    "1",
                    "",
                    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/"
                    "537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36",
                    "",
                    "",
                    0,
                    "",
                    ""},
    };
    return kClients;
}
} // anonymous namespace

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
    // Returns the English name of a country given its ISO 3166-1 alpha-2 code.
    // Used for constructing YouTube API queries that must be in English.
    // QLocale::territoryToString / countryToString always returns the Qt built-in
    // English name (not affected by the user's locale).
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const QLocale forCountry(QStringLiteral("en_") + code);
    const QString name = QLocale::countryToString(forCountry.country());
#else
    const QLocale forCountry(QLocale::English,
            QLocale::codeToTerritory(code));
    const QString name = QLocale::territoryToString(forCountry.territory());
#endif
    return name.isEmpty() ? code : name;
}

QString countryTopSongsCategoryQuery(const QString& region) {
    // IMPORTANT: Do NOT use tr() here — this string is sent as a YouTube
    // InnerTube API search query and must always be in English regardless of the
    // user's locale. Previously this was wrapped in QObject::tr(), so on
    // non-English locales (e.g. Greek) the query got translated into the local
    // language and YouTube's API returned zero results — surfacing as a
    // permanently-blank YouTube tab on first open.
    if (region.compare(QStringLiteral("GR"), Qt::CaseInsensitive) == 0) {
        return QStringLiteral("Greek top songs");
    }
    return QStringLiteral("%1 top songs").arg(countryDisplayName(region));
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

// Extract plain text from an InnerTube text object, which may use either the
// {"runs":[{"text":...}, ...]} or the {"simpleText":...} shape.
QString innerTubeText(const QJsonObject& obj) {
    const QJsonValue simple = obj.value(QStringLiteral("simpleText"));
    if (simple.isString()) {
        return simple.toString();
    }
    QString out;
    const QJsonArray runs = obj.value(QStringLiteral("runs")).toArray();
    for (const QJsonValue& r : runs) {
        out += r.toObject().value(QStringLiteral("text")).toString();
    }
    return out;
}

// Parse a "M:SS" or "H:MM:SS" timestamp into seconds. Returns -1 when the
// string is empty or not a recognizable timestamp. Live and upcoming videos
// expose no lengthText at all, so a -1 here doubles as a "this is a live or
// otherwise non-playable item, skip it" signal.
int parseTimestampToSeconds(const QString& text) {
    const QString t = text.trimmed();
    if (t.isEmpty()) {
        return -1;
    }
    const QStringList parts = t.split(QLatin1Char(':'));
    if (parts.isEmpty() || parts.size() > 3) {
        return -1;
    }
    int seconds = 0;
    for (const QString& p : parts) {
        bool ok = false;
        const int n = p.trimmed().toInt(&ok);
        if (!ok || n < 0) {
            return -1;
        }
        seconds = seconds * 60 + n;
    }
    return seconds;
}

// Recursively walk an InnerTube /search (or /browse) response collecting every
// video item. ANDROID_VR/TVHTML5 clients expose items as "compactVideoRenderer"
// and the WEB client as "videoRenderer"; both share the field layout we need
// (videoId, title, byline, lengthText). Channels, playlists and live streams
// are skipped. Stops once `cap` unique videos have been gathered.
void collectInnerTubeVideos(const QJsonValue& node,
        QList<YouTubeVideoInfo>* out,
        QSet<QString>* seen,
        int cap) {
    if (out->size() >= cap) {
        return;
    }
    if (node.isObject()) {
        const QJsonObject obj = node.toObject();
        const QJsonValue compact = obj.value(QStringLiteral("compactVideoRenderer"));
        const QJsonValue plain = obj.value(QStringLiteral("videoRenderer"));
        const QJsonObject video = compact.isObject()
                ? compact.toObject()
                : (plain.isObject() ? plain.toObject() : QJsonObject());
        if (!video.isEmpty()) {
            YouTubeVideoInfo info;
            info.id = video.value(QStringLiteral("videoId")).toString();
            info.title =
                    innerTubeText(video.value(QStringLiteral("title")).toObject());
            QString uploader = innerTubeText(
                    video.value(QStringLiteral("longBylineText")).toObject());
            if (uploader.isEmpty()) {
                uploader = innerTubeText(
                        video.value(QStringLiteral("shortBylineText")).toObject());
            }
            info.uploader = uploader;
            const int dur = parseTimestampToSeconds(innerTubeText(
                    video.value(QStringLiteral("lengthText")).toObject()));
            // No lengthText -> live/upcoming/non-playable: skip it.
            info.isLive = dur < 0;
            info.durationSec = dur > 0 ? dur : 0;
            if (!info.isLive && isValidYouTubeVideoId(info.id) &&
                    !info.title.isEmpty() && !seen->contains(info.id)) {
                seen->insert(info.id);
                out->append(info);
            }
            // Don't descend into a matched renderer's children.
            return;
        }
        for (const QJsonValue& v : obj) {
            collectInnerTubeVideos(v, out, seen, cap);
            if (out->size() >= cap) {
                return;
            }
        }
    } else if (node.isArray()) {
        const QJsonArray arr = node.toArray();
        for (const QJsonValue& v : arr) {
            collectInnerTubeVideos(v, out, seen, cap);
            if (out->size() >= cap) {
                return;
            }
        }
    }
}

// Parse an InnerTube search response into our internal video list, capped at
// `cap`. Returns an empty list when the response contained no usable videos.
QList<YouTubeVideoInfo> parseInnerTubeSearch(const QJsonObject& root, int cap) {
    QList<YouTubeVideoInfo> results;
    QSet<QString> seen;
    collectInnerTubeVideos(QJsonValue(root), &results, &seen, cap);
    return results;
}

// Extract a continuation token from an InnerTube search response, if present.
// The token is embedded inside a continuationItemRenderer node and is needed
// to fetch the next page of results. Returns an empty string when no
// continuation is available (end of results).
//
// YouTube embeds continuation data in two places depending on whether this is
// an initial search or a continuation response:
//
//   Initial:  root.contents.sectionListRenderer.continuations[0]
//             .nextContinuationData.continuation
//
//   Continuation: root.onResponseReceivedCommands[0]
//             .appendContinuationItemsAction.continuationItems[N]
//             .continuationItemRenderer.continuationEndpoint
//             .continuationCommand.token
//
// Extracting recursively is simpler and handles both shapes.
void extractContinuationTokens(const QJsonValue& node, QStringList* out) {
    if (out->size() >= 1) {
        return; // one token is enough
    }
    if (node.isObject()) {
        const QJsonObject obj = node.toObject();
        // nextContinuationData.continuation (initial search shape)
        {
            const QJsonObject ncd =
                    obj.value(QStringLiteral("nextContinuationData")).toObject();
            if (!ncd.isEmpty()) {
                const QString tok =
                        ncd.value(QStringLiteral("continuation")).toString();
                if (!tok.isEmpty()) {
                    out->append(tok);
                    return;
                }
            }
        }
        // continuationCommand.token (continuation response shape)
        {
            const QJsonObject cc =
                    obj.value(QStringLiteral("continuationCommand")).toObject();
            if (!cc.isEmpty()) {
                const QString tok = cc.value(QStringLiteral("token")).toString();
                if (!tok.isEmpty()) {
                    out->append(tok);
                    return;
                }
            }
        }
        for (const QJsonValue& v : obj) {
            extractContinuationTokens(v, out);
            if (out->size() >= 1) {
                return;
            }
        }
    } else if (node.isArray()) {
        const QJsonArray arr = node.toArray(); // clazy:exclude=range-loop-detach
        for (const QJsonValue& v : arr) {
            extractContinuationTokens(v, out);
            if (out->size() >= 1) {
                return;
            }
        }
    }
}

QString extractContinuationToken(const QJsonObject& root) {
    QStringList tokens;
    extractContinuationTokens(QJsonValue(root), &tokens);
    return tokens.isEmpty() ? QString() : tokens.first();
}

// Build the InnerTube `context.client` object for the given client descriptor.
// Shared by the player (download) and search request paths so both speak the
// exact same client identity to YouTube. When `regionOverride` is non-empty it
// replaces the default "US" geo-location hint, which is needed so that trending
// queries for e.g. Greece get Greek results instead of American ones.
QJsonObject innerTubeClientContext(const InnerTubeClient& c,
        const QString& regionOverride = QString()) {
    QJsonObject client;
    client.insert(QStringLiteral("clientName"), QString::fromLatin1(c.clientName));
    client.insert(QStringLiteral("clientVersion"),
            QString::fromLatin1(c.clientVersion));
    client.insert(QStringLiteral("hl"), QStringLiteral("en"));
    client.insert(QStringLiteral("gl"),
            regionOverride.isEmpty() ? QStringLiteral("US") : regionOverride);
    if (c.androidSdkVersion > 0) {
        client.insert(QStringLiteral("androidSdkVersion"), c.androidSdkVersion);
    }
    if (c.deviceMake[0] != '\0') {
        client.insert(QStringLiteral("deviceMake"),
                QString::fromLatin1(c.deviceMake));
    }
    if (c.deviceModel[0] != '\0') {
        client.insert(QStringLiteral("deviceModel"),
                QString::fromLatin1(c.deviceModel));
    }
    if (c.osName[0] != '\0') {
        client.insert(QStringLiteral("osName"), QString::fromLatin1(c.osName));
    }
    if (c.osVersion[0] != '\0') {
        client.insert(QStringLiteral("osVersion"),
                QString::fromLatin1(c.osVersion));
    }
    return client;
}

const QString YouTubeService::kTrendingQueryPrefix =
        QStringLiteral("__trending__:");

YouTubeService::YouTubeService(QObject* parent)
        : QObject(parent),
          m_pNam(new QNetworkAccessManager(this)),
          m_ytDlpPath(locateYtDlp()),
          m_pipedInstances(kPipedInstances) {
    m_rateLimitTimer.start();
    setupCookieJar();
    loadSessionState();
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
#if defined(Q_OS_ANDROID) && defined(HAVE_YTDLP_ANDROID)
    // Android + bundled AAR: the youtubedl-android AAR is compiled into the
    // APK at build time. Return a special marker so downloadVideo() routes to
    // the JNI-based downloader instead of trying to exec a binary.
    return QStringLiteral("android-bundled");
#endif
    // 2. Bundled-next-to-binary + common install dirs (includes Termux on Android).
    const QStringList fallbackBins = ytDlpFallbackBins();
    for (const QString& candidate : std::as_const(fallbackBins)) {
        if (QFileInfo(candidate).isExecutable()) {
            return candidate;
        }
    }
#if !defined(Q_OS_ANDROID)
    // 3. PATH lookup — handles "yt-dlp" and "yt-dlp.exe" on Windows.
    const QString fromPath =
            QStandardPaths::findExecutable(QStringLiteral("yt-dlp"));
    if (!fromPath.isEmpty()) {
        return fromPath;
    }
#endif
    return QString();
}

// =============================================================================
// Public API — InnerTube first (search + download), then Piped, then yt-dlp
// =============================================================================

void YouTubeService::searchVideos(const QString& query, int cap, int minResults) {
    if (query.trimmed().isEmpty()) {
        Q_EMIT searchResultsReady(query, {});
        return;
    }
#if defined(Q_OS_ANDROID)
    kLogger.info() << "[Android] searchVideos: query=" << query
                   << "cap=" << cap
                   << "minResults=" << minResults
                   << "ytDlpPath=" << m_ytDlpPath;
#endif
    // A new search invalidates any continuation from a previous search.
    m_searchContinuationToken.clear();
    m_searchMinResults = minResults;
    // Primary backend: the YouTube InnerTube /search endpoint. It is hit
    // directly (no third-party Piped proxy), so it does not depend on the
    // health of community-run instances — which is why search used to return
    // nothing and lag for ~100s while cycling through dead Piped hosts. We fail
    // over to Piped and then yt-dlp only if InnerTube itself fails.
    searchViaInnerTube(query,
            query,
            cap,
            /*clientIdx=*/0,
            [this, query, cap, minResults](const QString& innerTubeError) {
                const bool hasYtDlpFallback = !m_ytDlpPath.isEmpty();
#if defined(Q_OS_ANDROID)
                // On Android the community Piped instances are almost always
                // dead, so cascading through all of them (5 instances × 3
                // filters, ~10 s each) only adds ~100 s of stalling before
                // search finally fails — the user-reported "clicking YouTube
                // lags then shows nothing" symptom. InnerTube (ANDROID_VR →
                // TVHTML5 → WEB) is the reliable search path on Android, so when
                // it fails we go straight to the bundled/Termux yt-dlp if one is
                // usable, otherwise surface the error immediately. The bundled
                // ("android-bundled") runtime is download-only — it can't run a
                // search — so it does not count here.
                const bool canSearchViaYtDlp = hasYtDlpFallback &&
                        (m_ytDlpPath != QStringLiteral("android-bundled"));
                if (canSearchViaYtDlp) {
                    kLogger.warning()
                            << "InnerTube search failed for" << query << ":"
                            << innerTubeError << "— falling back to yt-dlp";
                    searchViaYtDlp(query, cap);
                } else {
                    kLogger.warning()
                            << "InnerTube search failed for" << query << ":"
                            << innerTubeError
                            << "— no usable fallback on Android, surfacing error";
                    Q_EMIT searchFailed(query, innerTubeError);
                }
#else
                kLogger.warning()
                        << "InnerTube search failed for" << query << ":"
                        << innerTubeError << "— falling back to Piped";
                const bool canSearchViaYtDlp = hasYtDlpFallback;
                searchViaPiped(query,
                        cap,
                        /*instanceIdx=*/0,
                        [this, query, cap, canSearchViaYtDlp](
                                const QString& lastError) {
                            if (canSearchViaYtDlp) {
                                kLogger.warning()
                                        << "All Piped instances failed for "
                                           "search"
                                        << query << ":" << lastError
                                        << "— falling back to yt-dlp";
                                searchViaYtDlp(query, cap);
                            } else {
                                kLogger.warning()
                                        << "All search backends failed for"
                                        << query << ":" << lastError;
                                Q_EMIT searchFailed(query, lastError);
                            }
                        });
#endif
            });
}

void YouTubeService::downloadVideo(const QString& videoId, const QString& cacheDir) {
    if (!isValidYouTubeVideoId(videoId)) {
        Q_EMIT downloadFailed(videoId, tr("Invalid YouTube video id"));
        return;
    }
#if defined(Q_OS_ANDROID)
    kLogger.info() << "[Android] downloadVideo: videoId=" << videoId
                   << "cacheDir=" << cacheDir
                   << "ytDlpPath=" << m_ytDlpPath;
#endif
    QDir().mkpath(cacheDir);

    // Prefetch SponsorBlock segments NOW, in parallel with the audio download.
    // By the time the audio file lands on disk the segments are (usually) ready
    // so finalizeDownload() can skip the sequential SponsorBlock round-trip and
    // proceed directly to cut → emit downloadFinished. This removes the 0–8s
    // wait that made downloads feel slow.
    if (!m_sponsorPrefetchCache.contains(videoId) &&
            !m_sponsorPrefetchWaiters.contains(videoId)) {
        // Insert an empty entry so a second downloadVideo() call for the same
        // id (e.g. retry) doesn't launch a duplicate fetch.
        m_sponsorPrefetchWaiters.insert(videoId, {});
        fetchSponsorSegmentsInternal(videoId,
                [this, videoId](const QList<SponsorSegment>& segments) {
                    // Store segments whether or not anyone is waiting yet.
                    m_sponsorPrefetchCache.insert(videoId, segments);
                    // Notify any finalizeDownload() that arrived while we
                    // were still fetching.
                    const auto waiters =
                            m_sponsorPrefetchWaiters.take(videoId);
                    for (const auto& cb : waiters) {
                        cb(segments);
                    }
                });
    }

    // Primary: the YouTube InnerTube player API (same reliable, proxy-free path
    // used for search). The Android client context returns plain (non-cipher)
    // stream URLs valid for ~6 hours with no external dependencies. We only fall
    // back to the (frequently-dead) Piped instances and then yt-dlp if every
    // InnerTube client fails — this avoids the long stall the user hit while
    // cycling through unreachable Piped hosts before each download.
    downloadViaInnerTube(videoId,
            cacheDir,
            [this, videoId, cacheDir](const QString& innerTubeError) {
#if defined(Q_OS_ANDROID) && defined(HAVE_YTDLP_ANDROID)
                // On Android, skip the Piped detour entirely: the community
                // instances are mostly dead, so cycling 5×10s through them only
                // adds ~50s of lag before the download finally fails. The
                // bundled yt-dlp (real Python yt-dlp via JNI) is the reliable
                // path that actually downloads the song, so go straight to it.
                if (m_ytDlpPath == QStringLiteral("android-bundled")) {
                    kLogger.warning() << "InnerTube download failed for"
                                      << videoId << ":" << innerTubeError
                                      << "— falling back to bundled yt-dlp (JNI)";
                    downloadViaAndroidBundled(videoId, cacheDir);
                    return;
                }
#endif
                kLogger.warning() << "InnerTube download failed for" << videoId
                                  << ":" << innerTubeError
                                  << "— falling back to Piped";
                downloadViaPiped(videoId,
                        cacheDir,
                        /*instanceIdx=*/0,
                        [this, videoId, cacheDir, innerTubeError](
                                const QString& pipedError) {
                            // Piped also failed — last resort: yt-dlp binary.
                            kLogger.warning()
                                    << "All Piped instances failed for download"
                                    << videoId << ":" << pipedError;
                            if (!m_ytDlpPath.isEmpty()) {
                                kLogger.warning() << "Falling back to yt-dlp binary";
                                downloadViaYtDlp(videoId, cacheDir);
                            } else {
                                Q_EMIT downloadFailed(videoId, innerTubeError);
                            }
                        });
            });
}

void YouTubeService::fetchTrending(const QString& region, int cap, int minResults) {
#if defined(Q_OS_ANDROID)
    kLogger.info() << "[Android] fetchTrending: region=" << region
                   << "cap=" << cap
                   << "minResults=" << minResults;
#endif
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
    // Primary backend: resolve the region's "top songs" feed through the
    // InnerTube /search endpoint (same reliable path as searchVideos), then
    // fall back to Piped only if InnerTube fails. The sentinel query carries
    // the region so YouTubeFeature renders a "Trending in <Country>" header.
    const QString sentinelQuery = kTrendingQueryPrefix + r;
    const QString requestQuery = countryTopSongsCategoryQuery(r);
    m_searchMinResults = minResults;
    searchViaInnerTube(sentinelQuery, requestQuery, cap,
            /*clientIdx=*/0,
            [this, r, cap, minResults](const QString& innerTubeError) {
#if defined(Q_OS_ANDROID)
                // See searchVideos(): the Piped instances are effectively dead
                // on Android and cycling through them only stalls the YouTube
                // tab for ~100 s before showing nothing. InnerTube is the
                // reliable trending path here, so surface the failure quickly
                // and let YouTubeFeature::activate() self-heal by retrying on
                // the next activation once connectivity recovers.
                Q_UNUSED(cap);
                kLogger.warning() << "InnerTube trending failed for" << r << ":"
                                  << innerTubeError
                                  << "— no Piped fallback on Android";
                Q_EMIT searchFailed(kTrendingQueryPrefix + r, innerTubeError);
#else
                kLogger.warning() << "InnerTube trending failed for" << r << ":"
                                  << innerTubeError << "— falling back to Piped";
                fetchTrendingViaPiped(r, cap, /*instanceIdx=*/0);
#endif
            },
            /*regionOverride=*/r);
}

void YouTubeService::fetchMusicGenres(const QString& region) {
    // YouTube Music's "Moods & Genres" page is served by InnerTube's /browse
    // endpoint with browseId "FEmusic_moods_and_genres". This returns localized
    // genre/mood category names (e.g. Greek names for gl=GR) that we surface
    // in the sidebar. We use the ANDROID_VR client (same as search) and parse
    // the category titles from the response.
    const QVector<InnerTubeClient>& clients = innerTubeSearchClients();
    if (clients.isEmpty()) {
        Q_EMIT genresReady({});
        return;
    }
    const InnerTubeClient& c = clients.first();

    QUrl reqUrl(QStringLiteral("https://music.youtube.com/youtubei/v1/browse"));
    if (c.apiKey[0] != '\0') {
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("key"), QString::fromLatin1(c.apiKey));
        reqUrl.setQuery(query);
    }

    QJsonObject clientCtx = innerTubeClientContext(c, region);
    if (!m_visitorData.isEmpty()) {
        clientCtx.insert(QStringLiteral("visitorData"), m_visitorData);
    }
    // Override client for YouTube Music browse (YTM uses different client name).
    clientCtx.insert(QStringLiteral("clientName"), QStringLiteral("WEB_REMIX"));
    clientCtx.insert(QStringLiteral("clientVersion"), QStringLiteral("1.20240101.01.00"));

    QJsonObject context;
    context.insert(QStringLiteral("client"), clientCtx);
    QJsonObject body;
    body.insert(QStringLiteral("context"), context);
    body.insert(QStringLiteral("browseId"), QStringLiteral("FEmusic_moods_and_genres"));

    QNetworkRequest req(reqUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader,
            QStringLiteral("application/json"));
    req.setRawHeader("User-Agent", QByteArray(c.userAgent));
    if (!m_visitorData.isEmpty()) {
        req.setRawHeader("X-Goog-Visitor-Id", m_visitorData.toUtf8());
    }
    req.setTransferTimeout(kSearchTimeoutMs);
    applyYouTubeRequestAttributes(&req);

    // Do NOT call recordRequest() here — fetchMusicGenres() is a background
    // metadata fetch to music.youtube.com (a different endpoint from the
    // /search calls the user triggers). Recording it in the shared rate-limit
    // pool would consume one of the kRateLimitBurst slots that user searches
    // need, causing the very first user-initiated search after app activation
    // to be throttled and return no results.

    QNetworkReply* reply = m_pNam->post(
            req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, region]() {
                reply->deleteLater();
                QStringList genres;
                if (reply->error() == QNetworkReply::NoError) {
                    const QJsonObject root =
                            QJsonDocument::fromJson(reply->readAll()).object();
                    // Parse genre titles from the browse response. YouTube Music
                    // returns them nested in grid/shelf renderers under various
                    // paths. We iteratively walk the JSON looking for
                    // "musicNavigationButtonRenderer" items which carry the
                    // genre/mood display text.
                    QVector<QJsonValue> stack;
                    stack.push_back(QJsonValue(root));
                    while (!stack.isEmpty() && genres.size() < 100) {
                        const QJsonValue val = stack.takeLast();
                        if (val.isObject()) {
                            const QJsonObject obj = val.toObject();
                            const QJsonObject navBtn =
                                    obj.value(QStringLiteral(
                                                      "musicNavigationB"
                                                      "uttonRenderer"))
                                            .toObject();
                            if (!navBtn.isEmpty()) {
                                const QJsonArray runs =
                                        navBtn.value(QStringLiteral("buttonText"))
                                                .toObject()
                                                .value(QStringLiteral("runs"))
                                                .toArray();
                                if (!runs.isEmpty()) {
                                    const QString text =
                                            runs.first()
                                                    .toObject()
                                                    .value(QStringLiteral("text"))
                                                    .toString()
                                                    .trimmed();
                                    if (!text.isEmpty() &&
                                            !genres.contains(text)) {
                                        genres.append(text);
                                    }
                                }
                                continue; // don't descend into matched renderer
                            }
                            for (const QJsonValue& v : obj) {
                                stack.push_back(v);
                            }
                        } else if (val.isArray()) {
                            const QJsonArray arr = val.toArray();
                            for (const QJsonValue& v : arr) {
                                stack.push_back(v);
                            }
                        }
                    }
                    kLogger.info() << "Fetched" << genres.size()
                                   << "music genres for region" << region;
                } else {
                    kLogger.warning() << "Genre fetch failed:"
                                      << reply->errorString();
                }
                Q_EMIT genresReady(genres);
            });
}

// =============================================================================
// Piped (primary backend, works on every Qt platform incl. Android)
// =============================================================================

// =============================================================================
// YouTube InnerTube search (primary backend — all platforms incl. Android)
// =============================================================================

void YouTubeService::searchViaInnerTube(const QString& emittedQuery,
        const QString& requestQuery,
        int cap,
        int clientIdx,
        const std::function<void(const QString&)>& onAllFailed,
        const QString& regionOverride,
        int retryCount) {
    const QVector<InnerTubeClient>& clients = innerTubeSearchClients();
    if (clientIdx < 0 || clientIdx >= clients.size()) {
        onAllFailed(tr("All InnerTube clients failed for search"));
        return;
    }
    const InnerTubeClient& c = clients.at(clientIdx);
#if defined(Q_OS_ANDROID)
    kLogger.info() << "[Android] searchViaInnerTube: client="
                   << c.clientName << "clientIdx=" << clientIdx
                   << "query=" << requestQuery
                   << "region=" << regionOverride;
#endif

    // Advance to the next client on any per-client failure (network error or a
    // response we could not parse a single video out of).
    auto tryNext = [this, emittedQuery, requestQuery, cap, clientIdx, onAllFailed, regionOverride](
                           const QString& err) {
        const QVector<InnerTubeClient>& cl = innerTubeSearchClients();
        if (clientIdx + 1 < cl.size()) {
            kLogger.info() << "InnerTube search client"
                           << cl.at(clientIdx).clientName << "failed:" << err
                           << "— trying next client";
            searchViaInnerTube(
                    emittedQuery, requestQuery, cap, clientIdx + 1, onAllFailed, regionOverride);
        } else {
            onAllFailed(err);
        }
    };

    QUrl reqUrl(QStringLiteral("https://www.youtube.com/youtubei/v1/search"));
    if (c.apiKey[0] != '\0') {
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("key"), QString::fromLatin1(c.apiKey));
        reqUrl.setQuery(query);
    }

    QJsonObject context;
    QJsonObject clientCtx = innerTubeClientContext(c, regionOverride);
    // Include visitorData in the client context to maintain session continuity.
    // This mirrors what yt-dlp does — YouTube uses it to track a "session" and
    // is less likely to flag subsequent requests as bot traffic when it's present.
    if (!m_visitorData.isEmpty()) {
        clientCtx.insert(QStringLiteral("visitorData"), m_visitorData);
    }
    context.insert(QStringLiteral("client"), clientCtx);
    QJsonObject body;
    body.insert(QStringLiteral("context"), context);
    body.insert(QStringLiteral("query"), requestQuery);
    // Restrict results to the "Video" type only (protobuf search filter
    // sp=EgIQAQ%3D%3D). Without it the InnerTube /search endpoint returns a
    // mix of playlists, channels and a handful of videos — e.g. the Greek home
    // feed came back with only ~5 playable videos drowned in 15 playlist
    // entries we cannot play. The filter both removes the unplayable
    // playlist/channel cards and surfaces a much larger set of actual songs
    // (verified live 2026-06: "Greek top songs" 5 → 20 video results).
    body.insert(QStringLiteral("params"), QStringLiteral("EgIQAQ=="));

    QNetworkRequest req(reqUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader,
            QStringLiteral("application/json"));
    req.setRawHeader("User-Agent", QByteArray(c.userAgent));
    req.setRawHeader("X-YouTube-Client-Name", QByteArray(c.clientNameId));
    req.setRawHeader("X-YouTube-Client-Version", QByteArray(c.clientVersion));
    // Echo back visitorData from a previous response to maintain session
    // continuity and reduce the chance of YouTube flagging us as a bot.
    if (!m_visitorData.isEmpty()) {
        req.setRawHeader("X-Goog-Visitor-Id", m_visitorData.toUtf8());
    }
    req.setTransferTimeout(kSearchTimeoutMs);
    applyYouTubeRequestAttributes(&req);
    applyBrowserFingerprint(&req, c.clientNameId);

    // Check for bot-flag recovery before each request attempt.
    maybeRecoverFromBotFlag();

    // Rate-limit / bot-flag check.
    //
    // When bot-flagged, skip all InnerTube clients entirely.
    //
    // When merely rate-limited, do NOT call tryNext() immediately — that
    // exhausts every client in < 1 ms (the window has not advanced at all)
    // and surfaces "no results" to the user. Instead, defer this exact same
    // client attempt by kMaxJitterMs+100 ms (long enough to guarantee the
    // minimum-gap check clears regardless of the random jitter value). Allow
    // one deferred retry per call; if we are still throttled after waiting,
    // fall through to tryNext() as a last resort.
    if (m_botFlagActive) {
        kLogger.info() << "Bot-flagged: skipping InnerTube search, "
                          "falling through to fallback";
        tryNext(tr("Request throttled (bot detection active)"));
        return;
    }
    if (shouldThrottleRequest()) {
        if (retryCount == 0) {
            const int retryMs = kMaxJitterMs + 100;
            kLogger.info() << "Throttled: deferring InnerTube search for"
                           << requestQuery << "by" << retryMs << "ms"
                           << "(client" << clientIdx << ")";
            QPointer<YouTubeService> guard(this);
            QTimer::singleShot(retryMs,
                    this,
                    [guard, emittedQuery, requestQuery, cap, clientIdx, onAllFailed, regionOverride]() {
                        if (!guard) {
                            return;
                        }
                        guard->searchViaInnerTube(emittedQuery,
                                requestQuery,
                                cap,
                                clientIdx,
                                onAllFailed,
                                regionOverride,
                                /*retryCount=*/1);
                    });
            return;
        }
        // Already waited once and still throttled — let the caller fall
        // through to the next client / fallback gracefully.
        kLogger.info() << "Throttled after retry: falling through to fallback";
        tryNext(tr("Request throttled (rate limit, already retried)"));
        return;
    }
    recordRequest();

    QNetworkReply* reply = m_pNam->post(
            req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, emittedQuery, requestQuery, cap, tryNext]() {
                reply->deleteLater();
                const int httpStatus = reply->attribute(
                                                    QNetworkRequest::HttpStatusCodeAttribute)
                                               .toInt();
                if (reply->error() != QNetworkReply::NoError) {
                    // Check for bot flagging before generic error handling.
                    const QByteArray rawBody = reply->readAll();
                    const QJsonObject errRoot =
                            QJsonDocument::fromJson(rawBody).object();
                    if (detectBotFlagging(httpStatus, errRoot, rawBody)) {
                        // Bot flagged — skip all remaining InnerTube clients.
                        tryNext(tr("Bot detection triggered (HTTP %1)")
                                        .arg(httpStatus));
                        return;
                    }
                    const QString detail = tr("InnerTube search request failed: %1 (HTTP %2)")
                                                   .arg(reply->errorString())
                                                   .arg(httpStatus);
                    kLogger.warning() << detail;
                    tryNext(detail);
                    return;
                }
                const QByteArray rawBody = reply->readAll();
                const QJsonObject root =
                        QJsonDocument::fromJson(rawBody).object();
                // Check for bot flagging even on HTTP 200 (YouTube sometimes
                // returns 200 with a LOGIN_REQUIRED playability status).
                if (detectBotFlagging(httpStatus, root, rawBody)) {
                    tryNext(tr("Bot detection triggered on search response"));
                    return;
                }
                const QList<YouTubeVideoInfo> results =
                        parseInnerTubeSearch(root, cap);
                if (results.isEmpty()) {
                    // A 200 with no parseable videos usually means this client
                    // returned a renderer shape we don't handle (e.g. the iOS
                    // elementRenderer). Fail over to the next client.
                    kLogger.warning()
                            << "InnerTube search returned HTTP" << httpStatus
                            << "but 0 parseable results for" << requestQuery
                            << "— response size:" << rawBody.size() << "bytes"
                            << "— first 200 chars:"
                            << QString::fromUtf8(rawBody.left(200));
                    tryNext(tr("InnerTube search returned no usable results (HTTP %1, %2 bytes)")
                                    .arg(httpStatus)
                                    .arg(rawBody.size()));
                    return;
                }
                kLogger.info() << "InnerTube search returned" << results.size()
                               << "results for" << requestQuery;
                // Store continuation token for fetchMoreSearchResults().
                m_searchContinuationToken = extractContinuationToken(root);
                // If we need more results and a continuation token is available,
                // auto-fetch additional pages before emitting. This ensures the
                // initial result set is large enough for infinite scroll.
                if (m_searchMinResults > 0 &&
                        results.size() < m_searchMinResults &&
                        !m_searchContinuationToken.isEmpty()) {
                    kLogger.info()
                            << "Auto-fetching continuation pages: have"
                            << results.size() << "need" << m_searchMinResults;
                    autoFetchContinuationPages(emittedQuery,
                            cap,
                            m_searchMinResults,
                            results,
                            clientIdx);
                    return;
                }
                m_searchMinResults = 0;
                Q_EMIT searchResultsReady(emittedQuery, results);
            });
}

void YouTubeService::fetchMoreSearchResults(
        const QString& emittedQuery, int cap) {
    if (m_searchContinuationToken.isEmpty()) {
        kLogger.debug() << "fetchMoreSearchResults: no continuation token";
        return;
    }
    const QVector<InnerTubeClient>& clients = innerTubeSearchClients();
    if (clients.isEmpty()) {
        return;
    }
    const InnerTubeClient& c = clients.at(0);
    const QString token = m_searchContinuationToken;

    QUrl reqUrl(QStringLiteral("https://www.youtube.com/youtubei/v1/search"));
    if (c.apiKey[0] != '\0') {
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("key"), QString::fromLatin1(c.apiKey));
        reqUrl.setQuery(query);
    }

    QJsonObject context;
    QJsonObject clientCtx = innerTubeClientContext(c);
    if (!m_visitorData.isEmpty()) {
        clientCtx.insert(QStringLiteral("visitorData"), m_visitorData);
    }
    context.insert(QStringLiteral("client"), clientCtx);
    QJsonObject body;
    body.insert(QStringLiteral("context"), context);
    body.insert(QStringLiteral("continuation"), token);

    QNetworkRequest req(reqUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader,
            QStringLiteral("application/json"));
    req.setRawHeader("User-Agent", QByteArray(c.userAgent));
    req.setRawHeader("X-YouTube-Client-Name", QByteArray(c.clientNameId));
    req.setRawHeader("X-YouTube-Client-Version", QByteArray(c.clientVersion));
    if (!m_visitorData.isEmpty()) {
        req.setRawHeader("X-Goog-Visitor-Id", m_visitorData.toUtf8());
    }
    req.setTransferTimeout(kSearchTimeoutMs);
    applyYouTubeRequestAttributes(&req);
    applyBrowserFingerprint(&req, c.clientNameId);

    maybeRecoverFromBotFlag();
    if (m_botFlagActive) {
        kLogger.info() << "Bot-flagged: skipping continuation fetch";
        return;
    }
    if (shouldThrottleRequest()) {
        // Defer rather than drop: preserve the continuation token and retry
        // after the jitter window. This prevents the scroll-to-load-more from
        // silently breaking when the user scrolls quickly after the initial
        // results arrive (the model's m_hasMore was already set to false by
        // fetchMore() before emitting fetchMoreRequested, so without the retry
        // the user would never see more results for this search).
        const int delay = jitterDelayMs();
        kLogger.info() << "Throttled: retrying continuation fetch in"
                       << delay << "ms";
        QPointer<YouTubeService> guard(this);
        QTimer::singleShot(delay, this, [guard, emittedQuery, cap]() {
            if (guard) {
                guard->fetchMoreSearchResults(emittedQuery, cap);
            }
        });
        return;
    }
    recordRequest();

    kLogger.info() << "Fetching more search results for" << emittedQuery;
    QNetworkReply* reply = m_pNam->post(
            req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, emittedQuery, cap]() {
                reply->deleteLater();
                if (reply->error() != QNetworkReply::NoError) {
                    kLogger.warning()
                            << "InnerTube continuation request failed:"
                            << reply->errorString();
                    m_searchContinuationToken.clear();
                    return;
                }
                const QByteArray rawBody = reply->readAll();
                const QJsonObject root =
                        QJsonDocument::fromJson(rawBody).object();
                const QList<YouTubeVideoInfo> results =
                        parseInnerTubeSearch(root, cap);
                if (results.isEmpty()) {
                    kLogger.debug()
                            << "InnerTube continuation returned 0 results";
                    m_searchContinuationToken.clear();
                    return;
                }
                kLogger.info() << "InnerTube continuation returned"
                               << results.size() << "more results";
                m_searchContinuationToken = extractContinuationToken(root);
                Q_EMIT searchMoreReady(emittedQuery, results);
            });
}

void YouTubeService::autoFetchContinuationPages(const QString& emittedQuery,
        int cap,
        int minResults,
        QList<mixxx::YouTubeVideoInfo> accumulated,
        int clientIdx) {
    // Guard: no continuation token or already have enough.
    if (m_searchContinuationToken.isEmpty() ||
            accumulated.size() >= minResults) {
        m_searchMinResults = 0;
        Q_EMIT searchResultsReady(emittedQuery, accumulated);
        return;
    }
    // Guard: cap is the hard upper bound — don't exceed it.
    if (accumulated.size() >= cap) {
        m_searchMinResults = 0;
        Q_EMIT searchResultsReady(emittedQuery, accumulated);
        return;
    }

    const QVector<InnerTubeClient>& clients = innerTubeSearchClients();
    if (clientIdx < 0 || clientIdx >= clients.size()) {
        m_searchMinResults = 0;
        Q_EMIT searchResultsReady(emittedQuery, accumulated);
        return;
    }
    const InnerTubeClient& c = clients.at(clientIdx);
    const QString token = m_searchContinuationToken;

    QUrl reqUrl(QStringLiteral("https://www.youtube.com/youtubei/v1/search"));
    if (c.apiKey[0] != '\0') {
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("key"), QString::fromLatin1(c.apiKey));
        reqUrl.setQuery(query);
    }

    QJsonObject context;
    QJsonObject clientCtx = innerTubeClientContext(c);
    if (!m_visitorData.isEmpty()) {
        clientCtx.insert(QStringLiteral("visitorData"), m_visitorData);
    }
    context.insert(QStringLiteral("client"), clientCtx);
    QJsonObject body;
    body.insert(QStringLiteral("context"), context);
    body.insert(QStringLiteral("continuation"), token);

    QNetworkRequest req(reqUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader,
            QStringLiteral("application/json"));
    req.setRawHeader("User-Agent", QByteArray(c.userAgent));
    req.setRawHeader("X-YouTube-Client-Name", QByteArray(c.clientNameId));
    req.setRawHeader("X-YouTube-Client-Version", QByteArray(c.clientVersion));
    if (!m_visitorData.isEmpty()) {
        req.setRawHeader("X-Goog-Visitor-Id", m_visitorData.toUtf8());
    }
    req.setTransferTimeout(kSearchTimeoutMs);
    applyYouTubeRequestAttributes(&req);
    applyBrowserFingerprint(&req, c.clientNameId);

    maybeRecoverFromBotFlag();
    if (m_botFlagActive) {
        kLogger.info() << "Bot-flagged: stopping auto-fetch, emitting"
                       << accumulated.size() << "results";
        m_searchMinResults = 0;
        Q_EMIT searchResultsReady(emittedQuery, accumulated);
        return;
    }
    if (shouldThrottleRequest()) {
        // Defer: retry after jitter delay, preserving accumulated results.
        const int delay = jitterDelayMs();
        kLogger.info() << "Throttled: retrying auto-fetch in" << delay << "ms";
        QPointer<YouTubeService> guard(this);
        QTimer::singleShot(delay, this, [guard, emittedQuery, cap, minResults, accumulated, clientIdx]() {
                    if (guard) {
                        guard->autoFetchContinuationPages(emittedQuery, cap, minResults, accumulated, clientIdx);
                    }
                });
        return;
    }
    recordRequest();

    kLogger.info() << "Auto-fetching continuation page: have"
                   << accumulated.size() << "need" << minResults;
    QNetworkReply* reply = m_pNam->post(
            req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    QPointer<YouTubeService> guard(this);
    connect(reply, &QNetworkReply::finished, this, [this, reply, guard, emittedQuery, cap, minResults, accumulated, clientIdx]() {
        if (!guard) {
            return;
        }
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            kLogger.warning()
                    << "Auto-fetch continuation failed:"
                    << reply->errorString()
                    << "— emitting" << accumulated.size() << "results so far";
            m_searchMinResults = 0;
            Q_EMIT searchResultsReady(emittedQuery, accumulated);
            return;
        }
        const QByteArray rawBody = reply->readAll();
        const QJsonObject root =
                QJsonDocument::fromJson(rawBody).object();
        if (detectBotFlagging(
                    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                            .toInt(),
                    root,
                    rawBody)) {
            kLogger.info() << "Bot-flagged during auto-fetch: emitting"
                           << accumulated.size() << "results so far";
            m_searchMinResults = 0;
            Q_EMIT searchResultsReady(emittedQuery, accumulated);
            return;
        }
        const QList<YouTubeVideoInfo> pageResults =
                parseInnerTubeSearch(root, cap);
        // Merge, deduplicating by video id.
        QSet<QString> seen;
        for (const auto& v : accumulated) {
            seen.insert(v.id);
        }
        for (const auto& v : pageResults) {
            if (!seen.contains(v.id)) {
                accumulated.append(v);
                seen.insert(v.id);
            }
        }
        kLogger.info() << "Auto-fetch page returned" << pageResults.size()
                       << "results, total now" << accumulated.size();
        m_searchContinuationToken = extractContinuationToken(root);
        // Recurse if we still need more and there are more pages.
        autoFetchContinuationPages(emittedQuery, cap, minResults, accumulated, clientIdx);
    });
}

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
            [this, query, cap, onAllFailed](const QString& /*lastError*/) {
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
    applyYouTubeRequestAttributes(&req);

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
    applyYouTubeRequestAttributes(&req);

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
    applyYouTubeRequestAttributes(&req);

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
        const std::function<void(const QString&)>& onFailure,
        const QString& streamUserAgent) {
#if defined(Q_OS_ANDROID)
    kLogger.info() << "[Android] downloadAudioStream: videoId=" << videoId
                   << "streams=" << audioStreams.size()
                   << "cacheDir=" << cacheDir;
#endif
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

    // Use parallel Range requests when InnerTube told us the file size upfront.
    // On cellular links a single TCP stream saturates its congestion window
    // slowly; 4 concurrent streams typically deliver 2-4x higher throughput.
    const qint64 contentLength =
            best.value(QStringLiteral("contentLength")).toVariant().toLongLong();
    if (contentLength >= kChunkedDownloadThresholdBytes) {
        downloadAudioStreamChunked(
                videoId, outPath, streamUrl, contentLength, streamUserAgent, onFailure);
        return;
    }

    // --- single-connection fallback (Piped, or InnerTube without contentLength) ---
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
    // googlevideo validates that the stream request comes from the same client
    // that resolved the URL. When the URL was obtained from an InnerTube client
    // (Android/iOS), reuse that client's User-Agent so the CDN does not reject
    // the request with HTTP 403. Piped URLs are already proxied/unsigned, so a
    // generic User-Agent is fine there.
    req.setRawHeader("User-Agent",
            streamUserAgent.isEmpty()
                    ? QByteArray("Mixxx/YouTube")
                    : streamUserAgent.toUtf8());
    // googlevideo CDN is generally fast; a transfer timeout of 10 min mirrors
    // the yt-dlp watchdog and matches kDownloadTimeoutMs.
    req.setTransferTimeout(kDownloadTimeoutMs);
    applyYouTubeRequestAttributes(&req);

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
                kLogger.info() << "Downloaded" << videoId << "→" << outPath;
                finalizeDownload(videoId, outPath);
            });
}

// Split the download into kParallelDownloadChunks concurrent Range requests.
// All Qt network signals fire on the UI thread so the QFile seek+write
// sequence is safe without any locking even when readyRead callbacks for
// different chunks interleave inside the event loop.
void YouTubeService::downloadAudioStreamChunked(
        const QString& videoId,
        const QString& outPath,
        const QString& streamUrl,
        qint64 contentLength,
        const QString& streamUserAgent,
        const std::function<void(const QString&)>& onFailure) {
    kLogger.info() << "Chunked download:" << videoId
                   << contentLength << "bytes in" << kParallelDownloadChunks
                   << "chunks";

    const QString partPath = outPath + QStringLiteral(".part");
    auto* outFile = new QFile(partPath);
    if (!outFile->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        const QString err = outFile->errorString();
        delete outFile;
        onFailure(tr("Cannot open %1: %2").arg(outPath, err));
        return;
    }
    // Pre-allocate so each chunk can seek and write at its offset without
    // the OS needing to extend the file on every write.
    if (!outFile->resize(contentLength)) {
        // Out of space or filesystem limitation — fall back to single stream.
        outFile->close();
        delete outFile;
        kLogger.warning() << "Chunked pre-alloc failed for" << videoId
                          << "— falling back to single stream";
        // Re-enter the single-connection path.
        QNetworkRequest req((QUrl(streamUrl)));
        req.setRawHeader("User-Agent",
                streamUserAgent.isEmpty() ? QByteArray("Mixxx/YouTube")
                                          : streamUserAgent.toUtf8());
        req.setTransferTimeout(kDownloadTimeoutMs);
        applyYouTubeRequestAttributes(&req);
        QNetworkReply* reply = m_pNam->get(req);
        auto* fb = new QFile(outPath + QStringLiteral(".part"));
        if (!fb->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            delete fb;
            onFailure(tr("Cannot open %1").arg(outPath));
            return;
        }
        connect(reply, &QNetworkReply::readyRead, this, [reply, fb]() {
            fb->write(reply->readAll());
        });
        connect(reply,
                &QNetworkReply::finished,
                this,
                [this, reply, fb, outPath, videoId, onFailure]() {
                    reply->deleteLater();
                    fb->write(reply->readAll());
                    fb->close();
                    if (reply->error() != QNetworkReply::NoError) {
                        QFile::remove(fb->fileName());
                        delete fb;
                        onFailure(reply->errorString());
                        return;
                    }
                    QFile::remove(outPath);
                    if (!fb->rename(outPath)) {
                        QFile::remove(fb->fileName());
                        delete fb;
                        onFailure(tr("Cannot finalize %1").arg(outPath));
                        return;
                    }
                    delete fb;
                    finalizeDownload(videoId, outPath);
                });
        return;
    }

    // Shared completion state — plain pointer, lifetime guarded by the
    // `remaining == 0` check in the last finished handler. All accesses are
    // on the UI thread so no lock is needed.
    struct ChunkState {
        int remaining;
        bool failed = false;
        QString error;
    };
    auto* state = new ChunkState{kParallelDownloadChunks, false, {}};

    const QByteArray ua = streamUserAgent.isEmpty()
            ? QByteArray("Mixxx/YouTube")
            : streamUserAgent.toUtf8();

    const qint64 chunkSize =
            (contentLength + kParallelDownloadChunks - 1) / kParallelDownloadChunks;

    for (int i = 0; i < kParallelDownloadChunks; i++) {
        const qint64 start = i * chunkSize;
        const qint64 end = qMin(start + chunkSize - 1, contentLength - 1);

        // Per-chunk write cursor — starts at `start` and advances as data
        // arrives. Heap-allocated so the lambda can capture a stable pointer.
        auto* writePos = new qint64(start);

        QNetworkRequest req((QUrl(streamUrl)));
        req.setRawHeader("User-Agent", ua);
        req.setRawHeader("Range",
                "bytes=" + QByteArray::number(start) + "-" +
                        QByteArray::number(end));
        req.setTransferTimeout(kDownloadTimeoutMs);
        applyYouTubeRequestAttributes(&req);

        QNetworkReply* reply = m_pNam->get(req);

        // Write each batch of incoming bytes at the correct file offset.
        // seek() is O(1) and safe to call interleaved with other chunks
        // because all readyRead signals fire serially on the UI thread.
        connect(reply, &QNetworkReply::readyRead, this, [reply, outFile, writePos, state]() {
            if (state->failed) {
                return;
            }
            const QByteArray data = reply->readAll();
            if (!data.isEmpty()) {
                outFile->seek(*writePos);
                outFile->write(data);
                *writePos += data.size();
            }
        });

        connect(reply,
                &QNetworkReply::finished,
                this,
                [this,
                        reply,
                        outFile,
                        writePos,
                        state,
                        outPath,
                        videoId,
                        onFailure]() {
                    reply->deleteLater();

                    // Drain any bytes that didn't trigger a readyRead.
                    if (!state->failed) {
                        const QByteArray tail = reply->readAll();
                        if (!tail.isEmpty()) {
                            outFile->seek(*writePos);
                            outFile->write(tail);
                            *writePos += tail.size();
                        }
                    }
                    delete writePos;

                    if (reply->error() != QNetworkReply::NoError &&
                            !state->failed) {
                        state->failed = true;
                        state->error = reply->errorString();
                    }

                    if (--state->remaining == 0) {
                        // Last chunk — finalize.
                        const bool ok = !state->failed;
                        const QString err = state->error;
                        delete state;

                        outFile->close();
                        if (!ok) {
                            QFile::remove(outFile->fileName());
                            delete outFile;
                            onFailure(
                                    tr("Chunked download failed: %1").arg(err));
                            return;
                        }
                        QFile::remove(outPath);
                        if (!outFile->rename(outPath)) {
                            const QString renErr = outFile->errorString();
                            QFile::remove(outFile->fileName());
                            delete outFile;
                            onFailure(tr("Cannot finalize %1: %2")
                                            .arg(outPath, renErr));
                            return;
                        }
                        delete outFile;
                        kLogger.info()
                                << "Chunked download completed:" << videoId
                                << "→" << outPath;
                        finalizeDownload(videoId, outPath);
                    }
                });
    }
}

// =============================================================================
// YouTube InnerTube API (secondary backend — all platforms incl. Android)
// =============================================================================

void YouTubeService::downloadViaInnerTube(
        const QString& videoId,
        const QString& cacheDir,
        const std::function<void(const QString&)>& onAllFailed) {
    // The InnerTube player API is what yt-dlp and ytdlnis use internally.
    // Sending a mobile/embedded client context causes YouTube to return plain
    // (non-cipher) adaptive stream URLs that are valid for ~6 hours. This
    // requires no external binary, no third-party proxy, and works on every Qt
    // platform including Android. We try several clients in turn (see
    // innerTubeClients()) and only fall back to yt-dlp once all of them fail.
    downloadViaInnerTubeClient(videoId, cacheDir, /*clientIdx=*/0, onAllFailed);
}

void YouTubeService::downloadViaInnerTubeClient(
        const QString& videoId,
        const QString& cacheDir,
        int clientIdx,
        const std::function<void(const QString&)>& onAllFailed) {
    const QVector<InnerTubeClient>& clients = innerTubeClients();
    if (clientIdx < 0 || clientIdx >= clients.size()) {
        onAllFailed(tr("All InnerTube clients failed"));
        return;
    }
    const InnerTubeClient& c = clients.at(clientIdx);
    const QString userAgent = QString::fromLatin1(c.userAgent);
#if defined(Q_OS_ANDROID)
    kLogger.info() << "[Android] downloadViaInnerTubeClient: videoId=" << videoId
                   << "client=" << c.clientName
                   << "clientIdx=" << clientIdx;
#endif

    // Helper to advance to the next client on any per-client failure.
    auto tryNext = [this, videoId, cacheDir, clientIdx, onAllFailed](
                           const QString& err) {
        const QVector<InnerTubeClient>& cl = innerTubeClients();
        if (clientIdx + 1 < cl.size()) {
            kLogger.warning() << "InnerTube client"
                              << cl.at(clientIdx).clientName << "failed for"
                              << videoId << ":" << err << "— trying next client";
            downloadViaInnerTubeClient(
                    videoId, cacheDir, clientIdx + 1, onAllFailed);
        } else {
            onAllFailed(err);
        }
    };

    QUrl reqUrl(QStringLiteral("https://www.youtube.com/youtubei/v1/player"));
    // The API key is optional for these endpoints — the client context is what
    // identifies the request. Only append it when one is configured, and never
    // for clients (e.g. ANDROID_VR) that authenticate purely via context.
    if (c.apiKey[0] != '\0') {
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("key"), QString::fromLatin1(c.apiKey));
        reqUrl.setQuery(query);
    }

    QJsonObject client = innerTubeClientContext(c);
    QJsonObject context;
    context.insert(QStringLiteral("client"), client);
    QJsonObject body;
    body.insert(QStringLiteral("videoId"), videoId);
    body.insert(QStringLiteral("context"), context);
    // Required for the embedded TV client to consider the video playable.
    body.insert(QStringLiteral("contentCheckOk"), true);
    body.insert(QStringLiteral("racyCheckOk"), true);

    QNetworkRequest req(reqUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader,
            QStringLiteral("application/json"));
    req.setRawHeader("User-Agent", userAgent.toUtf8());
    req.setRawHeader("X-YouTube-Client-Name", QByteArray(c.clientNameId));
    req.setRawHeader("X-YouTube-Client-Version", QByteArray(c.clientVersion));
    if (!m_visitorData.isEmpty()) {
        req.setRawHeader("X-Goog-Visitor-Id", m_visitorData.toUtf8());
    }
    req.setTransferTimeout(kDownloadTimeoutMs);
    applyYouTubeRequestAttributes(&req);
    applyBrowserFingerprint(&req, c.clientNameId);

    // Check for bot-flag recovery before each request attempt.
    maybeRecoverFromBotFlag();

    // If we've been bot-flagged, skip InnerTube entirely and go straight to
    // yt-dlp which has its own sophisticated anti-detection mechanisms.
    if (m_botFlagActive) {
        kLogger.info() << "Bot flag active — skipping InnerTube download for"
                       << videoId;
        onAllFailed(tr("Bot detection active — using yt-dlp fallback"));
        return;
    }
    if (shouldThrottleRequest()) {
        // Defer rather than immediately trying the next client — advancing to
        // the next client is pointless while still rate-limited (the window
        // has not moved at all). Wait for the minimum gap to clear, then retry
        // the same client. Only one deferred retry per client; if still
        // throttled after waiting, fall through to the next client.
        const int retryMs = kMaxJitterMs + 100;
        kLogger.info() << "Throttled: deferring InnerTube download for"
                       << videoId << "by" << retryMs << "ms"
                       << "(client" << clientIdx << ")";
        QPointer<YouTubeService> guard(this);
        QTimer::singleShot(retryMs,
                this,
                [guard, videoId, cacheDir, clientIdx, onAllFailed]() {
                    if (!guard) {
                        return;
                    }
                    // Retry same client once; if still throttled tryNext fires.
                    if (guard->shouldThrottleRequest()) {
                        guard->downloadViaInnerTubeClient(
                                videoId, cacheDir, clientIdx + 1, onAllFailed);
                    } else {
                        guard->downloadViaInnerTubeClient(
                                videoId, cacheDir, clientIdx, onAllFailed);
                    }
                });
        return;
    }
    recordRequest();

    QNetworkReply* reply = m_pNam->post(
            req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, videoId, cacheDir, userAgent, tryNext, onAllFailed]() {
                reply->deleteLater();
                const int httpStatus = reply->attribute(
                                                    QNetworkRequest::HttpStatusCodeAttribute)
                                               .toInt();
                if (reply->error() != QNetworkReply::NoError) {
                    const QByteArray rawBody = reply->readAll();
                    const QJsonObject errRoot =
                            QJsonDocument::fromJson(rawBody).object();
                    if (detectBotFlagging(httpStatus, errRoot, rawBody)) {
                        // Bot flagged — skip all remaining clients, go to yt-dlp.
                        onAllFailed(tr("Bot detection triggered (HTTP %1)")
                                        .arg(httpStatus));
                        return;
                    }
                    tryNext(tr("InnerTube API failed: %1")
                                    .arg(reply->errorString()));
                    return;
                }
                const QByteArray rawBody = reply->readAll();
                const QJsonObject root =
                        QJsonDocument::fromJson(rawBody).object();
                // Check for bot flagging before processing the response.
                if (detectBotFlagging(httpStatus, root, rawBody)) {
                    onAllFailed(tr("Bot detection triggered on player response"));
                    return;
                }
                // Check playability before touching streamingData.
                const QJsonObject playability =
                        root.value(QStringLiteral("playabilityStatus")).toObject();
                const QString status =
                        playability.value(QStringLiteral("status")).toString();
                if (status != QStringLiteral("OK")) {
                    const QString reason =
                            playability.value(QStringLiteral("reason")).toString();
                    tryNext(tr("Video not playable: %1")
                                    .arg(reason.isEmpty() ? status : reason));
                    return;
                }
                const QJsonArray adaptiveFormats =
                        root.value(QStringLiteral("streamingData"))
                                .toObject()
                                .value(QStringLiteral("adaptiveFormats"))
                                .toArray();
                if (adaptiveFormats.isEmpty()) {
                    tryNext(tr("InnerTube returned no adaptive formats"));
                    return;
                }
                // Convert InnerTube adaptiveFormats → Piped-compatible audioStreams
                // so we can reuse downloadAudioStream() directly. InnerTube embeds
                // the codec inside the mimeType ("audio/mp4; codecs=\"mp4a.40.2\"");
                // Piped exposes it as a dedicated "codec" field. Extract it here.
                QJsonArray audioStreams;
                for (const QJsonValue& v : adaptiveFormats) {
                    const QJsonObject f = v.toObject();
                    const QString mime =
                            f.value(QStringLiteral("mimeType")).toString();
                    if (!mime.startsWith(QStringLiteral("audio"))) {
                        continue; // skip video-only tracks
                    }
                    // Skip cipher-encrypted streams (those expose the params
                    // under "signatureCipher" instead of a ready-to-use "url").
                    const QString streamUrl =
                            f.value(QStringLiteral("url")).toString();
                    if (streamUrl.isEmpty()) {
                        continue;
                    }
                    // Extract codec string from e.g. "audio/mp4; codecs=\"mp4a.40.2\""
                    QString codec;
                    const int cIdx = mime.indexOf(QStringLiteral("codecs=\""));
                    if (cIdx >= 0) {
                        const int start = cIdx + 8; // length of 'codecs="'
                        const int end = mime.indexOf(QLatin1Char('"'), start);
                        if (end > start) {
                            codec = mime.mid(start, end - start);
                        }
                    }
                    QJsonObject stream;
                    stream.insert(QStringLiteral("url"), streamUrl);
                    stream.insert(QStringLiteral("mimeType"), mime);
                    stream.insert(QStringLiteral("codec"), codec);
                    stream.insert(QStringLiteral("bitrate"),
                            f.value(QStringLiteral("bitrate")).toInt());
                    // Forward the content length so downloadAudioStream() can
                    // decide whether to use parallel Range requests. InnerTube
                    // exposes this as a decimal string; store as a JSON number.
                    const QString clStr =
                            f.value(QStringLiteral("contentLength")).toString();
                    if (!clStr.isEmpty()) {
                        stream.insert(QStringLiteral("contentLength"),
                                QJsonValue(clStr.toLongLong()));
                    }
                    audioStreams.append(stream);
                }
                if (audioStreams.isEmpty()) {
                    tryNext(tr("InnerTube returned no audio-only streams"));
                    return;
                }
                kLogger.info() << "InnerTube: found" << audioStreams.size()
                               << "audio streams for" << videoId;
                // If the actual stream download fails (e.g. a 403 from the CDN
                // because this client now needs a poToken), fall through to the
                // next client rather than giving up.
                downloadAudioStream(
                        videoId, cacheDir, audioStreams, tryNext, userAgent);
            });
}

// =============================================================================
// yt-dlp (binary fallback — desktop and Termux-on-Android)
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
    QStringList args = {
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
    };
    // Add anti-bot arguments (cookies, extractor-args, sleep intervals).
    args.append(ytDlpAntiBotArgs());
    args.append(QStringLiteral("ytsearch%1:%2").arg(cap).arg(query));
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
    // Attempt a one-shot self-update of the bundled yt-dlp binary (desktop only).
    // This keeps the extractor current when YouTube rotates their API. The update
    // is fire-and-forget and only runs once per process — it does not block this
    // download (yt-dlp's self-update is atomic and fast on a good connection).
    maybeUpdateDesktopYtDlp();

    const QString outTemplate =
            QDir(cacheDir).filePath(QStringLiteral("%(id)s.%(ext)s"));
    const bool isPythonInterpreter =
            m_ytDlpPath.endsWith(QStringLiteral("python"), Qt::CaseInsensitive) ||
            m_ytDlpPath.endsWith(QStringLiteral("python3"), Qt::CaseInsensitive);
    QStringList args;
    if (isPythonInterpreter) {
        const QString ytDlpZip = QStandardPaths::locate(
                QStandardPaths::AppDataLocation,
                QStringLiteral("yt-dlp/__main__.py"),
                QStandardPaths::LocateFile);
        if (ytDlpZip.isEmpty()) {
            Q_EMIT downloadFailed(videoId,
                    tr("Python found (%1) but no bundled yt-dlp package").arg(m_ytDlpPath));
            return;
        }
        args.append(ytDlpZip);
    }
    args.append({
            QStringLiteral("-f"),
            QStringLiteral("bestaudio"),
            QStringLiteral("--no-playlist"),
            QStringLiteral("--no-warnings"),
            QStringLiteral("--no-progress"),
            QStringLiteral("--no-cache-dir"),
            QStringLiteral("--ignore-config"),
            QStringLiteral("--no-mtime"),
            // Download audio in parallel fragments for faster throughput on
            // good connections (yt-dlp splits the stream into 4 concurrent
            // range requests and reassembles on disk).
            QStringLiteral("--concurrent-fragments"),
            QStringLiteral("4"),
            QStringLiteral("-o"),
            outTemplate,
            QStringLiteral("--print"),
            QStringLiteral("after_move:filepath"),
    });
    // Add anti-bot arguments (cookies, extractor-args, sleep intervals).
    args.append(ytDlpAntiBotArgs());
    args.append({
            QStringLiteral("--"),
            QStringLiteral("https://www.youtube.com/watch?v=") + videoId,
    });
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
                                f.endsWith(QStringLiteral(".sponsorblock.json")) ||
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

#if defined(Q_OS_ANDROID) && defined(HAVE_YTDLP_ANDROID)
// =============================================================================
// Bundled youtubedl-android (JNI-based, no external dependencies)
// =============================================================================

// Whether we've already attempted to self-update the bundled yt-dlp this
// process. The update is a one-shot best-effort network call; we don't want to
// repeat it on every download.
namespace {
std::atomic<bool> s_ytdlpUpdateAttempted{false};
} // namespace

void YouTubeService::downloadViaAndroidBundled(
        const QString& videoId, const QString& cacheDir) {
    kLogger.info() << "[Android] downloadViaAndroidBundled: videoId=" << videoId
                   << "cacheDir=" << cacheDir;
    // Run the JNI download on a worker thread so we don't block the UI.
    // Use a QPointer to guard against YouTubeService being destroyed
    // before the thread completes.
    QPointer<YouTubeService> guard(this);
    QThread* thread = QThread::create([guard, videoId, cacheDir]() {
        QJniObject context = QNativeInterface::QAndroidApplication::context();
        if (!context.isValid()) {
            kLogger.warning() << "[Android] downloadViaAndroidBundled:"
                              << "QAndroidApplication::context() invalid for"
                              << videoId;
            if (guard)
                Q_EMIT guard->downloadFailed(videoId, "No Android context");
            return;
        }

        // Get the YoutubeDL singleton and initialize it.
        QJniObject ytdl = QJniObject::callStaticObjectMethod(
                "com/yausername/youtubedl_android/YoutubeDL",
                "getInstance",
                "()Lcom/yausername/youtubedl_android/YoutubeDL;");

        if (!ytdl.isValid()) {
            kLogger.warning() << "[Android] downloadViaAndroidBundled:"
                              << "YoutubeDL.getInstance() returned invalid for"
                              << videoId;
            if (guard)
                Q_EMIT guard->downloadFailed(
                        videoId, "Bundled yt-dlp (youtubedl-android) not available");
            return;
        }

        // Initialize with the Android context (required before first download).
        kLogger.info() << "[Android] downloadViaAndroidBundled:"
                       << "initializing YoutubeDL for" << videoId;
        QJniEnvironment env;
        ytdl.callMethod<void>("init",
                "(Landroid/content/Context;)V",
                context.object());
        if (env.checkAndClearExceptions()) {
            kLogger.warning() << "[Android] downloadViaAndroidBundled:"
                              << "YoutubeDL.init() threw exception for"
                              << videoId;
            if (guard)
                Q_EMIT guard->downloadFailed(
                        videoId, "yt-dlp init failed (youtubedl-android)");
            return;
        }

        // The yt-dlp packaged inside the AAR is whatever version was bundled at
        // the library's build time and goes stale quickly — YouTube regularly
        // breaks older extractors, which is the usual reason downloads stop
        // working ("gave us songs at some point and now nothing"). Update to the
        // latest stable yt-dlp release before downloading. Best-effort and
        // one-shot per process: if there's no network or GitHub is unreachable
        // we simply proceed with the bundled version.
        if (!s_ytdlpUpdateAttempted.exchange(true)) {
            QJniObject channel = QJniObject::getStaticObjectField(
                    "com/yausername/youtubedl_android/YoutubeDL$UpdateChannel$STABLE",
                    "INSTANCE",
                    "Lcom/yausername/youtubedl_android/YoutubeDL$UpdateChannel$STABLE;");
            if (channel.isValid()) {
                kLogger.info() << "Updating bundled yt-dlp to latest stable…";
                ytdl.callObjectMethod("updateYoutubeDL",
                        "(Landroid/content/Context;"
                        "Lcom/yausername/youtubedl_android/YoutubeDL$UpdateChannel;)"
                        "Lcom/yausername/youtubedl_android/YoutubeDL$UpdateStatus;",
                        context.object(),
                        channel.object());
            }
            // A failed update (network/rate-limit/parse) must not block the
            // download — clear any pending Java exception and carry on.
            env.checkAndClearExceptions();
        }

        // Build the request: URL + options.
        QString videoUrl = "https://www.youtube.com/watch?v=" + videoId;
        QString outputTemplate = QDir(cacheDir).filePath("%(id)s.%(ext)s");

        QJniObject request("com/yausername/youtubedl_android/YoutubeDLRequest",
                "(Ljava/lang/String;)V",
                QJniObject::fromString(videoUrl).object());

        // Add download options.
        request.callMethod<QJniObject>("addOption",
                "(Ljava/lang/String;Ljava/lang/String;)Lcom/yausername/youtubedl_android/"
                "YoutubeDLRequest;",
                QJniObject::fromString("-f").object(),
                QJniObject::fromString("bestaudio").object());
        request.callMethod<QJniObject>("addOption",
                "(Ljava/lang/String;Ljava/lang/String;)Lcom/yausername/youtubedl_android/"
                "YoutubeDLRequest;",
                QJniObject::fromString("-o").object(),
                QJniObject::fromString(outputTemplate).object());
        request.callMethod<QJniObject>("addOption",
                "(Ljava/lang/String;)Lcom/yausername/youtubedl_android/YoutubeDLRequest;",
                QJniObject::fromString("--no-playlist").object());
        request.callMethod<QJniObject>("addOption",
                "(Ljava/lang/String;)Lcom/yausername/youtubedl_android/YoutubeDLRequest;",
                QJniObject::fromString("--no-warnings").object());
        request.callMethod<QJniObject>("addOption",
                "(Ljava/lang/String;)Lcom/yausername/youtubedl_android/YoutubeDLRequest;",
                QJniObject::fromString("--no-cache-dir").object());
        request.callMethod<QJniObject>("addOption",
                "(Ljava/lang/String;)Lcom/yausername/youtubedl_android/YoutubeDLRequest;",
                QJniObject::fromString("--ignore-config").object());
        request.callMethod<QJniObject>("addOption",
                "(Ljava/lang/String;)Lcom/yausername/youtubedl_android/YoutubeDLRequest;",
                QJniObject::fromString("--no-mtime").object());

        // Execute the download (blocks this thread until complete). The
        // youtubedl-android API method is execute(YoutubeDLRequest) (with a
        // YoutubeDLResponse return) — the previous code called a non-existent
        // "exec" method, so every bundled download threw NoSuchMethodError and
        // silently failed. This is the core download fix.
        kLogger.info() << "[Android] downloadViaAndroidBundled:"
                       << "executing yt-dlp request for" << videoId;
        QJniObject response = ytdl.callObjectMethod(
                "execute",
                "(Lcom/yausername/youtubedl_android/YoutubeDLRequest;)"
                "Lcom/yausername/youtubedl_android/YoutubeDLResponse;",
                request.object());

        if (env.checkAndClearExceptions() || !response.isValid()) {
            kLogger.warning() << "[Android] downloadViaAndroidBundled:"
                              << "execute() failed or returned invalid for"
                              << videoId;
            if (guard)
                Q_EMIT guard->downloadFailed(videoId,
                        "yt-dlp download failed (youtubedl-android)");
            return;
        }

        // Extract the output file path. yt-dlp wrote it as "<videoId>.<ext>"
        // inside cacheDir (the -o template above). The exact extension depends
        // on the chosen bestaudio format (m4a / webm / opus / …), so locate the
        // file by id rather than trusting the process stdout — getOut() returns
        // yt-dlp's log text, not a path, which is why downloads that did run
        // were still reported as "file not found".
        if (!guard)
            return; // Service destroyed, nothing more to do

        QString outputPath;
        const QFileInfoList matches = QDir(cacheDir).entryInfoList(
                QStringList{videoId + QStringLiteral(".*")}, QDir::Files);
        for (const QFileInfo& fi : matches) {
            const QString suffix = fi.suffix().toLower();
            // Skip yt-dlp's transient/sidecar artefacts; keep the real audio.
            if (suffix == QStringLiteral("part") ||
                    suffix == QStringLiteral("ytdl") ||
                    suffix == QStringLiteral("json")) {
                continue;
            }
            outputPath = fi.absoluteFilePath();
            break;
        }

        if (!outputPath.isEmpty() && QFileInfo::exists(outputPath)) {
            kLogger.info() << "[Android] downloadViaAndroidBundled:"
                           << "download complete for" << videoId
                           << "→" << outputPath;
            // Route through finalizeDownload for SponsorBlock/post-processing,
            // same as the desktop path. Use invokeMethod to marshal back to
            // the object's thread.
            QMetaObject::invokeMethod(guard, [guard, videoId, outputPath]() {
                if (guard) guard->finalizeDownload(videoId, outputPath); }, Qt::QueuedConnection);
        } else {
            kLogger.warning() << "[Android] downloadViaAndroidBundled:"
                              << "output file not found for" << videoId
                              << "in" << cacheDir
                              << "(matched" << matches.size() << "files)";
            Q_EMIT guard->downloadFailed(videoId,
                    "yt-dlp finished but output file not found in cache");
        }
    });

    thread->setParent(this);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}
#endif // Q_OS_ANDROID && HAVE_YTDLP_ANDROID

// =============================================================================
// Bot-detection, rate limiting, session persistence, and yt-dlp self-update
// =============================================================================

bool YouTubeService::detectBotFlagging(int httpStatus,
        const QJsonObject& root,
        const QByteArray& rawBody) {
    // HTTP 429 — explicit rate limit.
    if (httpStatus == 429) {
        activateBotFlag(tr("YouTube rate limit (HTTP 429) — switching to yt-dlp"));
        return true;
    }

    // Check playabilityStatus for bot indicators.
    const QJsonObject playability =
            root.value(QStringLiteral("playabilityStatus")).toObject();
    const QString status = playability.value(QStringLiteral("status")).toString();
    const QString reason = playability.value(QStringLiteral("reason")).toString();

    // LOGIN_REQUIRED — YouTube wants authentication, typically bot-flagging.
    if (status == QStringLiteral("LOGIN_REQUIRED")) {
        activateBotFlag(tr("YouTube requires login (bot detection) — switching to yt-dlp"));
        return true;
    }

    // Check reason text for common bot-detection phrases.
    const QString reasonLower = reason.toLower();
    if (reasonLower.contains(QStringLiteral("sign in")) ||
            reasonLower.contains(QStringLiteral("bot")) ||
            reasonLower.contains(QStringLiteral("unusual traffic")) ||
            reasonLower.contains(QStringLiteral("automated")) ||
            reasonLower.contains(QStringLiteral("verify")) ||
            reasonLower.contains(QStringLiteral("confirm"))) {
        activateBotFlag(
                tr("YouTube bot detection: %1 — switching to yt-dlp").arg(reason));
        return true;
    }

    // Check for consent/captcha challenge in raw HTML body (can happen when
    // YouTube serves a web consent page instead of JSON). Only scan the first
    // 4 KB — consent/captcha markers appear near the top of the page, and
    // calling toLower() on a multi-MB response wastes time and memory.
    if (root.isEmpty() && rawBody.size() > 100) {
        const QByteArray lower = rawBody.left(4096).toLower();
        if (lower.contains("consent.youtube") ||
                lower.contains("recaptcha") ||
                lower.contains("captcha") ||
                lower.contains("before you continue") ||
                lower.contains("confirm your identity") ||
                lower.contains("unusual traffic")) {
            activateBotFlag(
                    tr("YouTube consent/captcha challenge — switching to yt-dlp"));
            return true;
        }
    }

    // Extract visitorData from response for future requests (reduces bot flagging).
    const QJsonObject responseContext =
            root.value(QStringLiteral("responseContext")).toObject();
    const QString visitorData =
            responseContext.value(QStringLiteral("visitorData")).toString();
    if (!visitorData.isEmpty() && visitorData != m_visitorData) {
        m_visitorData = visitorData;
        saveSessionState();
        kLogger.debug() << "Updated visitorData from InnerTube response";
    }

    return false;
}

void YouTubeService::activateBotFlag(const QString& detail) {
    m_botFlagActive = true;
    m_botFlagCount++;
    m_botFlagTimestamp = m_rateLimitTimer.elapsed();

    // Exponential backoff: each consecutive flag doubles the cooldown.
    if (m_botFlagBackoffMs == 0) {
        m_botFlagBackoffMs = kBotFlagInitialBackoffMs;
    } else {
        m_botFlagBackoffMs = qMin(
                m_botFlagBackoffMs * kBotFlagBackoffMultiplier,
                kBotFlagMaxBackoffMs);
    }

    kLogger.warning() << detail
                      << "| backoff:" << m_botFlagBackoffMs << "ms"
                      << "| consecutive flags:" << m_botFlagCount;
    saveSessionState();
    Q_EMIT botFlagged(detail);
}

void YouTubeService::maybeRecoverFromBotFlag() {
    if (!m_botFlagActive) {
        return;
    }
    const qint64 elapsed = m_rateLimitTimer.elapsed() - m_botFlagTimestamp;
    if (elapsed >= m_botFlagBackoffMs) {
        kLogger.info() << "Bot-flag cooldown expired after"
                       << elapsed << "ms (backoff was"
                       << m_botFlagBackoffMs << "ms) — recovering";
        m_botFlagActive = false;
        // Don't reset backoff or count — they persist so subsequent flags
        // still escalate. They only reset on app restart.
    }
}

bool YouTubeService::shouldThrottleRequest() {
    // All callers are on the main/UI thread — no mutex needed.
    const qint64 now = m_rateLimitTimer.elapsed();

    // Evict timestamps older than the window.
    while (!m_requestTimestamps.isEmpty() &&
            (now - m_requestTimestamps.first()) > kRateLimitWindowMs) {
        m_requestTimestamps.removeFirst();
    }

    // Check burst limit.
    if (m_requestTimestamps.size() >= kRateLimitBurst) {
        kLogger.info() << "Rate limit: burst limit reached ("
                       << m_requestTimestamps.size() << "/" << kRateLimitBurst
                       << " in " << kRateLimitWindowMs << "ms window)";
        return true;
    }

    // Check minimum inter-request gap with jitter.
    const int minGap = jitterDelayMs();
    if (!m_requestTimestamps.isEmpty() &&
            (now - m_requestTimestamps.last()) < minGap) {
        return true;
    }

    return false;
}

void YouTubeService::recordRequest() {
    // All callers are on the main/UI thread — no mutex needed.
    m_requestTimestamps.append(m_rateLimitTimer.elapsed());
}

int YouTubeService::jitterDelayMs() const {
    // Random delay in [kMinRequestGapMs, kMaxJitterMs] to look more human.
    return kMinRequestGapMs +
            QRandomGenerator::global()->bounded(kMaxJitterMs - kMinRequestGapMs);
}

int YouTubeService::randomClientStartIndex(int clientCount) const {
    if (clientCount <= 1) {
        return 0;
    }
    return QRandomGenerator::global()->bounded(clientCount);
}

void YouTubeService::applyBrowserFingerprint(QNetworkRequest* req,
        const char* clientNameId) const {
    // Add headers that a real browser/app would send to make the request look
    // less automated. These are cosmetic but reduce heuristic bot scoring.
    const QByteArray nameId(clientNameId);

    // For "WEB" client (nameId "1") add browser-like headers.
    if (nameId == "1" || nameId == "85") {
        req->setRawHeader("Accept",
                "text/html,application/xhtml+xml,application/xml;q=0.9,"
                "image/avif,image/webp,image/apng,*/*;q=0.8");
        req->setRawHeader("Accept-Language", "en-US,en;q=0.9,el;q=0.8");
        req->setRawHeader("Sec-Ch-Ua",
                "\"Chromium\";v=\"126\", \"Not=A?Brand\";v=\"8\"");
        req->setRawHeader("Sec-Ch-Ua-Mobile", "?0");
        req->setRawHeader("Sec-Ch-Ua-Platform", "\"Windows\"");
        req->setRawHeader("Sec-Fetch-Dest", "empty");
        req->setRawHeader("Sec-Fetch-Mode", "same-origin");
        req->setRawHeader("Sec-Fetch-Site", "same-origin");
        req->setRawHeader("Origin", "https://www.youtube.com");
        req->setRawHeader("Referer", "https://www.youtube.com/");
    } else if (nameId == "28" || nameId == "5") {
        // Mobile/VR clients — set Origin and Accept-Language.
        req->setRawHeader("Accept-Language", "en-US,en;q=0.9");
        req->setRawHeader("Origin", "https://www.youtube.com");
    }

    // X-Origin is sent by the official JS client and mirrors Origin.
    req->setRawHeader("X-Origin", "https://www.youtube.com");
}

void YouTubeService::setupCookieJar() {
    // Use a standard cookie jar that shares cookies across all requests to
    // YouTube endpoints. This maintains session cookies (CONSENT, GPS, VISITOR)
    // which YouTube uses to identify a session — lacking them looks bot-like.
    // Qt's default QNetworkCookieJar stores cookies in-memory only; we persist
    // visitorData separately via QSettings (the most impactful field).
    m_pNam->setCookieJar(new QNetworkCookieJar(m_pNam));

    // Set initial CONSENT cookie to bypass the EU consent screen that causes
    // YouTube to serve HTML instead of JSON. Value "PENDING+999" is a common
    // workaround used by yt-dlp and invidious.
    QNetworkCookie consent;
    consent.setName("CONSENT");
    consent.setValue("PENDING+999");
    consent.setDomain(".youtube.com");
    consent.setPath("/");
    m_pNam->cookieJar()->insertCookie(consent);

    // SOCS cookie — newer consent bypass that YouTube checks in some regions.
    QNetworkCookie socs;
    socs.setName("SOCS");
    socs.setValue("CAESEwgDEgk2MjQxMTY0MTkaAmVuIAEaBgiA_ZMZBQ");
    socs.setDomain(".youtube.com");
    socs.setPath("/");
    m_pNam->cookieJar()->insertCookie(socs);
}

void YouTubeService::saveSessionState() {
    QSettings settings;
    settings.beginGroup(kSettingsGroupYouTube);
    if (!m_visitorData.isEmpty()) {
        settings.setValue(kSettingsVisitorData, m_visitorData);
    }
    settings.setValue(kSettingsBotFlagCount, m_botFlagCount);
    settings.endGroup();
}

void YouTubeService::loadSessionState() {
    QSettings settings;
    settings.beginGroup(kSettingsGroupYouTube);
    m_visitorData = settings.value(kSettingsVisitorData).toString();
    m_botFlagCount = settings.value(kSettingsBotFlagCount, 0).toInt();
    settings.endGroup();

    if (!m_visitorData.isEmpty()) {
        kLogger.info() << "Restored persisted visitorData from previous session";
    }
    if (m_botFlagCount > 0) {
        kLogger.info() << "Previous session had" << m_botFlagCount
                       << "bot-flag events — starting with elevated caution";
        // Start with a proportional backoff based on prior history.
        m_botFlagBackoffMs = qMin(
                kBotFlagInitialBackoffMs * (1 << qMin(m_botFlagCount - 1, 6)),
                kBotFlagMaxBackoffMs);
    }
}

QStringList YouTubeService::ytDlpAntiBotArgs() const {
    QStringList args;

    // 1. Use yt-dlp's built-in browser cookie extraction when available.
    //    This passes the user's actual YouTube session cookies (if they're
    //    logged into YouTube in their browser), which virtually eliminates
    //    bot detection since yt-dlp then looks like the user's browser.
#if !defined(Q_OS_ANDROID)
    // Try common browsers in preference order. yt-dlp will silently fall back
    // if the specified browser isn't installed or has no cookies.
    args << QStringLiteral("--cookies-from-browser")
         << QStringLiteral("firefox");
#endif

    // 2. Tell yt-dlp to use the same ANDROID_VR client we use for InnerTube.
    //    This avoids the WEB client which is more heavily rate-limited.
    args << QStringLiteral("--extractor-args")
         << QStringLiteral("youtube:player_client=android_vr,mediaconnect");

    // 3. Sleep between downloads in a playlist scenario only. For single-video
    //    downloads these are no-ops. Do NOT use --sleep-requests here: it
    //    inserts a forced pause between every extraction API call (including
    //    the initial video-info fetch), adding 1–3 s of latency to every
    //    yt-dlp download for no bot-avoidance benefit on single-item requests.
    args << QStringLiteral("--sleep-interval")
         << QStringLiteral("2");
    args << QStringLiteral("--max-sleep-interval")
         << QStringLiteral("5");

    // 4. User-agent override: use a recent Chrome UA to avoid being flagged
    //    by the default yt-dlp UA string.
    args << QStringLiteral("--user-agent")
         << QStringLiteral(
                    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
                    "AppleWebKit/537.36 (KHTML, like Gecko) "
                    "Chrome/126.0.0.0 Safari/537.36");

    // 5. Retry on HTTP errors (including 429). yt-dlp will wait and retry.
    args << QStringLiteral("--retries") << QStringLiteral("3");
    args << QStringLiteral("--retry-sleep") << QStringLiteral("exp=1:5:30");

    return args;
}

// Desktop yt-dlp self-update: runs `yt-dlp --update-to stable` once per process.
// This keeps the bundled binary current so downloads don't break when YouTube
// rotates their extractor API — the same problem the Android path solves via
// youtubedl-android's updateYoutubeDL(). On desktop the PyInstaller binary
// supports in-place self-update, so we just shell out to it.
namespace {
std::atomic<bool> s_desktopYtdlpUpdateAttempted{false};
} // namespace

void YouTubeService::maybeUpdateDesktopYtDlp() {
#if defined(Q_OS_ANDROID)
    return; // Android uses the JNI path in downloadViaAndroidBundled()
#endif
    if (m_ytDlpPath.isEmpty() ||
            m_ytDlpPath == QStringLiteral("android-bundled")) {
        return;
    }
    if (s_desktopYtdlpUpdateAttempted.exchange(true)) {
        return; // Already attempted this process lifetime.
    }

    kLogger.info() << "Attempting one-shot yt-dlp self-update from"
                   << m_ytDlpPath;

    // Fire-and-forget: we don't block the download on the update completing.
    // If it succeeds, subsequent downloads benefit. If it fails, we still
    // proceed with the existing binary version.
    const QStringList args = {
            QStringLiteral("--update-to"),
            QStringLiteral("stable"),
            QStringLiteral("--no-cache-dir"),
    };
    runYtDlp(
            args,
            kYtDlpUpdateTimeoutMs,
            [](const QByteArray& out) {
                kLogger.info() << "yt-dlp self-update completed:"
                               << QString::fromUtf8(out).trimmed().left(200);
            },
            [](const QString& err) {
                kLogger.warning() << "yt-dlp self-update failed (non-fatal):"
                                  << err;
            });
}

// =============================================================================
// Shared post-download chain: SponsorBlock fetch → in-place cut → emit
// =============================================================================

void YouTubeService::finalizeDownload(const QString& videoId, const QString& outPath) {
    // Helper lambda: given segments (possibly empty), do the cut on a
    // background thread and emit downloadFinished when done. We run
    // cutAudioRanges() off the UI thread so the app stays responsive while
    // FFmpeg rewrites the audio file (can take 1–3 s for a long track).
    // QThreadPool::globalInstance() reuses pre-warmed OS threads, eliminating
    // the ~5-20 ms thread-creation overhead of the previous QThread::create().
    auto doFinalize = [this, videoId, outPath](const QList<SponsorSegment>& segments) {
        // Capture a copy of segments for the worker (QList is CoW, cheap).
        QThreadPool::globalInstance()->start(
                [this, videoId, outPath, segments]() {
                    if (!segments.isEmpty()) {
                        // Physically cut non-music segments out of the file so that
                        // duration, BPM, and waveform all reflect the clean music
                        // length. This is strongly preferred over skip-at-playback
                        // because it works correctly with waveform zoom, looping, and
                        // cue points.
                        const bool cut = cutAudioRanges(outPath, segments);
                        if (!cut) {
                            // Cutting failed (e.g. mid-file packet boundary on opus).
                            // Write a sidecar in the canonical SponsorBlockController
                            // format so it can fall back to skip-at-playback. The file
                            // must be named VIDEOID.sponsorblock.json and use the
                            // {"segment":[start,end], "category":"..."} schema — NOT
                            // the old {start,end} format that was written as
                            // VIDEOID.m4a.sponsor.json (those were silently ignored).
                            const QString sidecarPath =
                                    QDir(QFileInfo(outPath).absolutePath())
                                            .filePath(videoId +
                                                    QStringLiteral(
                                                            ".sponsorblock.json"));
                            QFile sidecar(sidecarPath);
                            if (sidecar.open(
                                        QIODevice::WriteOnly | QIODevice::Truncate)) {
                                QJsonArray arr;
                                for (const auto& s : segments) {
                                    QJsonObject o;
                                    QJsonArray seg;
                                    seg.append(s.start);
                                    seg.append(s.end);
                                    o.insert(QStringLiteral("segment"), seg);
                                    o.insert(QStringLiteral("category"), s.category);
                                    arr.append(o);
                                }
                                sidecar.write(QJsonDocument(arr).toJson(
                                        QJsonDocument::Compact));
                                kLogger.info()
                                        << "Wrote SponsorBlock sidecar for"
                                        << videoId << "with" << segments.size()
                                        << "segment(s)";
                            }
                        } else {
                            kLogger.info() << "SponsorBlock: cut" << segments.size()
                                           << "non-music segment(s) from" << videoId;
                        }
                    }
                    // Signal from the worker — Qt queues it to the main thread
                    // automatically because the signal target lives there.
                    QMetaObject::invokeMethod(
                            this,
                            [this, videoId, outPath]() {
                                // Clean up the prefetch cache entry now that
                                // finalization is complete.
                                m_sponsorPrefetchCache.remove(videoId);
                                Q_EMIT downloadFinished(videoId, outPath);
                            },
                            Qt::QueuedConnection);
                });
    };

    // If we started a SponsorBlock prefetch in downloadVideo() the result may
    // already be sitting in the cache — use it immediately without any extra
    // network round-trip. Otherwise, if the prefetch is still in-flight,
    // register as a waiter so we get called as soon as it completes. If there
    // was no prefetch at all (e.g. a path that bypassed downloadVideo) fall
    // back to a direct fetch.
    if (m_sponsorPrefetchCache.contains(videoId)) {
        // Segments arrived before the download finished — zero wait.
        doFinalize(m_sponsorPrefetchCache.value(videoId));
    } else if (m_sponsorPrefetchWaiters.contains(videoId)) {
        // Prefetch is still in-flight; queue ourselves behind it.
        m_sponsorPrefetchWaiters[videoId].append(doFinalize);
    } else {
        // No prefetch was started — fetch now (legacy / direct path).
        fetchSponsorSegmentsInternal(videoId, doFinalize);
    }
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
    applyYouTubeRequestAttributes(&request);
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
