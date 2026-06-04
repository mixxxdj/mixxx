#include "library/youtube/youtubefeature.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTextBrowser>
#include <QTimer>
#include <QUrl>

#include "analyzer/analyzerscheduledtrack.h"
#include "library/basetrackcache.h"
#include "library/dao/trackschema.h"
#include "library/library.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/treeitem.h"
#include "library/treeitemmodel.h"
#include "library/youtube/youtubetrackmodel.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "preferences/configobject.h"
#include "track/track.h"
#include "track/trackref.h"
#include "util/file.h"
#include "util/logger.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarytextbrowser.h"

namespace {
const mixxx::Logger kLogger("YouTubeFeature");

#if defined(Q_OS_ANDROID)
constexpr int kSearchResultsMax = 50;
#else
constexpr int kSearchResultsMax = 100;
#endif

// Upper duration bound (seconds) for items shown in the trending/home feed.
// The Greek "top songs" feed otherwise mixes in hour-long "megamix" / "best
// of" compilations the user explicitly does not want — a DJ wants individual
// tracks, not a 1:47:00 mix. Items longer than this are dropped from the home
// feed only; an explicit user search is left untouched so long sets remain
// findable. Items with an unknown duration (0) are kept so we never hide a
// genuine song just because its length failed to parse.
constexpr int kTrendingMaxDurationSec = 15 * 60; // 15 minutes

// We tag the TreeItem `data` payload so activateChild() can tell apart
// "search result the user wants to load" from "already-downloaded track".
const QString kSearchPrefix = QStringLiteral("yt-search:");
const QString kCachedPrefix = QStringLiteral("yt-cached:");
const QString kGenrePrefix = QStringLiteral("yt-genre:");

// URL schemes used by the clickable links rendered into the home-pane HTML.
// Plain `<li>title</li>` would only render text — the user reported "songs..
// just text. Nothing to click, play". Wrapping each entry in <a href> +
// dispatching anchorClicked() to the existing requestDownload / load paths
// gives them a real action with no extra UI surface.
const QString kHomePlayScheme = QStringLiteral("ytplay");
const QString kHomeCachedScheme = QStringLiteral("ytcached");
const QString kHomeAutoAnalyzeScheme = QStringLiteral("ytautoanalyze");
const QString kHomeGenreScheme = QStringLiteral("ytgenre");
const QString kAutoAnalyzePayload = QStringLiteral("yt-auto-analyze");

// DJ-oriented music genres shown in the sidebar "Genres" node as a fallback
// when auto-discovery from YouTube Music fails. Includes both international
// electronic/dance genres and Greek-specific categories the user explicitly
// requested. These are replaced at runtime by fetchMusicGenres() results.
const QStringList kDefaultGenres = {
        // Greek genres
        QStringLiteral("Καψούρα"),
        QStringLiteral("Τραπίλες"),
        QStringLiteral("Ελληνικά τραγούδια"),
        QStringLiteral("Λαϊκά"),
        QStringLiteral("Ρεμπέτικα"),
        QStringLiteral("Έντεχνα"),
        QStringLiteral("Ελληνικό ραπ"),
        QStringLiteral("Σκυλάδικα"),
        QStringLiteral("Ζεϊμπέκικα"),
        QStringLiteral("Νησιώτικα"),
        // International DJ genres
        QStringLiteral("House"),
        QStringLiteral("Deep House"),
        QStringLiteral("Tech House"),
        QStringLiteral("Techno"),
        QStringLiteral("Trance"),
        QStringLiteral("Drum & Bass"),
        QStringLiteral("Hip Hop"),
        QStringLiteral("R&B"),
        QStringLiteral("Pop"),
        QStringLiteral("Reggaeton"),
        QStringLiteral("Afrobeats"),
        QStringLiteral("Amapiano"),
        QStringLiteral("Latin"),
        QStringLiteral("Lo-Fi"),
        QStringLiteral("EDM"),
};

/// Returns true for sidecar files that accompany a downloaded YouTube audio
/// file and should be excluded when looking for the actual audio bytes.
/// Matches: .info.json (yt-dlp metadata), .sponsor.json (SponsorBlock data),
///          .part (incomplete download temp file).
bool isYouTubeSidecarFile(const QString& name) {
    return name.endsWith(QStringLiteral(".info.json")) ||
            name.endsWith(QStringLiteral(".sponsor.json")) ||
            name.endsWith(QStringLiteral(".part"));
}

struct TrackDisplayMetadata {
    QString artist;
    QString title;
};

QString cleanYouTubeSongTitle(const QString& input) {
    QString title = input.simplified();
    static const QRegularExpression kBracketedNoise(
            QStringLiteral(
                    R"(\s*[\[(](official\s*(music\s*)?video|official\s*audio|audio|lyrics?|lyric\s*video|visuali[sz]er|music\s*video|hd|hq|4k)[\])]\s*)"),
            QRegularExpression::CaseInsensitiveOption);
    title.remove(kBracketedNoise);
    static const QRegularExpression kTrailingNoise(
            QStringLiteral(
                    R"(\s*[-–—|]\s*(official\s*(music\s*)?video|official\s*audio|audio|lyrics?|lyric\s*video|visuali[sz]er|music\s*video|hd|hq|4k)\s*$)"),
            QRegularExpression::CaseInsensitiveOption);
    title.remove(kTrailingNoise);
    return title.simplified();
}

QString cleanYouTubeArtist(const QString& input) {
    QString artist = input.simplified();
    static const QRegularExpression kTopicSuffix(
            QStringLiteral(R"(\s*-\s*Topic\s*$)"),
            QRegularExpression::CaseInsensitiveOption);
    artist.remove(kTopicSuffix);
    return artist.simplified();
}

TrackDisplayMetadata displayMetadataForVideo(
        const mixxx::YouTubeVideoInfo& info) {
    TrackDisplayMetadata metadata;
    const QString rawTitle = info.title.simplified();
    const QStringList separators = {
            QStringLiteral(" - "),
            QStringLiteral(" – "),
            QStringLiteral(" — "),
            QStringLiteral(" | "),
    };
    for (const QString& separator : separators) {
        const int separatorPos = rawTitle.indexOf(separator);
        if (separatorPos <= 0) {
            continue;
        }
        const QString artist = rawTitle.left(separatorPos).simplified();
        const QString title =
                rawTitle.mid(separatorPos + separator.size()).simplified();
        if (!artist.isEmpty() && !title.isEmpty()) {
            metadata.artist = cleanYouTubeArtist(artist);
            metadata.title = cleanYouTubeSongTitle(title);
            if (!metadata.artist.isEmpty() && !metadata.title.isEmpty()) {
                return metadata;
            }
        }
    }
    metadata.artist = cleanYouTubeArtist(info.uploader);
    metadata.title = cleanYouTubeSongTitle(rawTitle);
    if (metadata.title.isEmpty()) {
        metadata.title = info.id;
    }
    return metadata;
}

bool isValidYouTubeVideoId(const QString& videoId) {
    static const QRegularExpression kVideoIdPattern(
            QStringLiteral(R"(^[A-Za-z0-9_-]{11}$)"));
    return kVideoIdPattern.match(videoId).hasMatch();
}

QString displayLabelForVideo(const mixxx::YouTubeVideoInfo& info) {
    const TrackDisplayMetadata metadata = displayMetadataForVideo(info);
    if (metadata.artist.isEmpty()) {
        return metadata.title;
    }
    return metadata.artist + QStringLiteral(" - ") + metadata.title;
}

// Human-readable country name for display in the "Trending in <Country>"
// header. QLocale knows how to localize country names from the alpha-2 code,
// but the API names changed between Qt 5 (countryToString / Country) and
// Qt 6 (territoryToString / Territory) — Mixxx still supports both.
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
// Config key for an optional user-set trending-region override (Preferences UI
// / future config dialog).
const ConfigKey kCfgTrendingOverride(
        QStringLiteral("[YouTube]"), QStringLiteral("trending_region"));
const ConfigKey kCfgAutoAnalyzeResults(
        QStringLiteral("[YouTube]"), QStringLiteral("auto_analyze_results"));

// Project default region. Per project requirement: open on Greek top songs
// unless the user explicitly overrides it.
const QString kDefaultRegion = QStringLiteral("GR");
} // namespace

YouTubeFeature::YouTubeFeature(Library* pLibrary, UserSettingsPointer pConfig)
        : BaseExternalLibraryFeature(pLibrary, pConfig, "youtube"),
          m_pSidebarModel(make_parented<TreeItemModel>(this)),
          m_service(this) {
    // Build the persistent track model that backs the right-hand pane.
    // Mirrors ITunesFeature ctor exactly — same column set, same cache
    // wiring — so the standard WTrackTableView UI works out of the box
    // (sortable columns, drag-to-deck, right-click → Add to Auto DJ, …).
    QStringList columns = {
            QStringLiteral("id"),
            QStringLiteral("artist"),
            QStringLiteral("title"),
            QStringLiteral("album"),
            QStringLiteral("year"),
            QStringLiteral("genre"),
            QStringLiteral("tracknumber"),
            QStringLiteral("location"),
            QStringLiteral("comment"),
            QStringLiteral("duration"),
            QStringLiteral("bitrate"),
            QStringLiteral("bpm"),
            QStringLiteral("rating"),
    };
    QStringList searchColumns = {
            QStringLiteral("artist"),
            QStringLiteral("title"),
            QStringLiteral("album"),
            QStringLiteral("comment"),
    };
    m_pTrackCache = QSharedPointer<BaseTrackCache>::create(
            m_pLibrary->trackCollectionManager()->internalCollection(),
            QStringLiteral("youtube_library"),
            QStringLiteral("id"),
            columns,
            searchColumns,
            false);
    m_pTrackModel = new YouTubeTrackModel(this,
            m_pLibrary->trackCollectionManager(),
            m_pTrackCache);
    // setSearch("") primes the model so it actually issues its first
    // SELECT — without this the first activate() shows an empty pane even
    // when youtube_library has rows from a previous session.
    m_pTrackModel->setSearch(QString());

    // Per-view search box: typing in the search bar while the YouTube
    // pane is active fires YouTubeTrackModel::searchRequested → here →
    // a fresh YouTubeService search via the existing pipeline.
    connect(m_pTrackModel,
            &YouTubeTrackModel::searchRequested,
            this,
            &YouTubeFeature::searchAndActivate);
    connect(m_pLibrary,
            &Library::onTrackAnalyzerProgress,
            this,
            &YouTubeFeature::onTrackAnalysisProgress);

    connect(&m_service,
            &mixxx::YouTubeService::searchResultsReady,
            this,
            &YouTubeFeature::onSearchResultsReady);
    connect(&m_service,
            &mixxx::YouTubeService::searchFailed,
            this,
            &YouTubeFeature::onSearchFailed);
    connect(&m_service,
            &mixxx::YouTubeService::downloadFinished,
            this,
            &YouTubeFeature::onDownloadFinished);
    connect(&m_service,
            &mixxx::YouTubeService::downloadFailed,
            this,
            &YouTubeFeature::onDownloadFailed);
    connect(&m_service,
            &mixxx::YouTubeService::botFlagged,
            this,
            [this](const QString& detail) {
                kLogger.warning() << "Bot flagging detected:" << detail;
                // Surface bot-detection warnings in the home HTML pane so
                // the user knows why results might be degraded or downloads
                // are falling back to yt-dlp.
                m_lastSearchError = detail;
                rebuildHomeHtml();
            });
    connect(&m_service,
            &mixxx::YouTubeService::genresReady,
            this,
            [this](const QStringList& genres) {
                if (!genres.isEmpty()) {
                    m_discoveredGenres = genres;
                    kLogger.info() << "Auto-discovered" << genres.size()
                                   << "genres from YouTube Music";
                    rebuildSidebar();
                    rebuildHomeHtml();
                }
            });

    // Auto-cleanup: when a YouTube-cached track is ejected from a deck (i.e.
    // replaced by a new one or unloaded), and no other deck still has it
    // loaded, delete the cached audio + sponsorblock sidecar from disk and
    // purge the database entry. This gives the user the "search → on deck →
    // forget about it" experience without unbounded disk growth. Analysis
    // results (BPM/key/waveform) are already persisted at this point so they
    // are not lost — only the audio bytes are.
    connect(&PlayerInfo::instance(),
            &PlayerInfo::trackChanged,
            this,
            [this](const QString& /*group*/,
                    TrackPointer pNew,
                    TrackPointer pOld) {
                // Pre-download safety net: if AutoDJ (or the user from a stale
                // playlist) is loading a YouTube-cache track whose file no
                // longer exists, kick off a background re-download so the
                // next play attempt or analysis pass succeeds without the
                // user noticing a stall.
                ensureDownloaded(pNew);
                if (pOld) {
                    maybeReleaseCachedTrack(pOld);
                }
            });

    // At startup, pre-fetch every YouTube-cache track that's queued in AutoDJ
    // but no longer present on disk. This is the "I closed the app halfway
    // through a set, restart, hit play" case — without this you'd hear a gap
    // when AutoDJ tried to crossfade into the missing track.
    QTimer::singleShot(0, this, [this]() {
        prefetchAutoDjQueue();
    });

    rebuildSidebar();
}

QString YouTubeFeature::cacheDir() const {
    QString base = m_pConfig->getSettingsPath();
    if (base.isEmpty()) {
        base = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    }
    const QString dir = QDir(base).filePath(QStringLiteral("youtube_cache"));
    QDir().mkpath(dir);
    return dir;
}

QString YouTubeFeature::resolvedTrendingRegion() const {
    // 1. Explicit user override (e.g. set from Preferences). Wins over
    //    everything so a user can opt into a different country's feed.
    const QString override = m_pConfig->getValueString(kCfgTrendingOverride)
                                     .trimmed()
                                     .toUpper();
    if (override.size() == 2) {
        return override;
    }
    // 2. Project default. Do not use stale geo-IP cache or en_US system
    // locales for this fork: the YouTube home feed should open on Greek top
    // songs unless the user explicitly overrides it.
    return kDefaultRegion;
}

void YouTubeFeature::activate() {
    kLogger.debug() << "YouTube feature activated";
    // Show the real WTrackTableView backed by our YouTubeTrackModel — this
    // is the "songs show like the rest of the songs would" fix. The HTML
    // browser pane (YOUTUBE_HOME) is no longer the primary surface; we
    // still keep the registration in bindLibraryWidget() as a fallback for
    // skin/back-compat reasons but never switch to it from here.
    Q_EMIT showTrackModel(m_pTrackModel);
    Q_EMIT enableCoverArtDisplay(false);
    // Force a fresh SELECT so the table reflects the current contents of
    // youtube_library even if the model state went stale while another
    // feature was in the foreground (e.g. results landed in the background
    // after a combined library/YouTube search).
    if (m_pTrackModel) {
        m_pTrackModel->select();
    }
    rebuildHomeHtml();
    // Prime the pane with Greek top songs from YouTube Music's songs category
    // on first open so a DJ sees music, not YouTube's generic live/gaming/news
    // trending page.
    //
    // Re-fetch whenever we still have no results AND the last thing we showed
    // was the trending feed (or nothing yet). The first trending fetch can fail
    // — e.g. a transient network error on startup before connectivity settles —
    // and previously that left m_lastQuery set to the trending sentinel with an
    // empty result list, so this guard was permanently false and clicking the
    // YouTube node again did nothing (the pane stayed blank forever). Retrying
    // on each activation makes the node self-heal once the network recovers. A
    // genuine user search is left untouched (its sentinel doesn't match), and an
    // in-flight fetch is not duplicated.
    const bool showingTrendingOrNothing = m_lastQuery.isEmpty() ||
            m_lastQuery.startsWith(mixxx::YouTubeService::kTrendingQueryPrefix);
    if (m_lastResults.isEmpty() && showingTrendingOrNothing &&
            !m_trendingFetchInFlight) {
        const QString country = resolvedTrendingRegion();
        m_lastQuery = mixxx::YouTubeService::kTrendingQueryPrefix + country;
        m_lastResults.clear();
        m_lastSearchError.clear();
        m_trendingFetchInFlight = true;
        rebuildSidebar();
        rebuildHomeHtml();
        // NOTE: we deliberately do NOT clear the youtube_library table here.
        // The clear is a synchronous SQLite write on the UI thread; during the
        // first-run device scan the LibraryScanner holds the write lock, so the
        // DELETE blocked the main thread for several seconds (the "clicking
        // YouTube lags" symptom) before failing anyway. The freshly-fetched
        // results replace the table in onSearchResultsReady() once they arrive;
        // until then the previous batch (or an empty table on first open) stays
        // visible, which is strictly better than a frozen UI.
        kLogger.info() << "Fetching YouTube trending for region" << country;
        m_service.fetchTrending(country, kSearchResultsMax);
    }
    // Auto-discover genres from YouTube Music for the user's region. Only
    // fetched once per session — the results are cached in m_discoveredGenres.
    if (m_discoveredGenres.isEmpty()) {
        const QString country = resolvedTrendingRegion();
        m_service.fetchMusicGenres(country);
    }
}

void YouTubeFeature::bindLibraryWidget(WLibrary* pLibraryWidget,
        KeyboardEventFilter* /*pKeyboard*/) {
    // The YOUTUBE_HOME view backs the right-hand library pane when the user
    // selects "YouTube" in the sidebar. Without this registration the pane
    // would render as an empty grey rectangle (the sidebar tree still
    // populates separately, but the user has no main-area feedback that the
    // search ran).
    auto pBrowser = make_parented<WLibraryTextBrowser>(pLibraryWidget);
    pBrowser->setOpenLinks(false);
    // Without this connection the home pane is a wall of unclickable text
    // (the user can only act on items via the sidebar tree) — the rendered
    // <a href="ytplay:…"> / <a href="ytcached:…"> links need a sink.
    connect(pBrowser.get(),
            &QTextBrowser::anchorClicked,
            this,
            &YouTubeFeature::onHomeAnchorClicked);
    m_pHomeView = pBrowser.get();
    pLibraryWidget->registerView(QStringLiteral("YOUTUBE_HOME"), pBrowser);
    rebuildHomeHtml();
}

void YouTubeFeature::onHomeAnchorClicked(const QUrl& url) {
    const QString scheme = url.scheme();
    if (scheme == kHomeAutoAnalyzeScheme) {
        setAutoAnalyzeResultsEnabled(!autoAnalyzeResultsEnabled());
        return;
    }
    // Genre links: "ytgenre:House" → search for "House songs 2024"
    if (scheme == kHomeGenreScheme) {
        const QString genre = url.path();
        if (!genre.isEmpty()) {
            const QString query = genre + QStringLiteral(" songs 2024");
            searchAndActivate(query);
        }
        return;
    }
    // Both schemes carry the YouTube video id as their path component (the
    // 11-char `[A-Za-z0-9_-]+` token). Both paths ensure the track is cached,
    // registered with the library, and queued for analysis without
    // automatically loading it onto a deck.
    if (scheme == kHomePlayScheme || scheme == kHomeCachedScheme) {
        const QString videoId = url.path();
        if (!videoId.isEmpty()) {
            requestDownload(videoId);
        }
    }
}

void YouTubeFeature::activateChild(const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }
    const auto* pItem = static_cast<TreeItem*>(index.internalPointer());
    if (!pItem) {
        return;
    }
    const QString payload = pItem->getData().toString();
    if (payload == kAutoAnalyzePayload) {
        setAutoAnalyzeResultsEnabled(!autoAnalyzeResultsEnabled());
    } else if (payload.startsWith(kGenrePrefix)) {
        // User clicked a genre: run a search for that genre's top songs.
        // Use a natural search query that works well with YouTube's algorithm.
        const QString genre = payload.mid(kGenrePrefix.size());
        const QString query = genre + QStringLiteral(" songs 2024");
        searchAndActivate(query);
    } else if (payload.startsWith(kSearchPrefix)) {
        // User clicked a search result: download (or use cache) and analyze.
        const QString videoId = payload.mid(kSearchPrefix.size());
        requestDownload(videoId);
    } else if (payload.startsWith(kCachedPrefix)) {
        // User clicked an already-downloaded track: load it now.
        const QString localPath = payload.mid(kCachedPrefix.size());
        TrackRef ref = TrackRef::fromFilePath(localPath);
        TrackPointer pTrack = m_pLibrary->trackCollectionManager()->getOrAddTrack(ref);
        if (pTrack) {
            Q_EMIT loadTrack(pTrack);
        }
    }
}

TreeItemModel* YouTubeFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void YouTubeFeature::searchAndActivate(const QString& query) {
    kLogger.info() << "Searching YouTube for:" << query;
    m_lastQuery = query;
    m_lastResults.clear();
    m_lastSearchError.clear();
    m_trendingFetchInFlight = false;
    rebuildSidebar();
    rebuildHomeHtml();
    // Do NOT clear youtube_library synchronously here — that DELETE on the UI
    // thread stalls for seconds behind the LibraryScanner's write lock during
    // the initial device scan (the "searching lags" symptom) and would fail
    // under contention anyway. onSearchResultsReady() replaces the rows with
    // the network results when they land; the previous batch stays visible in
    // the meantime instead of freezing the UI on an empty clear.
    m_service.searchVideos(query, kSearchResultsMax);
}

void YouTubeFeature::onSearchResultsReady(
        const QString& query, const QList<mixxx::YouTubeVideoInfo>& results) {
    if (query != m_lastQuery) {
        return; // a newer search has superseded this one
    }
    m_trendingFetchInFlight = false;
    // For the trending/home feed, drop hour-long "megamix"/"best of"
    // compilations so the DJ sees individual songs, not giant mixes. A genuine
    // user search (non-trending sentinel) is shown verbatim so long sets stay
    // findable. Unknown-duration items (0) are kept to avoid hiding real songs.
    QList<mixxx::YouTubeVideoInfo> filtered = results;
    if (query.startsWith(mixxx::YouTubeService::kTrendingQueryPrefix)) {
        filtered.clear();
        filtered.reserve(results.size());
        for (const mixxx::YouTubeVideoInfo& info : results) {
            if (info.durationSec > kTrendingMaxDurationSec) {
                continue;
            }
            filtered.append(info);
        }
    }
    m_lastResults = filtered;
    m_lastSearchError.clear();
    rebuildSidebar();
    rebuildHomeHtml();
    // Real fix: replace the rows in youtube_library so the user sees a
    // proper Title/Artist/Duration table, sortable, draggable, double-
    // clickable — not an HTML link list.
    replaceTrackTable(filtered);
    // Bring the populated track table to the foreground. Without this the
    // results only ever land in the model: if the visible pane was something
    // else when the (asynchronous) network reply arrived — e.g. the local
    // library after a combined search, or the YouTube node before the reply —
    // the user saw an empty main area and the results "stuck" in the sidebar
    // tree only. This is the user-reported "I have YouTube on the left bar but
    // no actual results" symptom. Showing the model switches the main pane to
    // the freshly-filled table and clears any stale search-box filter.
    Q_EMIT showTrackModel(m_pTrackModel);
    if (autoAnalyzeResultsEnabled()) {
        autoAnalyzeCurrentResults();
    }
}

void YouTubeFeature::onSearchFailed(const QString& query, const QString& error) {
    if (query != m_lastQuery) {
        return;
    }
    m_trendingFetchInFlight = false;
    kLogger.warning() << "YouTube search failed:" << error;
    m_lastSearchError = error;
    rebuildHomeHtml();
#if !defined(Q_OS_ANDROID)
    // Switch to the HTML pane so the user can see the error message.
    // Without this, the main area stays on the empty track table and the
    // user has no feedback at all — the "blank YouTube tab" symptom.
    // On Android we keep the track table view (the user explicitly does not
    // want a "browser view" — the error is still visible in the sidebar HTML
    // if they navigate there, and in the log file).
    Q_EMIT switchToView(QStringLiteral("YOUTUBE_HOME"));
#endif
}

void YouTubeFeature::requestDownload(const QString& videoId) {
    if (!isValidYouTubeVideoId(videoId)) {
        kLogger.warning() << "Ignoring invalid YouTube video id:" << videoId;
        return;
    }
    requestDownloadFile(videoId);
}

void YouTubeFeature::requestDownloadToPlayer(
        const QString& videoId, const QString& group, bool play) {
    if (!isValidYouTubeVideoId(videoId)) {
        kLogger.warning() << "Ignoring invalid YouTube deck-load request:"
                          << videoId << group;
        return;
    }
    PendingPlayerLoad pendingLoad;
    pendingLoad.group = group;
    pendingLoad.play = play;
    m_pendingPlayerLoads[videoId].append(pendingLoad);
    requestDownloadFile(videoId);
}

void YouTubeFeature::requestDownloadToAutoDJ(
        const QString& videoId, PlaylistDAO::AutoDJSendLoc loc) {
    if (!isValidYouTubeVideoId(videoId)) {
        kLogger.warning() << "Ignoring invalid YouTube Auto DJ request:" << videoId;
        return;
    }
    m_pendingAutoDjLoads[videoId].append(loc);
    requestDownloadFile(videoId);
}

void YouTubeFeature::requestDownloadFile(const QString& videoId) {
    if (!isValidYouTubeVideoId(videoId)) {
        kLogger.warning() << "Ignoring invalid YouTube video id:" << videoId;
        return;
    }
    // If we already have a cached file for this id, skip the download and load
    // it straight away.
    const QDir dir(cacheDir());
    const QStringList existing =
            dir.entryList({videoId + QStringLiteral(".*")},
                    QDir::Files | QDir::NoDotAndDotDot);
    for (const QString& f : existing) {
        if (isYouTubeSidecarFile(f)) {
            continue;
        }
        onDownloadFinished(videoId, dir.filePath(f));
        return;
    }
    if (m_videoIdsDownloading.contains(videoId)) {
        return;
    }
    m_videoIdsDownloading.insert(videoId);
    kLogger.info() << "Downloading YouTube video" << videoId;
    m_service.downloadVideo(videoId, cacheDir());
}

void YouTubeFeature::requestPrefetch(const QString& videoId) {
    // Background re-download — do NOT register for auto-load. Only kicks off
    // if the file isn't already there.
    if (isValidYouTubeVideoId(videoId)) {
        requestDownloadFile(videoId);
    }
}

bool YouTubeFeature::autoAnalyzeResultsEnabled() const {
    return m_pConfig->getValue<bool>(kCfgAutoAnalyzeResults, false);
}

void YouTubeFeature::setAutoAnalyzeResultsEnabled(bool enabled) {
    m_pConfig->setValue(kCfgAutoAnalyzeResults, enabled);
    rebuildSidebar();
    rebuildHomeHtml();
    if (enabled) {
        autoAnalyzeCurrentResults();
    }
}

void YouTubeFeature::autoAnalyzeCurrentResults() {
    for (const auto& info : std::as_const(m_lastResults)) {
        requestPrefetch(info.id);
    }
}

void YouTubeFeature::onDownloadFinished(
        const QString& videoId, const QString& localPath) {
    m_videoIdsDownloading.remove(videoId);
    m_downloadRetryCount.remove(videoId);
    if (!QFileInfo::exists(localPath)) {
        kLogger.warning() << "Downloaded file disappeared:" << localPath;
        return;
    }
    // SponsorBlock cuts have already been physically applied to the file by
    // YouTubeService at this point (or, on cut failure, a .sponsor.json
    // sidecar has been written so SponsorBlockController can fall back to
    // skip-at-playback). Either way, the file we hand to the analyzer here
    // already represents the music-only timeline.

    // Look up a friendly label from m_lastResults if the video came from the
    // current search, otherwise just fall back to the videoId.
    QString label = videoId;
    for (const auto& info : std::as_const(m_lastResults)) {
        if (info.id == videoId) {
            label = displayLabelForVideo(info);
            break;
        }
    }
    m_downloadedTracks.insert(videoId, label);
    rebuildSidebar();
    rebuildHomeHtml();
    // Update the corresponding youtube_library row (or insert a new one if
    // this download wasn't part of the current search) so the track table
    // reflects the new on-disk path. Without this the row's location would
    // remain "youtube://VIDEOID" and a second double-click would re-trigger
    // the download instead of loading from cache.
    int durationSec = 0;
    QString uploader;
    QString title = label;
    for (const auto& info : std::as_const(m_lastResults)) {
        if (info.id == videoId) {
            const TrackDisplayMetadata metadata = displayMetadataForVideo(info);
            durationSec = info.durationSec;
            uploader = metadata.artist;
            title = metadata.title;
            break;
        }
    }
    upsertDownloadedRow(videoId, localPath, title, uploader, durationSec);

    // Always register with the track collection so analysis runs and the DB
    // entry exists. Downloads triggered by selection or context-menu Analyze
    // stay in the library; explicit Load-to-deck requests are handled below.
    TrackRef ref = TrackRef::fromFilePath(localPath);
    TrackPointer pTrack = m_pLibrary->trackCollectionManager()->getOrAddTrack(ref);
    if (!pTrack) {
        kLogger.warning() << "Could not add downloaded track to library:"
                          << localPath;
        return;
    }
    pTrack->setArtist(uploader);
    pTrack->setTitle(title.isEmpty() ? videoId : title);
    pTrack->setAlbum(QStringLiteral("YouTube"));
    pTrack->setComment(videoId);
    if (pTrack->getId().isValid()) {
        if (pTrack->getBpm() > 0) {
            syncAnalyzedTrackMetadata(pTrack);
        } else {
            QList<AnalyzerScheduledTrack> tracks;
            tracks.append(AnalyzerScheduledTrack(pTrack->getId()));
            Q_EMIT analyzeTracks(tracks);
        }
    }
    const QList<PendingPlayerLoad> pendingLoads = m_pendingPlayerLoads.take(videoId);
    for (const auto& pendingLoad : pendingLoads) {
        if (pendingLoad.group.isEmpty()) {
            // Empty group = "load to next available deck" (double-click path).
            // Emit loadTrack so the standard deck-selection logic applies,
            // same as double-clicking a local library track.
            Q_EMIT loadTrack(pTrack);
        } else {
#ifdef __STEM__
            Q_EMIT loadTrackToPlayer(pTrack,
                    pendingLoad.group,
                    mixxx::StemChannelSelection(),
                    pendingLoad.play);
#else
            Q_EMIT loadTrackToPlayer(pTrack, pendingLoad.group, pendingLoad.play);
#endif
        }
    }
    const QList<PlaylistDAO::AutoDJSendLoc> pendingAutoDjLoads =
            m_pendingAutoDjLoads.take(videoId);
    if (!pendingAutoDjLoads.isEmpty() && pTrack->getId().isValid()) {
        PlaylistDAO& playlistDao = m_pLibrary->trackCollectionManager()
                                           ->internalCollection()
                                           ->getPlaylistDAO();
        m_pLibrary->trackCollectionManager()->unhideTracks({pTrack->getId()});
        for (const auto loc : pendingAutoDjLoads) {
            playlistDao.addTracksToAutoDJQueue({pTrack->getId()}, loc);
        }
    }
}

void YouTubeFeature::onDownloadFailed(const QString& videoId, const QString& error) {
    m_videoIdsDownloading.remove(videoId);
    int& retries = m_downloadRetryCount[videoId];
    if (retries < kMaxDownloadRetries) {
        retries++;
        kLogger.info() << "YouTube download failed for" << videoId << ":" << error
                       << "— retrying (attempt" << retries + 1 << ")";
        // Re-trigger download. requestDownloadFile will check the cache first
        // and skip if the file appeared in the meantime.
        m_service.downloadVideo(videoId, cacheDir());
        m_videoIdsDownloading.insert(videoId);
        return;
    }
    // Exhausted retries — give up and clean up pending state.
    m_downloadRetryCount.remove(videoId);
    m_pendingPlayerLoads.remove(videoId);
    m_pendingAutoDjLoads.remove(videoId);
    kLogger.warning() << "YouTube download failed for" << videoId
                      << "after" << kMaxDownloadRetries + 1
                      << "attempts:" << error;
}

void YouTubeFeature::onTrackAnalysisProgress(
        TrackId trackId, AnalyzerProgress analyzerProgress) {
    if (analyzerProgress != kAnalyzerProgressDone || !trackId.isValid()) {
        return;
    }
    syncAnalyzedTrackMetadata(
            m_pLibrary->trackCollectionManager()->getTrackById(trackId));
}

void YouTubeFeature::syncAnalyzedTrackMetadata(const TrackPointer& pTrack) {
    if (!pTrack || !m_pTrackModel || !m_pTrackCache || !m_pTrackCollection) {
        return;
    }
    const QFileInfo fileInfo(pTrack->getLocation());
    const QDir cache(cacheDir());
    if (!fileInfo.absoluteFilePath().startsWith(
                cache.absolutePath() + QLatin1Char('/'))) {
        return;
    }
    const QString videoId = fileInfo.completeBaseName();
    const double bpm = pTrack->getBpm();
    QSqlDatabase db = m_pTrackCollection->database();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
            "UPDATE youtube_library SET "
            "bpm = CASE WHEN :bpm > 0 THEN :bpm ELSE bpm END, "
            "duration = CASE WHEN :duration > 0 THEN :duration ELSE duration END "
            "WHERE comment = :comment"));
    query.bindValue(QStringLiteral(":bpm"), bpm);
    query.bindValue(QStringLiteral(":duration"), pTrack->getDurationSecondsInt());
    query.bindValue(QStringLiteral(":comment"), videoId);
    if (!query.exec()) {
        kLogger.warning() << "youtube_library analysis metadata UPDATE failed:"
                          << query.lastError().text();
        return;
    }
    m_pTrackCache->buildIndex();
    m_pTrackModel->select();
}

void YouTubeFeature::maybeReleaseCachedTrack(const TrackPointer& pTrack) {
    if (!pTrack) {
        return;
    }
    const QString location = pTrack->getLocation();
    if (location.isEmpty()) {
        return;
    }
    // Only files we own under our cache dir are eligible for cleanup.
    const QDir cache(cacheDir());
    if (!QFileInfo(location).absoluteFilePath().startsWith(
                cache.absolutePath() + QLatin1Char('/'))) {
        return;
    }
    // Don't delete if any other deck still has it loaded — common when the
    // user practices crossfading the same track between decks.
    const auto loaded = PlayerInfo::instance().getLoadedTracks();
    for (auto it = loaded.cbegin(); it != loaded.cend(); ++it) {
        if (it.value() && it.value()->getLocation() == location) {
            return;
        }
    }
    // Don't delete if the track is referenced by *anything* the user might
    // come back to: any playlist (which includes the AutoDJ queue — that's a
    // hidden playlist of type AutoDJ), or any crate. This is what makes
    // pre-queued AutoDJ sets safe — queued YouTube tracks survive eject.
    auto* pTcm = m_pLibrary->trackCollectionManager();
    if (!pTcm) {
        return;
    }
    auto* pInternal = pTcm->internalCollection();
    const TrackId trackId = pTrack->getId();
    if (trackId.isValid()) {
        QSet<int> playlistSet;
        pInternal->getPlaylistDAO().getPlaylistsTrackIsIn(trackId, &playlistSet);
        if (!playlistSet.isEmpty()) {
            return;
        }
        if (pInternal->crates().selectTrackCratesSorted(trackId).next()) {
            return;
        }
    }
    // All checks passed → safe to drop bytes + DB row. Defer the unlink so the
    // engine has a moment to release any open file descriptor on slow Android
    // storage; failure here is non-fatal because the next sweep will retry.
    QString videoId = QFileInfo(location).completeBaseName();
    QTimer::singleShot(2000, this, [this, location, videoId, trackId]() {
        if (QFile::remove(location)) {
            kLogger.info() << "Released cached YouTube track" << videoId;
        }
        // SponsorBlock sidecar lives next to the audio file.
        const QString sidecar = QFileInfo(location).absoluteFilePath() +
                QStringLiteral(".sponsor.json");
        QFile::remove(sidecar);
        if (trackId.isValid()) {
            m_pLibrary->trackCollectionManager()->purgeTracks(
                    {TrackRef::fromFilePath(location, trackId)});
        }
        m_downloadedTracks.remove(videoId);
        rebuildSidebar();
        rebuildHomeHtml();
    });
}

void YouTubeFeature::ensureDownloaded(const TrackPointer& pTrack) {
    if (!pTrack) {
        return;
    }
    const QString location = pTrack->getLocation();
    if (location.isEmpty()) {
        return;
    }
    // Only re-fetch tracks under our cache dir.
    const QDir cache(cacheDir());
    if (!QFileInfo(location).absoluteFilePath().startsWith(
                cache.absolutePath() + QLatin1Char('/'))) {
        return;
    }
    if (QFileInfo::exists(location)) {
        return; // already on disk
    }
    const QString videoId = QFileInfo(location).completeBaseName();
    if (videoId.isEmpty()) {
        return;
    }
    requestPrefetch(videoId);
}

void YouTubeFeature::prefetchAutoDjQueue() {
    auto* pTcm = m_pLibrary->trackCollectionManager();
    if (!pTcm) {
        return;
    }
    auto* pInternal = pTcm->internalCollection();
    const int autoDjId = pInternal->getPlaylistDAO().getPlaylistIdFromName(
            AUTODJ_TABLE);
    if (autoDjId < 0) {
        return;
    }
    const QList<TrackId> ids =
            pInternal->getPlaylistDAO().getTrackIdsInPlaylistOrder(autoDjId);
    for (const TrackId& id : ids) {
        TrackPointer pTrack = pTcm->getTrackById(id);
        ensureDownloaded(pTrack);
    }
}

void YouTubeFeature::appendTrackIdsFromRightClickIndex(
        QList<TrackId>* /*trackIds*/, QString* /*pPlaylist*/) {
    // YouTube tracks are loaded on demand and we do not track collection ids
    // until they have been imported via getOrAddTrack(). Right-click context
    // actions therefore have nothing to append here.
}

void YouTubeFeature::rebuildSidebar() {
    auto pRoot = TreeItem::newRoot(this);

    pRoot->appendChild(autoAnalyzeResultsEnabled()
                    ? tr("Auto Analyze: On")
                    : tr("Auto Analyze: Off"),
            kAutoAnalyzePayload);

    // Genres node — contains DJ-oriented genre shortcuts that trigger a
    // YouTube search for that genre's top songs. Uses auto-discovered genres
    // from YouTube Music when available, otherwise falls back to defaults.
    const QStringList& genres = m_discoveredGenres.isEmpty()
            ? kDefaultGenres
            : m_discoveredGenres;
    TreeItem* pGenresNode = pRoot->appendChild(tr("Genres"));
    for (const QString& genre : genres) {
        pGenresNode->appendChild(genre, kGenrePrefix + genre);
    }

    // Note: search/trending results are intentionally NOT listed as sidebar
    // tree children. They belong in the main track table (the right-hand
    // pane), exactly like every other library source — listing each song in
    // the left sidebar instead confused users ("why do I have YouTube on the
    // left bar and not actual results?"). The table is populated by
    // replaceTrackTable() and surfaced via showTrackModel(); the sidebar only
    // carries navigation/toggle nodes.

    if (!m_downloadedTracks.isEmpty()) {
        TreeItem* pCachedNode = pRoot->appendChild(tr("Downloaded"));
        const QDir dir(cacheDir());
        for (auto it = m_downloadedTracks.cbegin();
                it != m_downloadedTracks.cend();
                ++it) {
            const QStringList files =
                    dir.entryList({it.key() + QStringLiteral(".*")},
                            QDir::Files | QDir::NoDotAndDotDot);
            QString localPath;
            for (const QString& f : files) {
                // Skip both yt-dlp's metadata sidecar (.info.json) and our
                // SponsorBlock sidecar (.sponsor.json) — only the audio file
                // is loadable. Without this guard, depending on filesystem
                // sort order we could wire the sidebar entry to a JSON file.
                if (isYouTubeSidecarFile(f)) {
                    continue;
                }
                localPath = dir.filePath(f);
                break;
            }
            if (localPath.isEmpty()) {
                continue;
            }
            pCachedNode->appendChild(it.value(), QString(kCachedPrefix + localPath));
        }
    }

    m_pSidebarModel->setRootItem(std::move(pRoot));
}

void YouTubeFeature::rebuildHomeHtml() {
    if (!m_pHomeView) {
        return;
    }
    QString html;
    html += QStringLiteral("<h2>") + tr("YouTube") + QStringLiteral("</h2>");

    html += QStringLiteral("<p>") +
            tr("Type a song, artist or video title in the search box at the "
               "top of the library to search YouTube. Click a result below "
               "(or in the sidebar) to download it — finished tracks appear "
               "under <b>Downloaded</b> and in the main library view, ready "
               "to drag onto any deck. Audio is downloaded ad-free and "
               "SponsorBlock automatically trims sponsored intros, self-promo "
               "and other non-music segments out of the file.") +
            QStringLiteral("</p>");

    html += QStringLiteral("<p><a href=\"") + kHomeAutoAnalyzeScheme +
            QStringLiteral(":toggle\">") +
            (autoAnalyzeResultsEnabled() ? tr("Auto Analyze: On")
                                         : tr("Auto Analyze: Off")) +
            QStringLiteral("</a> — ") +
            tr("when enabled, all YouTube results are downloaded and analyzed "
               "automatically. Leave it off for best performance and storage "
               "usage.") +
            QStringLiteral("</p>");

    // Genre quick-links — rendered as a horizontal flow of clickable tags
    // so the user can browse by genre without manually typing a search.
    // Uses auto-discovered genres when available, otherwise defaults.
    const QStringList& genres = m_discoveredGenres.isEmpty()
            ? kDefaultGenres
            : m_discoveredGenres;
    html += QStringLiteral("<h3>") + tr("Browse by Genre") + QStringLiteral("</h3>");
    html += QStringLiteral("<p>");
    for (int i = 0; i < genres.size(); ++i) {
        if (i > 0) {
            html += QStringLiteral(" &middot; ");
        }
        html += QStringLiteral("<a href=\"") + kHomeGenreScheme +
                QStringLiteral(":") + genres.at(i) +
                QStringLiteral("\">") + genres.at(i).toHtmlEscaped() +
                QStringLiteral("</a>");
    }
    html += QStringLiteral("</p>");

    if (!m_lastQuery.isEmpty()) {
        // Trending sentinel query — render a friendly "Trending in <Country>"
        // header instead of the raw `__trending__:US` token.
        const bool isTrending = m_lastQuery.startsWith(
                mixxx::YouTubeService::kTrendingQueryPrefix);
        if (isTrending) {
            const QString region = m_lastQuery.mid(
                    mixxx::YouTubeService::kTrendingQueryPrefix.size());
            html += QStringLiteral("<h3>") +
                    tr("Trending music in %1").arg(countryDisplayName(region)) +
                    QStringLiteral("</h3>");
        } else {
            html += QStringLiteral("<h3>") +
                    tr("Results for: %1").arg(m_lastQuery.toHtmlEscaped()) +
                    QStringLiteral("</h3>");
        }
        if (!m_lastSearchError.isEmpty()) {
            // Surface the underlying error so the user can act on it (e.g.
            // network failure, yt-dlp out-of-date, region-blocked content).
            html += QStringLiteral("<p><b>") +
                    tr("Search failed") +
                    QStringLiteral(":</b> ") +
                    m_lastSearchError.toHtmlEscaped() +
                    QStringLiteral("</p>");
            // Help the user find the log file for further diagnosis. On Android
            // the logs are in the app's documents folder which is browseable
            // from a file manager at data/data/org.mixxx.Mixxx/files/documents/Mixxx.
            const QString logDir =
                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
            html += QStringLiteral("<p><small>") +
                    tr("Logs: %1/Mixxx/mixxx.log")
                            .arg(logDir.toHtmlEscaped()) +
                    QStringLiteral("</small></p>");
        } else if (m_lastResults.isEmpty()) {
            html += QStringLiteral("<p><i>") +
                    (isTrending ? tr("Loading trending…") : tr("Searching…")) +
                    QStringLiteral("</i></p>");
        } else {
            html += QStringLiteral("<ul>");
            for (const auto& info : std::as_const(m_lastResults)) {
                const TrackDisplayMetadata metadata = displayMetadataForVideo(info);
                QString line = metadata.title.toHtmlEscaped();
                if (!metadata.artist.isEmpty()) {
                    line += QStringLiteral(" — ") +
                            metadata.artist.toHtmlEscaped();
                }
                if (info.durationSec > 0) {
                    line += QStringLiteral(" [%1:%2]")
                                    .arg(info.durationSec / 60)
                                    .arg(info.durationSec % 60,
                                            2,
                                            10,
                                            QLatin1Char('0'));
                }
                // Wrap the entry in an <a href="ytplay:VIDEOID"> so the user
                // can actually act on it from the main pane. We rely on
                // setOpenLinks(false) + the anchorClicked handler set up in
                // bindLibraryWidget() to dispatch the click. The video id
                // matches [A-Za-z0-9_-]+ and so needs no extra escaping.
                html += QStringLiteral("<li><a href=\"") + kHomePlayScheme +
                        QStringLiteral(":") + info.id +
                        QStringLiteral("\">") + line +
                        QStringLiteral("</a></li>");
            }
            html += QStringLiteral("</ul>");
        }
    }

    if (!m_downloadedTracks.isEmpty()) {
        html += QStringLiteral("<h3>") + tr("Downloaded") + QStringLiteral("</h3>");
        html += QStringLiteral("<ul>");
        for (auto it = m_downloadedTracks.cbegin();
                it != m_downloadedTracks.cend();
                ++it) {
            // Same href shape as search results — videoId in the path. The
            // anchor handler routes via requestDownload(), which short-
            // circuits when the cached file is on disk and refreshes the
            // library/analyzer state through onDownloadFinished().
            html += QStringLiteral("<li><a href=\"") + kHomeCachedScheme +
                    QStringLiteral(":") + it.key() +
                    QStringLiteral("\">") + it.value().toHtmlEscaped() +
                    QStringLiteral("</a></li>");
        }
        html += QStringLiteral("</ul>");
    }

    m_pHomeView->setHtml(html);
}

void YouTubeFeature::replaceTrackTable(
        const QList<mixxx::YouTubeVideoInfo>& videos, int attempt) {
    if (!m_pTrackModel || !m_pTrackCache) {
        return;
    }
    QSqlDatabase db = m_pTrackCollection->database();
    ScopedTransaction transaction(db);

    // Wipe the current results — we always show one batch (latest search
    // OR latest trending) at a time, mirroring how the previous HTML pane
    // worked. Tracks the user has actually played stay registered with the
    // main `library` table independently, so this DELETE doesn't lose
    // analysis data — it just clears the YouTube *search* feed.
    QSqlQuery del(db);
    del.prepare(QStringLiteral("DELETE FROM youtube_library"));
    if (!del.exec()) {
        // During the initial full-device library scan the LibraryScanner
        // thread holds the SQLite write lock for long stretches, so this
        // DELETE on the main thread fails with "database is locked". Aborting
        // here used to silently drop the freshly-fetched results — the user's
        // "search shows nothing / just the old small list" symptom. Instead of
        // blocking the UI thread on the lock, reschedule the write on the event
        // loop so the table is populated as soon as the scanner yields.
        const QString errText = del.lastError().text();
        const bool locked = errText.contains(
                QStringLiteral("locked"), Qt::CaseInsensitive);
        // Retry on the event loop (non-blocking) for up to ~10 s, which
        // comfortably outlasts the per-track lock the scanner holds.
        constexpr int kRetryIntervalMs = 250;
        constexpr int kRetryTimeoutMs = 10 * 1000;
        constexpr int kMaxAttempts = kRetryTimeoutMs / kRetryIntervalMs;
        if (locked && attempt < kMaxAttempts) {
            transaction.rollback();
            const QList<mixxx::YouTubeVideoInfo> pending = videos;
            QTimer::singleShot(kRetryIntervalMs, this, [this, pending, attempt]() {
                replaceTrackTable(pending, attempt + 1);
            });
            return;
        }
        kLogger.warning() << "youtube_library DELETE failed:" << errText;
        return;
    }

    QSqlQuery ins(db);
    ins.prepare(QStringLiteral(
            "INSERT INTO youtube_library "
            "(artist, title, album, location, comment, duration) "
            "VALUES (:artist, :title, :album, :location, :comment, :duration)"));

    const QDir dir(cacheDir());
    for (const mixxx::YouTubeVideoInfo& info : videos) {
        if (info.id.isEmpty()) {
            continue;
        }
        // Determine whether we already have a downloaded file for this id
        // — if so, point `location` at the real path so double-click loads
        // immediately via BaseExternalTrackModel::getTrack(). Otherwise
        // store the placeholder URI; YouTubeTrackModel::getTrack()
        // intercepts placeholders to kick off the download.
        QString location;
        const QStringList existing =
                dir.entryList({info.id + QStringLiteral(".*")},
                        QDir::Files | QDir::NoDotAndDotDot);
        for (const QString& f : existing) {
            if (isYouTubeSidecarFile(f)) {
                continue;
            }
            location = dir.filePath(f);
            break;
        }
        if (location.isEmpty()) {
            location = YouTubeTrackModel::kPlaceholderScheme + info.id;
        }
        const TrackDisplayMetadata metadata = displayMetadataForVideo(info);
        ins.bindValue(QStringLiteral(":artist"), metadata.artist);
        ins.bindValue(QStringLiteral(":title"), metadata.title);
        ins.bindValue(QStringLiteral(":album"), QStringLiteral("YouTube"));
        ins.bindValue(QStringLiteral(":location"), location);
        ins.bindValue(QStringLiteral(":comment"), info.id);
        ins.bindValue(QStringLiteral(":duration"), info.durationSec);
        if (!ins.exec()) {
            // Likely a UNIQUE conflict (two trending entries collapse onto
            // the same cached file). Non-fatal — the existing row keeps
            // working — but log so duplicates are visible.
            kLogger.debug() << "youtube_library INSERT skipped:"
                            << ins.lastError().text();
        }
    }
    transaction.commit();

    m_pTrackCache->buildIndex();
    // Clear any residual per-view search filter before re-selecting. When the
    // user types a query in the library search box, YouTubeTrackModel::search()
    // applies that text as a local SQL filter (for instant feedback) *and*
    // fires the network search. Once the network results land here, the rows we
    // just inserted ARE the answer for that query — re-applying the same text as
    // a local filter would hide every result that doesn't literally contain the
    // query string in its artist/title/album/comment, leaving the right-hand
    // pane empty even though the sidebar listed results. Resetting the filter
    // guarantees the full fetched result set is shown.
    m_pTrackModel->setSearch(QString());
    // select() re-runs the SELECT against the underlying SQL view so the
    // WTrackTableView picks up the fresh row set.
    m_pTrackModel->select();
}

void YouTubeFeature::upsertDownloadedRow(const QString& videoId,
        const QString& localPath,
        const QString& title,
        const QString& uploader,
        int durationSec) {
    if (!m_pTrackModel || !m_pTrackCache || videoId.isEmpty()) {
        return;
    }
    QSqlDatabase db = m_pTrackCollection->database();
    QSqlQuery upd(db);
    // Use comment (videoId) to find the row even if its location changed
    // from the placeholder to a real path. Keep the parsed display metadata in
    // sync too, so old placeholder rows with channel/video-title metadata are
    // normalized when their audio file lands.
    upd.prepare(QStringLiteral(
            "UPDATE youtube_library SET "
            "location = :location, "
            "title = :title, "
            "artist = :artist, "
            "duration = COALESCE(NULLIF(duration, 0), :duration) "
            "WHERE comment = :comment"));
    upd.bindValue(QStringLiteral(":location"), localPath);
    upd.bindValue(QStringLiteral(":title"), title.isEmpty() ? videoId : title);
    upd.bindValue(QStringLiteral(":artist"), uploader);
    upd.bindValue(QStringLiteral(":duration"), durationSec);
    upd.bindValue(QStringLiteral(":comment"), videoId);
    if (!upd.exec()) {
        kLogger.warning() << "youtube_library UPDATE failed:"
                          << upd.lastError().text();
        return;
    }
    if (upd.numRowsAffected() == 0) {
        // No matching row — happens when the download was triggered from
        // outside the current search (e.g. AutoDJ pre-fetch). Insert a fresh
        // row so the user still sees it in the table when they switch back to
        // the YouTube tab.
        QSqlQuery ins(db);
        ins.prepare(QStringLiteral(
                "INSERT OR IGNORE INTO youtube_library "
                "(artist, title, album, location, comment, duration) VALUES "
                "(:artist, :title, :album, :location, :comment, :duration)"));
        ins.bindValue(QStringLiteral(":artist"), uploader);
        ins.bindValue(QStringLiteral(":title"),
                title.isEmpty() ? videoId : title);
        ins.bindValue(QStringLiteral(":album"), QStringLiteral("YouTube"));
        ins.bindValue(QStringLiteral(":location"), localPath);
        ins.bindValue(QStringLiteral(":comment"), videoId);
        ins.bindValue(QStringLiteral(":duration"), durationSec);
        if (!ins.exec()) {
            kLogger.debug() << "youtube_library INSERT (downloaded) skipped:"
                            << ins.lastError().text();
        }
    }
    m_pTrackCache->buildIndex();
    m_pTrackModel->select();
}

#include "moc_youtubefeature.cpp"
