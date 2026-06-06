#include "library/youtube/youtubefeature.h"

#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTextBrowser>
#include <QTimer>
#include <QUrl>

#include "analyzer/analyzerscheduledtrack.h"
#include "control/controlproxy.h"
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
#include "widget/wsearchlineedit.h"

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
// Sidebar nodes in the Samples section:
//   kSampleQueryPrefix  → fires a YouTube search so the user can pick a version
//   kMyInstantFetch     → triggers a live fetch of the Greek myinstants.com page
//   kMyInstantPrefix    → downloads the MP3 at this CDN URL from myinstants.com
const QString kSampleQueryPrefix = QStringLiteral("yt-sample-query:");
const QString kMyInstantFetch = QStringLiteral("myinstant-fetch");
const QString kMyInstantPrefix = QStringLiteral("myinstant:");

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
        QStringLiteral("Δημοτικά"),
        QStringLiteral("Κρητικά"),
        QStringLiteral("Ποντιακά"),
        QStringLiteral("Ελληνικό ροκ"),
        QStringLiteral("Greek trap"),
        QStringLiteral("Ελληνική ποπ"),
        QStringLiteral("Ελληνική ντίσκο"),
        // House & derivatives
        QStringLiteral("House"),
        QStringLiteral("Deep House"),
        QStringLiteral("Tech House"),
        QStringLiteral("Progressive House"),
        QStringLiteral("Afro House"),
        QStringLiteral("Acid House"),
        QStringLiteral("Funky House"),
        QStringLiteral("Electro House"),
        QStringLiteral("Bass House"),
        QStringLiteral("Future House"),
        QStringLiteral("Tribal House"),
        QStringLiteral("Jackin House"),
        QStringLiteral("Minimal House"),
        QStringLiteral("Soulful House"),
        QStringLiteral("Latin House"),
        // Techno & derivatives
        QStringLiteral("Techno"),
        QStringLiteral("Hard Techno"),
        QStringLiteral("Dark Techno"),
        QStringLiteral("Acid Techno"),
        QStringLiteral("Industrial Techno"),
        QStringLiteral("Melodic Techno"),
        QStringLiteral("Minimal Techno"),
        QStringLiteral("Detroit Techno"),
        QStringLiteral("Dub Techno"),
        QStringLiteral("Peak Time Techno"),
        QStringLiteral("Hypnotic Techno"),
        QStringLiteral("Raw Techno"),
        // Trance & derivatives
        QStringLiteral("Trance"),
        QStringLiteral("Psytrance"),
        QStringLiteral("Hard Trance"),
        QStringLiteral("Progressive Trance"),
        QStringLiteral("Uplifting Trance"),
        QStringLiteral("Goa Trance"),
        QStringLiteral("Vocal Trance"),
        QStringLiteral("Dark Psytrance"),
        QStringLiteral("Full-On"),
        QStringLiteral("Hi-Tech"),
        // Drum & Bass / Jungle
        QStringLiteral("Drum & Bass"),
        QStringLiteral("Liquid DnB"),
        QStringLiteral("Neurofunk"),
        QStringLiteral("Jump Up"),
        QStringLiteral("Jungle"),
        QStringLiteral("Darkstep"),
        QStringLiteral("Ragga Jungle"),
        // Hardcore & hard dance
        QStringLiteral("Hardcore"),
        QStringLiteral("Frenchcore"),
        QStringLiteral("Speedcore"),
        QStringLiteral("Lolicore"),
        QStringLiteral("Gabber"),
        QStringLiteral("Hardstyle"),
        QStringLiteral("Rawstyle"),
        QStringLiteral("Happy Hardcore"),
        QStringLiteral("Breakcore"),
        QStringLiteral("Terrorcore"),
        QStringLiteral("Uptempo Hardcore"),
        QStringLiteral("Industrial Hardcore"),
        QStringLiteral("J-Core"),
        QStringLiteral("UK Hardcore"),
        QStringLiteral("Hard Dance"),
        QStringLiteral("Hard Bounce"),
        // Bass music
        QStringLiteral("Dubstep"),
        QStringLiteral("Riddim"),
        QStringLiteral("Brostep"),
        QStringLiteral("Tearout"),
        QStringLiteral("UK Garage"),
        QStringLiteral("Bassline"),
        QStringLiteral("Future Bass"),
        QStringLiteral("Trap (EDM)"),
        QStringLiteral("Wave"),
        QStringLiteral("Colour Bass"),
        // Breakbeat & electro
        QStringLiteral("Breakbeat"),
        QStringLiteral("Big Beat"),
        QStringLiteral("Electro"),
        QStringLiteral("Electroclash"),
        QStringLiteral("Nu-Disco"),
        QStringLiteral("Disco"),
        QStringLiteral("Italo Disco"),
        QStringLiteral("Space Disco"),
        // Dark / industrial / EBM
        QStringLiteral("EBM"),
        QStringLiteral("Darkwave"),
        QStringLiteral("Synthwave"),
        QStringLiteral("Retrowave"),
        QStringLiteral("Cyberpunk"),
        QStringLiteral("Industrial"),
        QStringLiteral("Dark Electro"),
        QStringLiteral("Aggrotech"),
        QStringLiteral("Witch House"),
        // Ambient / downtempo
        QStringLiteral("Ambient"),
        QStringLiteral("Dark Ambient"),
        QStringLiteral("Downtempo"),
        QStringLiteral("Chillout"),
        QStringLiteral("Lo-Fi"),
        QStringLiteral("Trip Hop"),
        QStringLiteral("Dub"),
        // Pop / mainstream
        QStringLiteral("Pop"),
        QStringLiteral("Electropop"),
        QStringLiteral("Synthpop"),
        QStringLiteral("K-Pop"),
        QStringLiteral("J-Pop"),
        QStringLiteral("Indie Pop"),
        QStringLiteral("Dream Pop"),
        // Hip Hop / Rap
        QStringLiteral("Hip Hop"),
        QStringLiteral("Trap"),
        QStringLiteral("Drill"),
        QStringLiteral("Phonk"),
        QStringLiteral("Boom Bap"),
        QStringLiteral("Lo-Fi Hip Hop"),
        QStringLiteral("Grime"),
        QStringLiteral("Cloud Rap"),
        // R&B / Soul
        QStringLiteral("R&B"),
        QStringLiteral("Soul"),
        QStringLiteral("Neo-Soul"),
        QStringLiteral("Funk"),
        // Latin
        QStringLiteral("Reggaeton"),
        QStringLiteral("Latin"),
        QStringLiteral("Dembow"),
        QStringLiteral("Baile Funk"),
        QStringLiteral("Cumbia"),
        QStringLiteral("Salsa"),
        QStringLiteral("Bachata"),
        QStringLiteral("Merengue"),
        // African
        QStringLiteral("Afrobeats"),
        QStringLiteral("Amapiano"),
        QStringLiteral("Afro Tech"),
        QStringLiteral("Gqom"),
        QStringLiteral("Kwaito"),
        // Asian & fusion
        QStringLiteral("Kawaii Metal"),
        QStringLiteral("Kawaii Future Bass"),
        QStringLiteral("City Pop"),
        QStringLiteral("Vaporwave"),
        QStringLiteral("Future Funk"),
        // Rock / Metal / Alternative
        QStringLiteral("Rock"),
        QStringLiteral("Metal"),
        QStringLiteral("Punk"),
        QStringLiteral("Post-Punk"),
        QStringLiteral("Shoegaze"),
        QStringLiteral("Grunge"),
        QStringLiteral("Emo"),
        QStringLiteral("Screamo"),
        QStringLiteral("Metalcore"),
        QStringLiteral("Deathcore"),
        // Reggae / Caribbean
        QStringLiteral("Reggae"),
        QStringLiteral("Dancehall"),
        QStringLiteral("Soca"),
        // Misc / EDM
        QStringLiteral("EDM"),
        QStringLiteral("Minimal"),
        QStringLiteral("Glitch"),
        QStringLiteral("IDM"),
        QStringLiteral("Footwork"),
        QStringLiteral("Jersey Club"),
        QStringLiteral("Baltimore Club"),
        QStringLiteral("Bounce"),
        QStringLiteral("Moombahton"),
        QStringLiteral("Tropical House"),
};

/// Master list of known genre names used for auto-detecting the genre of a
/// downloaded track from the search query that found it. Each entry is matched
/// case-insensitively against the query string. The list is intentionally
/// exhaustive: it covers electronic, Greek, global and niche genres so that a
/// search like "frenchcore 2024" or "Λαϊκά τραγούδια" correctly tags the track.
/// The list is ordered longest-first within each category so that more specific
/// genres match before their parent (e.g. "Deep House" before "House").
const QStringList kGenreMatchList = {
        // Greek
        QStringLiteral("Ελληνική ντίσκο"),
        QStringLiteral("Ελληνικά τραγούδια"),
        QStringLiteral("Ελληνικό ραπ"),
        QStringLiteral("Ελληνικό ροκ"),
        QStringLiteral("Ελληνική ποπ"),
        QStringLiteral("Ρεμπέτικα"),
        QStringLiteral("Σκυλάδικα"),
        QStringLiteral("Ζεϊμπέκικα"),
        QStringLiteral("Νησιώτικα"),
        QStringLiteral("Δημοτικά"),
        QStringLiteral("Καψούρα"),
        QStringLiteral("Τραπίλες"),
        QStringLiteral("Ποντιακά"),
        QStringLiteral("Κρητικά"),
        QStringLiteral("Έντεχνα"),
        QStringLiteral("Λαϊκά"),
        QStringLiteral("Greek trap"),
        // House — most specific first
        QStringLiteral("Progressive House"),
        QStringLiteral("Soulful House"),
        QStringLiteral("Minimal House"),
        QStringLiteral("Jackin House"),
        QStringLiteral("Tribal House"),
        QStringLiteral("Future House"),
        QStringLiteral("Electro House"),
        QStringLiteral("Funky House"),
        QStringLiteral("Latin House"),
        QStringLiteral("Afro House"),
        QStringLiteral("Acid House"),
        QStringLiteral("Bass House"),
        QStringLiteral("Deep House"),
        QStringLiteral("Tech House"),
        QStringLiteral("Tropical House"),
        QStringLiteral("House"),
        // Techno
        QStringLiteral("Industrial Techno"),
        QStringLiteral("Peak Time Techno"),
        QStringLiteral("Hypnotic Techno"),
        QStringLiteral("Melodic Techno"),
        QStringLiteral("Minimal Techno"),
        QStringLiteral("Detroit Techno"),
        QStringLiteral("Hard Techno"),
        QStringLiteral("Dark Techno"),
        QStringLiteral("Acid Techno"),
        QStringLiteral("Raw Techno"),
        QStringLiteral("Dub Techno"),
        QStringLiteral("Techno"),
        // Trance
        QStringLiteral("Progressive Trance"),
        QStringLiteral("Uplifting Trance"),
        QStringLiteral("Dark Psytrance"),
        QStringLiteral("Vocal Trance"),
        QStringLiteral("Hard Trance"),
        QStringLiteral("Goa Trance"),
        QStringLiteral("Psytrance"),
        QStringLiteral("Full-On"),
        QStringLiteral("Hi-Tech"),
        QStringLiteral("Trance"),
        // DnB / Jungle
        QStringLiteral("Liquid DnB"),
        QStringLiteral("Ragga Jungle"),
        QStringLiteral("Neurofunk"),
        QStringLiteral("Drum & Bass"),
        QStringLiteral("Jump Up"),
        QStringLiteral("Darkstep"),
        QStringLiteral("Jungle"),
        // Hardcore / hard dance
        QStringLiteral("Industrial Hardcore"),
        QStringLiteral("Uptempo Hardcore"),
        QStringLiteral("Happy Hardcore"),
        QStringLiteral("UK Hardcore"),
        QStringLiteral("Hard Bounce"),
        QStringLiteral("Hard Dance"),
        QStringLiteral("Frenchcore"),
        QStringLiteral("Speedcore"),
        QStringLiteral("Terrorcore"),
        QStringLiteral("Breakcore"),
        QStringLiteral("Lolicore"),
        QStringLiteral("Hardstyle"),
        QStringLiteral("Rawstyle"),
        QStringLiteral("Hardcore"),
        QStringLiteral("Gabber"),
        QStringLiteral("J-Core"),
        // Bass music
        QStringLiteral("Colour Bass"),
        QStringLiteral("Future Bass"),
        QStringLiteral("Trap (EDM)"),
        QStringLiteral("UK Garage"),
        QStringLiteral("Bassline"),
        QStringLiteral("Brostep"),
        QStringLiteral("Tearout"),
        QStringLiteral("Dubstep"),
        QStringLiteral("Riddim"),
        QStringLiteral("Wave"),
        // Breakbeat / electro / disco
        QStringLiteral("Electroclash"),
        QStringLiteral("Space Disco"),
        QStringLiteral("Italo Disco"),
        QStringLiteral("Breakbeat"),
        QStringLiteral("Nu-Disco"),
        QStringLiteral("Big Beat"),
        QStringLiteral("Electro"),
        QStringLiteral("Disco"),
        // Dark / EBM / industrial
        QStringLiteral("Dark Electro"),
        QStringLiteral("Witch House"),
        QStringLiteral("Aggrotech"),
        QStringLiteral("Retrowave"),
        QStringLiteral("Synthwave"),
        QStringLiteral("Darkwave"),
        QStringLiteral("Cyberpunk"),
        QStringLiteral("Industrial"),
        QStringLiteral("EBM"),
        // Ambient / downtempo
        QStringLiteral("Dark Ambient"),
        QStringLiteral("Downtempo"),
        QStringLiteral("Chillout"),
        QStringLiteral("Trip Hop"),
        QStringLiteral("Ambient"),
        QStringLiteral("Lo-Fi"),
        QStringLiteral("Dub"),
        // Pop
        QStringLiteral("Electropop"),
        QStringLiteral("Dream Pop"),
        QStringLiteral("Indie Pop"),
        QStringLiteral("Synthpop"),
        QStringLiteral("K-Pop"),
        QStringLiteral("J-Pop"),
        QStringLiteral("Pop"),
        // Hip Hop / Rap
        QStringLiteral("Lo-Fi Hip Hop"),
        QStringLiteral("Cloud Rap"),
        QStringLiteral("Boom Bap"),
        QStringLiteral("Hip Hop"),
        QStringLiteral("Phonk"),
        QStringLiteral("Drill"),
        QStringLiteral("Grime"),
        QStringLiteral("Trap"),
        // R&B / Soul / Funk
        QStringLiteral("Neo-Soul"),
        QStringLiteral("Soul"),
        QStringLiteral("Funk"),
        QStringLiteral("R&B"),
        // Latin
        QStringLiteral("Baile Funk"),
        QStringLiteral("Reggaeton"),
        QStringLiteral("Merengue"),
        QStringLiteral("Bachata"),
        QStringLiteral("Dembow"),
        QStringLiteral("Cumbia"),
        QStringLiteral("Salsa"),
        QStringLiteral("Latin"),
        // African
        QStringLiteral("Afrobeats"),
        QStringLiteral("Afro Tech"),
        QStringLiteral("Amapiano"),
        QStringLiteral("Kwaito"),
        QStringLiteral("Gqom"),
        // Asian & fusion
        QStringLiteral("Kawaii Future Bass"),
        QStringLiteral("Kawaii Metal"),
        QStringLiteral("Future Funk"),
        QStringLiteral("Vaporwave"),
        QStringLiteral("City Pop"),
        // Rock / Metal
        QStringLiteral("Post-Punk"),
        QStringLiteral("Metalcore"),
        QStringLiteral("Deathcore"),
        QStringLiteral("Shoegaze"),
        QStringLiteral("Screamo"),
        QStringLiteral("Grunge"),
        QStringLiteral("Metal"),
        QStringLiteral("Punk"),
        QStringLiteral("Rock"),
        QStringLiteral("Emo"),
        // Reggae / Caribbean
        QStringLiteral("Dancehall"),
        QStringLiteral("Reggae"),
        QStringLiteral("Soca"),
        // Misc
        QStringLiteral("Jersey Club"),
        QStringLiteral("Baltimore Club"),
        QStringLiteral("Moombahton"),
        QStringLiteral("Footwork"),
        QStringLiteral("Minimal"),
        QStringLiteral("Bounce"),
        QStringLiteral("Glitch"),
        QStringLiteral("IDM"),
        QStringLiteral("EDM"),
};

/// Try to infer a genre from the given search query by matching against
/// kGenreMatchList. Returns the first (most specific) genre whose name appears
/// as a substring (case-insensitive) in the query, or an empty string if no
/// match is found.
QString inferGenreFromQuery(const QString& query) {
    const QString lower = query.toLower();
    for (const QString& genre : kGenreMatchList) {
        if (lower.contains(genre.toLower())) {
            return genre;
        }
    }
    return {};
}

/// Returns true for sidecar files that accompany a downloaded YouTube audio
/// file and should be excluded when looking for the actual audio bytes.
/// Matches: .info.json (yt-dlp metadata), .sponsorblock.json (SponsorBlock
/// fallback sidecar), .part (incomplete download temp file).
bool isYouTubeSidecarFile(const QString& name) {
    return name.endsWith(QStringLiteral(".info.json")) ||
            name.endsWith(QStringLiteral(".sponsorblock.json")) ||
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
    // Debounce timer for sidebar + home HTML rebuilds. Rapid-fire download
    // completions (e.g. batch auto-analyze) each want to rebuild; coalescing
    // them to a single repaint 120 ms after the last event keeps the UI
    // thread free during the download burst.
    m_rebuildTimer = new QTimer(this);
    m_rebuildTimer->setSingleShot(true);
    m_rebuildTimer->setInterval(120);
    connect(m_rebuildTimer, &QTimer::timeout, this, [this]() {
        rebuildSidebar();
        rebuildHomeHtml();
        // If upsertDownloadedRow() or syncAnalyzedTrackMetadata() deferred a
        // model update, apply it now — after the sidebar/HTML rebuild so the
        // user sees a consistent snapshot.
        if (m_pendingModelUpdate) {
            m_pendingModelUpdate = false;
            if (m_pTrackCache) {
                m_pTrackCache->buildIndex();
            }
            // On Android, the background library scan frequently holds the SQLite
            // write lock. BaseSqlTableModel::select() performs a synchronous read
            // which can block the UI thread if the lock is held, causing the
            // "YouTube lags when clicked" symptom. Defer the first select until
            // the event loop is free.
            QTimer::singleShot(0, this, [this]() {
                if (m_pTrackModel) {
                    m_pTrackModel->select();
                }
            });
        }
    });

    // Debounce timer for thumbnail dataChanged() notifications. Up to 20
    // thumbnails arrive in parallel; emitting dataChanged() on every arrival
    // causes a full view repaint for each one. Coalescing them into a single
    // emission 50 ms after the last arrival cuts repaints from N→1.
    m_thumbnailTimer = new QTimer(this);
    m_thumbnailTimer->setSingleShot(true);
    m_thumbnailTimer->setInterval(50);
    connect(m_thumbnailTimer, &QTimer::timeout, this, [this]() {
        if (!m_thumbnailsDirty || !m_pTrackModel) {
            return;
        }
        m_thumbnailsDirty = false;
        const int rows = m_pTrackModel->rowCount();
        if (rows > 0) {
            emit m_pTrackModel->dataChanged(
                    m_pTrackModel->index(0, 0),
                    m_pTrackModel->index(
                            rows - 1, m_pTrackModel->columnCount() - 1));
        }
    });
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
    // Tell the model where thumbnail images live so getCoverInfo() can
    // serve them without a schema change.
    m_pTrackModel->setThumbnailDir(thumbnailDir());

    // Per-view search box: typing in the search bar while the YouTube
    // pane is active fires YouTubeTrackModel::searchRequested → here →
    // a fresh YouTubeService search via the existing pipeline.
    connect(m_pTrackModel,
            &YouTubeTrackModel::searchRequested,
            this,
            &YouTubeFeature::searchAndActivate);
    // Infinite scroll: when the user scrolls to the bottom and the model
    // emits fetchMoreRequested(), ask the service for the next page.
    // After the call, restore m_hasMore based on whether the service still
    // holds a continuation token: if the service was throttled it schedules
    // a timer retry and the token is NOT consumed, so hasMoreSearchResults()
    // remains true — allowing canFetchMore() to return true again so the
    // auto-retry appends results without requiring another user scroll.
    connect(m_pTrackModel,
            &YouTubeTrackModel::fetchMoreRequested,
            this,
            [this]() {
                if (!m_lastQuery.isEmpty()) {
                    m_service.fetchMoreSearchResults(
                            m_lastQuery, kSearchResultsMax);
                    m_pTrackModel->setHasMore(
                            m_service.hasMoreSearchResults());
                }
            });
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
            &mixxx::YouTubeService::searchMoreReady,
            this,
            &YouTubeFeature::onSearchMoreReady);
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

QString YouTubeFeature::thumbnailDir() const {
    const QString dir = QDir(cacheDir()).filePath(QStringLiteral("thumbnails"));
    QDir().mkpath(dir);
    return dir;
}

void YouTubeFeature::fetchThumbnails(
        const QList<mixxx::YouTubeVideoInfo>& videos) {
    if (!m_pSamplesNam) {
        m_pSamplesNam = new QNetworkAccessManager(this);
    }
    const QString tDir = thumbnailDir();
    for (const mixxx::YouTubeVideoInfo& info : videos) {
        if (info.id.isEmpty()) {
            continue;
        }
        const QString destPath =
                tDir + QLatin1Char('/') + info.id + QStringLiteral(".jpg");
        if (QFileInfo::exists(destPath)) {
            continue; // already on disk
        }
        if (m_thumbnailsDownloading.contains(info.id)) {
            continue; // in flight
        }
        m_thumbnailsDownloading.insert(info.id);
        // mqdefault.jpg (320×180) is a good balance of visual quality and
        // download size. hqdefault.jpg (480×360) sometimes has black bars.
        const QUrl url(QStringLiteral("https://i.ytimg.com/vi/") + info.id +
                QStringLiteral("/mqdefault.jpg"));
        QNetworkRequest req(url);
        req.setTransferTimeout(10 * 1000); // 10 s
        QNetworkReply* reply = m_pSamplesNam->get(req);
        connect(reply,
                &QNetworkReply::finished,
                this,
                [this, reply, destPath, videoId = info.id]() {
                    reply->deleteLater();
                    m_thumbnailsDownloading.remove(videoId);
                    if (reply->error() != QNetworkReply::NoError) {
                        return; // non-fatal: no thumbnail is fine
                    }
                    QFile f(destPath);
                    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                        f.write(reply->readAll());
                        f.close();
                        // Schedule a batched dataChanged() notification so
                        // N parallel thumbnail arrivals produce only one
                        // full repaint (50 ms after the last arrival).
                        m_thumbnailsDirty = true;
                        if (m_thumbnailTimer) {
                            m_thumbnailTimer->start();
                        }
                    }
                });
    }
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
    Q_EMIT enableCoverArtDisplay(true); // show YouTube thumbnail cover art
    // Force a fresh SELECT so the table reflects the current contents of
    // youtube_library even if the model state went stale while another
    // feature was in the foreground (e.g. results landed in the background
    // after a combined library/YouTube search).
    // On Android, the background library scan frequently holds the SQLite
    // write lock. BaseSqlTableModel::select() performs a synchronous read
    // which can block the UI thread if the lock is held, causing the
    // "YouTube lags when clicked" symptom. Defer the first select until
    // the event loop is free.
    QTimer::singleShot(0, this, [this]() {
        if (m_pTrackModel) {
            m_pTrackModel->select();
        }
    });
    // Defer the expensive rebuildHomeHtml() — it builds a massive HTML string
    // with all results, genres, etc. and blocks the UI thread. The track table
    // is the primary view now; the HTML pane is only a fallback.
    // rebuildHomeHtml() is still called from bindLibraryWidget() and
    // onSearchResultsReady() when results actually change.
    // rebuildHomeHtml();
    rebuildSidebar();
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
        kLogger.info() << "Fetching YouTube trending for region" << country;
        m_service.fetchTrending(country, kSearchResultsMax, kSearchResultsMax);
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

void YouTubeFeature::bindSearchboxWidget(WSearchLineEdit* pSearchboxWidget) {
    if (!pSearchboxWidget || !m_pTrackModel) {
        return;
    }
    // Connect returnPressed (Enter key) to searchNow() so YouTube only
    // searches when the user explicitly presses Enter, not on every
    // keystroke-debounced signal.
    connect(pSearchboxWidget,
            &WSearchLineEdit::returnPressed,
            m_pTrackModel,
            &YouTubeTrackModel::searchNow);
}

void YouTubeFeature::onHomeAnchorClicked(const QUrl& url) {
    const QString scheme = url.scheme();
    if (scheme == kHomeAutoAnalyzeScheme) {
        setAutoAnalyzeResultsEnabled(!autoAnalyzeResultsEnabled());
        return;
    }
    // Genre links: "ytgenre:House" → search for "House songs <year>"
    if (scheme == kHomeGenreScheme) {
        const QString genre = url.path();
        if (!genre.isEmpty()) {
            const QString query = genre + QStringLiteral(" songs ") +
                    QString::number(QDate::currentDate().year());
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
        const QString query = genre + QStringLiteral(" songs ") +
                QString::number(QDate::currentDate().year());
        searchAndActivate(query);
    } else if (payload.startsWith(kSampleQueryPrefix)) {
        // User clicked a DJ sample from the sidebar Samples section. Fire a
        // YouTube search so results appear in the main track table AND set the
        // sampler-target flag so onSearchResultsReady() auto-downloads the
        // first result and loads it into the next available sampler slot.
        m_samplerTargetSearch = true;
        searchAndActivate(payload.mid(kSampleQueryPrefix.size()));
    } else if (payload == kMyInstantFetch) {
        // First-time expand of Greek Memes: start the live fetch.
        fetchMyInstantsSounds();
    } else if (payload.startsWith(kMyInstantPrefix)) {
        // User clicked a Greek meme sound: download its MP3 and load it.
        const QString mp3Url = payload.mid(kMyInstantPrefix.size());
        const QString name = pItem->getLabel();
        downloadMyInstant(mp3Url, name);
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
    // If searchAndActivate() is called from anything other than a sidebar
    // Samples click (genre click, search box, trending), clear the sampler
    // intent so a coincidental search doesn't accidentally steal a sampler
    // slot. The flag is set explicitly by activateChild(kSampleQueryPrefix).
    // We do NOT clear it here because activateChild sets it BEFORE calling
    // searchAndActivate(), so clearing it here would always wipe it.
    rebuildSidebar();
    // Clear the SQL model synchronously so the user does NOT see stale
    // results filtered by the new query. Set the search to the query text
    // (not empty) so the search box preserves what the user typed.
    if (m_pTrackModel) {
        m_pTrackModel->setSearch(query);
    }
    replaceTrackTable({});
    m_service.searchVideos(query, kSearchResultsMax, kSearchResultsMax);
}

void YouTubeFeature::onSearchResultsReady(
        const QString& query, const QList<mixxx::YouTubeVideoInfo>& results) {
    if (query != m_lastQuery) {
        return; // a newer search has superseded this one
    }
    m_trendingFetchInFlight = false;
    // For the trending/home feed, drop hour-long "megamix" / "best of"
    // compilations — a DJ wants individual songs, not 1-hour continuous mixes.
    // For an explicit user search the filter is intentionally NOT applied:
    // the user may be searching for an extended version, a live set, or a
    // DJ mix on purpose, so we must not silently hide results. The constant
    // name "kTrendingMaxDurationSec" reflects this scope. Items with an
    // unknown duration (0) are always kept to avoid hiding genuine songs
    // whose length failed to parse. Live streams are excluded upstream by
    // collectInnerTubeVideos().
    const bool isTrendingQuery =
            query.startsWith(mixxx::YouTubeService::kTrendingQueryPrefix);
    QList<mixxx::YouTubeVideoInfo> filtered;
    filtered.reserve(results.size());
    for (const mixxx::YouTubeVideoInfo& info : results) {
        if (isTrendingQuery && info.durationSec > 0 &&
                info.durationSec > kTrendingMaxDurationSec) {
            continue;
        }
        filtered.append(info);
    }
    m_lastResults = filtered;
    m_lastSearchError.clear();
    // Defer the sidebar rebuild — rebuildHomeHtml() is expensive (builds
    // 200+ genre links + full result list HTML) and the track table is the
    // primary view now, so there's no reason to block the UI thread here.
    // scheduleRebuild() coalesces rapid calls within 120 ms.
    scheduleRebuild();
    // Real fix: replace the rows in youtube_library so the user sees a
    // proper Title/Artist/Duration table, sortable, draggable, double-
    // clickable — not an HTML link list.
    replaceTrackTable(filtered);
    // Tell the model whether the service has more results to offer.
    // canFetchMore() / fetchMore() use this to trigger the next page when
    // the user scrolls to the bottom.
    if (m_pTrackModel) {
        m_pTrackModel->setHasMore(m_service.hasMoreSearchResults());
    }
    // Fetch thumbnails for all results so the cover-art column shows
    // YouTube thumbnail images instead of the generic placeholder.
    fetchThumbnails(filtered);
    // Bring the populated track table to the foreground. Without this the
    // results only ever land in the model: if the visible pane was something
    // else when the (asynchronous) network reply arrived — e.g. the local
    // library after a combined search, or the YouTube node before the reply —
    // the user saw an empty main area and the results "stuck" in the sidebar
    // tree only. This is the user-reported "I have YouTube on the left bar but
    // no actual results" symptom. Showing the model switches the main pane to
    // the freshly-filled table and clears any stale search-box filter.
    Q_EMIT showTrackModel(m_pTrackModel);
    // Restore the search box text that was cleared by setSearch("") inside
    // replaceTrackTable(). Library::slotShowTrackModel emits restoreSearch("")
    // (currentSearch() is now empty), which blanks the search box. Re-emit
    // the actual query so the user sees what they typed.
    if (!m_lastQuery.isEmpty() &&
            !m_lastQuery.startsWith(mixxx::YouTubeService::kTrendingQueryPrefix)) {
        Q_EMIT restoreSearch(m_lastQuery);
    }
    if (autoAnalyzeResultsEnabled()) {
        autoAnalyzeCurrentResults();
    }
    // Auto-download the first result to the next available sampler when this
    // search was triggered from the sidebar Samples section (DJ Tools).
    if (m_samplerTargetSearch && !filtered.isEmpty()) {
        m_samplerTargetSearch = false;
        const QString firstId = filtered.first().id;
        if (!firstId.isEmpty()) {
            // Find the next empty sampler group — use ControlProxy for a
            // lightweight one-shot read, no persistent connection needed.
            QString samplerGroup;
            for (int i = 1; i <= 32; ++i) {
                const QString g = QStringLiteral("[Sampler%1]").arg(i);
                ControlProxy cp(ConfigKey(g, QStringLiteral("track_loaded")));
                if (cp.get() == 0.0) {
                    samplerGroup = g;
                    break;
                }
            }
            if (!samplerGroup.isEmpty()) {
                // Register the download intent so onDownloadFinished can load
                // into this specific sampler slot.
                m_pendingPlayerLoads[firstId].append({samplerGroup, false});
                requestDownloadFile(firstId);
            }
        }
    }
}

void YouTubeFeature::onSearchMoreReady(
        const QString& query, const QList<mixxx::YouTubeVideoInfo>& results) {
    if (query != m_lastQuery) {
        return; // superseded by a newer search
    }
    if (results.isEmpty()) {
        if (m_pTrackModel) {
            m_pTrackModel->setHasMore(false);
        }
        return;
    }
    // Append the new batch below the existing results.
    appendToTrackTable(results);
    // Accumulate into m_lastResults so rebuildSidebar() shows full count.
    m_lastResults.append(results);
    // Defer the sidebar / HTML rebuild — scheduleRebuild() coalesces rapid
    // continuation-page arrivals into a single UI update 120 ms after the last.
    scheduleRebuild();
    // Update hasMore for the freshly-fetched page.
    if (m_pTrackModel) {
        m_pTrackModel->setHasMore(m_service.hasMoreSearchResults());
    }
    // Pre-fetch thumbnails for the new batch.
    fetchThumbnails(results);
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
    // YouTubeService at this point (or, on cut failure, a .sponsorblock.json
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
    // Batch downloads (auto-analyze) each emit onDownloadFinished; debounce
    // the paired sidebar+home rebuild so the UI thread isn't churning HTML
    // after every individual file lands.
    scheduleRebuild();
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
    // Auto-detect genre from the search query that found this track. If the
    // user searched for "frenchcore 2024" or clicked the "Hard Techno" genre
    // shortcut, the track gets tagged automatically so the library's genre
    // column is useful without manual tagging.
    if (pTrack->getGenre().isEmpty() && !m_lastQuery.isEmpty()) {
        const QString detectedGenre = inferGenreFromQuery(m_lastQuery);
        if (!detectedGenre.isEmpty()) {
            pTrack->updateGenre(detectedGenre);
        }
    }
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
    // Defer buildIndex()+select() — analysis completion fires once per track,
    // and during batch auto-analyze that can be many tracks in a short window.
    // Coalescing into one rebuild via the 120 ms debounce timer avoids N heavy
    // select() calls.
    m_pendingModelUpdate = true;
    scheduleRebuild();
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
        // SponsorBlock sidecar: canonical name is VIDEOID.sponsorblock.json
        // (same as what SponsorBlockController reads). Remove it alongside
        // the audio file.
        const QString sidecar = QDir(QFileInfo(location).absolutePath())
                                        .filePath(videoId + QStringLiteral(".sponsorblock.json"));
        QFile::remove(sidecar);
        if (trackId.isValid()) {
            m_pLibrary->trackCollectionManager()->purgeTracks(
                    {TrackRef::fromFilePath(location, trackId)});
        }
        m_downloadedTracks.remove(videoId);
        scheduleRebuild();
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
        pGenresNode->appendChild(genre, QString(kGenrePrefix + genre));
    }

    // Note: search/trending results are intentionally NOT listed as sidebar
    // tree children. They belong in the main track table (the right-hand
    // pane), exactly like every other library source — listing each song in
    // the left sidebar instead confused users ("why do I have YouTube on the
    // left bar and not actual results?"). The table is populated by
    // replaceTrackTable() and surfaced via showTrackModel(); the sidebar only
    // carries navigation/toggle nodes.

    // Samples node — contains two sub-sections:
    //   • "DJ Tools": curated classic DJ sound effects searched via YouTube.
    //     Clicking an entry fires a YouTube search so the user can pick any
    //     version, then drag/download it to their sampler pads.
    //   • "Greek Memes": sound clips fetched live from myinstants.com/gr.
    //     Clicking an entry downloads the MP3 directly and loads it.
    {
        // Curated DJ sample searches — (search query, display label) pairs.
        // The query is sent verbatim to YouTube; results appear in the main
        // track table just like any manual search.
        static const QList<QPair<QString, QString>> kDJSamples = {
                {QStringLiteral("dj air horn sound effect short"),
                        tr("Air Horn")},
                {QStringLiteral("vinyl record scratch dj sound effect"),
                        tr("Record Scratch")},
                {QStringLiteral("dj vinyl rewind turntable effect"),
                        tr("Vinyl Rewind")},
                {QStringLiteral("bass drop sound effect edm"),
                        tr("Bass Drop")},
                {QStringLiteral("crowd cheering applause sound effect"),
                        tr("Crowd Cheer")},
                {QStringLiteral("rimshot drum comedy sound effect"),
                        tr("Rimshot")},
                {QStringLiteral("dj siren sound effect"),
                        tr("DJ Siren")},
                {QStringLiteral("whoosh transition swoosh sound effect"),
                        tr("Whoosh FX")},
                {QStringLiteral("orchestra hit stab sound effect"),
                        tr("Orchestra Hit")},
                {QStringLiteral("sad trombone wah wah sound effect"),
                        tr("Sad Trombone")},
                {QStringLiteral("foghorn sound effect short"),
                        tr("Foghorn")},
                {QStringLiteral("reggae airhorn dancehall sound effect"),
                        tr("Reggae Horn")},
                {QStringLiteral("dj laser sound effect"),
                        tr("Laser FX")},
                {QStringLiteral("big room edm drop build sound effect"),
                        tr("Big Room Drop")},
                {QStringLiteral("trap 808 clap snare sound effect"),
                        tr("Trap Clap")},
                {QStringLiteral("dj rewind reverse effect"),
                        tr("Rewind FX")},
        };

        TreeItem* pSamplesNode = pRoot->appendChild(tr("Samples"));
        TreeItem* pDJNode = pSamplesNode->appendChild(tr("🎛 DJ Tools"));
        for (const auto& [query, label] : kDJSamples) {
            pDJNode->appendChild(label, QString(kSampleQueryPrefix + query));
        }

        TreeItem* pMemesNode = pSamplesNode->appendChild(tr("😂 Greek Memes"));
        if (m_myInstantsFetchInFlight) {
            // Spinner placeholder while the network request is in flight.
            pMemesNode->appendChild(tr("Loading…"));
        } else if (m_myInstantSounds.isEmpty()) {
            // First visit: show a single click-to-fetch entry.
            pMemesNode->appendChild(tr("Fetch sounds from myinstants.com"),
                    kMyInstantFetch);
        } else {
            // Sounds loaded — list them. The payload carries the CDN URL so
            // activateChild() knows where to download from.
            for (auto it = m_myInstantSounds.cbegin(); it != m_myInstantSounds.cend(); ++it) {
                pMemesNode->appendChild(it->name, QString(kMyInstantPrefix + it->mp3Url));
            }
        }
    }

    if (!m_downloadedTracks.isEmpty()) {
        TreeItem* pCachedNode = pRoot->appendChild(tr("Downloaded"));
        const QDir dir(cacheDir());
        // Pre-scan the cache directory ONCE so we don't pay one
        // dir.entryList({id+".*"}) glob call per downloaded track (which on
        // Android emulated storage was ~10-50 ms × N tracks of UI-thread
        // stall — the "clicking YouTube freezes" symptom whenever a sidebar
        // rebuild fires after a new download).
        QHash<QString, QString> cachedFiles; // videoId → local path
        const QStringList allFiles =
                dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        for (const QString& f : allFiles) {
            if (isYouTubeSidecarFile(f)) {
                continue;
            }
            const int dotIdx = f.lastIndexOf(QLatin1Char('.'));
            if (dotIdx > 0) {
                const QString id = f.left(dotIdx);
                if (!cachedFiles.contains(id)) {
                    cachedFiles.insert(id, dir.filePath(f));
                }
            }
        }
        for (auto it = m_downloadedTracks.cbegin();
                it != m_downloadedTracks.cend();
                ++it) {
            const QString localPath = cachedFiles.value(it.key());
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
            // the logs are in the app's documents folder which is browsable
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

void YouTubeFeature::scheduleRebuild() {
    // Restart the timer on every call — fires 120 ms after the last request,
    // coalescing N calls within the window into a single rebuild pair.
    m_rebuildTimer->start();
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
        // Use the native SQLite error code for a precise check:
        //   SQLITE_BUSY (5)   — another connection holds a write lock
        //   SQLITE_LOCKED (6) — intra-connection lock (same connection, rare)
        // Falling back to substring matching only when the driver does not
        // expose a numeric code (e.g. QPSQL shim in tests).
        const QString nativeCode = del.lastError().nativeErrorCode();
        const bool locked = (nativeCode == QLatin1String("5") ||
                nativeCode == QLatin1String("6") ||
                errText.contains(QStringLiteral("locked"),
                        Qt::CaseInsensitive));
        // Retry on the event loop (non-blocking) for up to ~300 s, which
        // comfortably outlasts even a full first-run device scan.
        constexpr int kRetryIntervalMs = 250;
        // The Android library scanner holds the SQLite write lock for the full
        // duration of the initial scan, which can exceed 3 minutes on large
        // libraries (Camera roll, SD-card backup, Downloads). 10 s was far too
        // short — all YouTube search results silently disappeared whenever the
        // user searched during startup. 300 s comfortably outlasts even a full
        // first-run scan while still bounding the pending-results queue.
        constexpr int kRetryTimeoutMs = 300 * 1000;
        constexpr int kMaxAttempts = kRetryTimeoutMs / kRetryIntervalMs;
        if (locked && attempt < kMaxAttempts) {
            transaction.rollback();
            const QList<mixxx::YouTubeVideoInfo> pending = videos;
            QPointer<YouTubeFeature> guard(this);
            QTimer::singleShot(kRetryIntervalMs, this, [guard, pending, attempt]() {
                if (guard) {
                    guard->replaceTrackTable(pending, attempt + 1);
                }
            });
            return;
        }
        kLogger.warning() << "youtube_library DELETE failed:" << errText;
        return;
    }

    QSqlQuery ins(db);
    ins.prepare(QStringLiteral(
            "INSERT INTO youtube_library "
            "(artist, title, album, genre, location, comment, duration) "
            "VALUES (:artist, :title, :album, :genre, :location, :comment, :duration)"));

    const QString detectedGenre = inferGenreFromQuery(m_lastQuery);
    const QDir dir(cacheDir());
    // Pre-scan the cache directory ONCE so we don't pay one QDir::entryList()
    // call per video (which on Android emulated storage was 10–50 ms × 50
    // videos = up to 2.5 s of UI-thread stall — the "clicking YouTube freezes
    // everything" symptom).
    QHash<QString, QString> cachedFiles; // videoId → local path
    const QStringList allFiles =
            dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QString& f : allFiles) {
        if (isYouTubeSidecarFile(f)) {
            continue;
        }
        const int dotIdx = f.lastIndexOf(QLatin1Char('.'));
        if (dotIdx > 0) {
            const QString id = f.left(dotIdx);
            if (!cachedFiles.contains(id)) {
                cachedFiles.insert(id, dir.filePath(f));
            }
        }
    }

    for (const mixxx::YouTubeVideoInfo& info : videos) {
        if (info.id.isEmpty()) {
            continue;
        }
        // Determine whether we already have a downloaded file for this id
        // — if so, point `location` at the real path so double-click loads
        // immediately via BaseExternalTrackModel::getTrack(). Otherwise
        // store the placeholder URI; YouTubeTrackModel::getTrack()
        // intercepts placeholders to kick off the download.
        const QString location = cachedFiles.value(
                info.id, YouTubeTrackModel::kPlaceholderScheme + info.id);
        const TrackDisplayMetadata metadata = displayMetadataForVideo(info);
        ins.bindValue(QStringLiteral(":artist"), metadata.artist);
        ins.bindValue(QStringLiteral(":title"), metadata.title);
        ins.bindValue(QStringLiteral(":album"), QStringLiteral("YouTube"));
        ins.bindValue(QStringLiteral(":genre"), detectedGenre);
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

void YouTubeFeature::appendToTrackTable(
        const QList<mixxx::YouTubeVideoInfo>& videos) {
    if (!m_pTrackModel || !m_pTrackCache) {
        return;
    }
    QSqlDatabase db = m_pTrackCollection->database();
    ScopedTransaction transaction(db);

    QSqlQuery ins(db);
    ins.prepare(QStringLiteral(
            "INSERT OR IGNORE INTO youtube_library "
            "(artist, title, album, genre, location, comment, duration) "
            "VALUES (:artist, :title, :album, :genre, :location, :comment, :duration)"));

    const QString detectedGenre = inferGenreFromQuery(m_lastQuery);
    const QDir dir(cacheDir());
    // Pre-scan the cache directory once (same optimisation as replaceTrackTable).
    QHash<QString, QString> cachedFiles;
    const QStringList allFiles =
            dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QString& f : allFiles) {
        if (isYouTubeSidecarFile(f)) {
            continue;
        }
        const int dotIdx = f.lastIndexOf(QLatin1Char('.'));
        if (dotIdx > 0) {
            const QString id = f.left(dotIdx);
            if (!cachedFiles.contains(id)) {
                cachedFiles.insert(id, dir.filePath(f));
            }
        }
    }

    for (const mixxx::YouTubeVideoInfo& info : videos) {
        if (info.id.isEmpty()) {
            continue;
        }
        // Resolve the location same way as replaceTrackTable().
        const QString location = cachedFiles.value(
                info.id, YouTubeTrackModel::kPlaceholderScheme + info.id);
        const TrackDisplayMetadata metadata = displayMetadataForVideo(info);
        ins.bindValue(QStringLiteral(":artist"), metadata.artist);
        ins.bindValue(QStringLiteral(":title"), metadata.title);
        ins.bindValue(QStringLiteral(":album"), QStringLiteral("YouTube"));
        ins.bindValue(QStringLiteral(":genre"), detectedGenre);
        ins.bindValue(QStringLiteral(":location"), location);
        ins.bindValue(QStringLiteral(":comment"), info.id);
        ins.bindValue(QStringLiteral(":duration"), info.durationSec);
        if (!ins.exec()) {
            kLogger.debug() << "youtube_library append INSERT skipped:"
                            << ins.lastError().text();
        }
    }
    transaction.commit();

    m_pTrackCache->buildIndex();
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
    // Defer buildIndex()+select() via the rebuild debounce timer. When N
    // downloads complete in rapid succession (auto-analyze mode) this prevents
    // N consecutive heavy rebuilds — only one fires 120 ms after the last.
    m_pendingModelUpdate = true;
    scheduleRebuild();
}

void YouTubeFeature::onRightClick(const QPoint& globalPos) {
    BaseExternalLibraryFeature::onRightClick(globalPos);
    QMenu menu(nullptr);
    QAction cleanAction(tr("Clean YouTube cache…"), &menu);
    menu.addAction(&cleanAction);
    const QAction* chosen = menu.exec(globalPos);
    if (chosen == &cleanAction) {
        slotCleanCache();
    }
}

void YouTubeFeature::slotCleanCache() {
    const QDir cache(cacheDir());
    if (!cache.exists()) {
        return;
    }
    // Collect all audio files (not sidecars or .part temps) in the cache dir.
    const QStringList files = cache.entryList(
            {QStringLiteral("*.m4a"),
                    QStringLiteral("*.webm"),
                    QStringLiteral("*.mp4"),
                    QStringLiteral("*.ogg"),
                    QStringLiteral("*.opus")},
            QDir::Files | QDir::NoDotAndDotDot);

    auto* pTcm = m_pLibrary ? m_pLibrary->trackCollectionManager() : nullptr;
    auto* pInternal = pTcm ? pTcm->internalCollection() : nullptr;
    const auto loaded = PlayerInfo::instance().getLoadedTracks();

    int cleaned = 0;
    for (const QString& fileName : files) {
        const QString location = cache.filePath(fileName);

        // Skip files still loaded on a deck.
        bool onDeck = false;
        for (auto it = loaded.cbegin(); it != loaded.cend(); ++it) {
            if (it.value() && it.value()->getLocation() == location) {
                onDeck = true;
                break;
            }
        }
        if (onDeck) {
            continue;
        }

        // Skip files referenced by a playlist or crate.
        if (pInternal) {
            const TrackPointer pTrack =
                    pInternal->getTrackDAO().getTrackByRef(
                            TrackRef::fromFilePath(location));
            if (pTrack) {
                const TrackId trackId = pTrack->getId();
                QSet<int> playlistSet;
                pInternal->getPlaylistDAO().getPlaylistsTrackIsIn(
                        trackId, &playlistSet);
                if (!playlistSet.isEmpty()) {
                    continue;
                }
                if (pInternal->crates()
                                .selectTrackCratesSorted(trackId)
                                .next()) {
                    continue;
                }
            }
        }

        // Safe to delete.
        if (QFile::remove(location)) {
            ++cleaned;
            const QString videoId = QFileInfo(location).completeBaseName();
            kLogger.info() << "Cache clean: removed" << videoId;
            // Remove sidecar if present (canonical name: VIDEOID.sponsorblock.json).
            QFile::remove(QDir(QFileInfo(location).absolutePath())
                            .filePath(videoId +
                                    QStringLiteral(".sponsorblock.json")));
            m_downloadedTracks.remove(videoId);
            // Purge from main library DB.
            if (pTcm && pInternal) {
                const TrackPointer pTrack =
                        pInternal->getTrackDAO().getTrackByRef(
                                TrackRef::fromFilePath(location));
                if (pTrack) {
                    pTcm->purgeTracks({TrackRef::fromFilePath(location, pTrack->getId())});
                }
            }
        }
    }

    kLogger.info() << "YouTube cache clean: removed" << cleaned << "of"
                   << files.size() << "cached files";

    // Also clean downloaded myinstants samples.
    int myInstantsCleaned = 0;
    const QDir myDir(myInstantsCacheDir());
    if (myDir.exists()) {
        const QStringList myFiles = myDir.entryList(
                {QStringLiteral("*.mp3")},
                QDir::Files | QDir::NoDotAndDotDot);
        for (const QString& f : myFiles) {
            const QString loc = myDir.filePath(f);
            bool inUse = false;
            for (auto it = loaded.cbegin(); it != loaded.cend(); ++it) {
                if (it.value() && it.value()->getLocation() == loc) {
                    inUse = true;
                    break;
                }
            }
            if (!inUse && QFile::remove(loc)) {
                ++myInstantsCleaned;
            }
        }
    }
    if (myInstantsCleaned > 0) {
        kLogger.info() << "Removed" << myInstantsCleaned << "myinstants sample(s)";
    }

    // Clean up stale youtube_library rows whose local files are gone
    // (non-placeholder rows where the file was already deleted above).
    if (m_pTrackCollection) {
        QSqlDatabase db = m_pTrackCollection->database();
        QSqlQuery rows(db);
        rows.prepare(QStringLiteral(
                "SELECT id, location FROM youtube_library "
                "WHERE location NOT LIKE 'youtube://%'"));
        if (rows.exec()) {
            QList<int> toDelete;
            while (rows.next()) {
                const QString loc = rows.value(1).toString();
                if (!QFileInfo::exists(loc)) {
                    toDelete.append(rows.value(0).toInt());
                }
            }
            for (int rowId : toDelete) {
                QSqlQuery del(db);
                del.prepare(QStringLiteral(
                        "DELETE FROM youtube_library WHERE id = :id"));
                del.bindValue(QStringLiteral(":id"), rowId);
                del.exec();
            }
        }
    }

    rebuildSidebar();
    rebuildHomeHtml();
    if (m_pTrackCache) {
        m_pTrackCache->buildIndex();
    }
    // On Android, the background library scan frequently holds the SQLite
    // write lock. BaseSqlTableModel::select() performs a synchronous read
    // which can block the UI thread if the lock is held, causing the
    // "YouTube lags when clicked" symptom. Defer the first select until
    // the event loop is free.
    QTimer::singleShot(0, this, [this]() {
        if (m_pTrackModel) {
            m_pTrackModel->select();
        }
    });

    QMessageBox::information(nullptr,
            tr("YouTube Cache"),
            tr("Removed %n downloaded track(s) from the cache.", "", cleaned + myInstantsCleaned));
}

// =============================================================================
// Samples section — DJ Tools (YouTube searches) + Greek Memes (myinstants.com)
// =============================================================================

QString YouTubeFeature::myInstantsCacheDir() const {
    return cacheDir() + QStringLiteral("/myinstants");
}

void YouTubeFeature::fetchMyInstantsSounds() {
    if (m_myInstantsFetchInFlight) {
        return;
    }
    m_myInstantsFetchInFlight = true;
    rebuildSidebar(); // show "Loading…" placeholder

    // Lazy-create a QNetworkAccessManager dedicated to the Samples section.
    if (!m_pSamplesNam) {
        m_pSamplesNam = new QNetworkAccessManager(this);
    }

    QNetworkRequest req(QUrl(QStringLiteral("https://www.myinstants.com/en/index/gr/")));
    // Use a browser-like UA so myinstants.com doesn't serve a 403 to the app.
    req.setRawHeader("User-Agent",
            "Mozilla/5.0 (Linux; Android 10) AppleWebKit/537.36 "
            "(KHTML, like Gecko) Chrome/120.0 Mobile Safari/537.36");
    req.setRawHeader("Accept", "text/html,application/xhtml+xml");
    req.setTransferTimeout(10000); // 10 s — generous but bounded
    QNetworkReply* reply = m_pSamplesNam->get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        m_myInstantsFetchInFlight = false;

        if (reply->error() != QNetworkReply::NoError) {
            kLogger.warning() << "myinstants.com fetch failed:"
                              << reply->errorString();
            rebuildSidebar();
            return;
        }

        const QString html = QString::fromUtf8(reply->readAll());
        QList<MyInstantSound> sounds;
        QSet<QString> seen; // dedup by CDN path

        // myinstants.com HTML patterns (cope with minor version changes):
        //   onclick="play('/media/sounds/FILE.mp3', …)">TITLE<
        // or data-url="/media/sounds/FILE.mp3"
        static const QRegularExpression reOnclick(
                QStringLiteral(
                        R"(onclick="play\('(/media/sounds/[^']+\.mp3)'[^>]*>\s*([^<]+?)\s*<)"),
                QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression reDataUrl(
                QStringLiteral(
                        R"re(data-url="(/media/sounds/[^"]+\.mp3)"[^>]*>\s*([^<]+?)\s*<)re"),
                QRegularExpression::CaseInsensitiveOption);

        {
            auto it = reOnclick.globalMatch(html);
            while (it.hasNext() && sounds.size() < 80) {
                const auto m = it.next();
                const QString path = m.captured(1).trimmed();
                const QString name = m.captured(2).trimmed();
                if (name.isEmpty() || seen.contains(path)) {
                    continue;
                }
                seen.insert(path);
                sounds.append({name,
                        QStringLiteral("https://www.myinstants.com") + path});
            }
        }
        if (sounds.isEmpty()) {
            auto it = reDataUrl.globalMatch(html);
            while (it.hasNext() && sounds.size() < 80) {
                const auto m = it.next();
                const QString path = m.captured(1).trimmed();
                const QString name = m.captured(2).trimmed();
                if (name.isEmpty() || seen.contains(path)) {
                    continue;
                }
                seen.insert(path);
                sounds.append({name,
                        QStringLiteral("https://www.myinstants.com") + path});
            }
        }

        if (sounds.isEmpty()) {
            kLogger.warning()
                    << "myinstants.com: page fetched but no sounds found "
                       "(HTML structure may have changed)";
        } else {
            kLogger.info() << "myinstants.com: loaded" << sounds.size()
                           << "Greek meme sounds";
        }

        m_myInstantSounds = sounds;
        rebuildSidebar();
    });
}

void YouTubeFeature::downloadMyInstant(
        const QString& mp3Url, const QString& displayName) {
    const QString dir = myInstantsCacheDir();
    QDir().mkpath(dir);

    const QString filename = QUrl(mp3Url).fileName();
    if (filename.isEmpty()) {
        kLogger.warning() << "Cannot derive filename from myinstants URL:" << mp3Url;
        return;
    }
    const QString localPath = dir + QStringLiteral("/") + filename;

    // If already on disk, just load it immediately — no network needed.
    if (QFileInfo::exists(localPath)) {
        TrackRef ref = TrackRef::fromFilePath(localPath);
        TrackPointer pTrack =
                m_pLibrary->trackCollectionManager()->getOrAddTrack(ref);
        if (pTrack) {
            if (pTrack->getTitle().isEmpty()) {
                pTrack->setTitle(displayName);
            }
            loadTrackToNextSampler(pTrack);
        }
        return;
    }

    if (m_myInstantsDownloading.contains(mp3Url)) {
        return; // already in-flight
    }
    m_myInstantsDownloading.insert(mp3Url);

    if (!m_pSamplesNam) {
        m_pSamplesNam = new QNetworkAccessManager(this);
    }

    kLogger.info() << "Downloading myinstants sample:" << displayName
                   << "from" << mp3Url;

    QNetworkRequest req{QUrl(mp3Url)};
    req.setRawHeader("User-Agent",
            "Mozilla/5.0 (Linux; Android 10) AppleWebKit/537.36 "
            "(KHTML, like Gecko) Chrome/120.0 Mobile Safari/537.36");
    req.setRawHeader("Referer", "https://www.myinstants.com/");
    req.setTransferTimeout(15000);
    QNetworkReply* reply = m_pSamplesNam->get(req);

    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, mp3Url, localPath, displayName]() {
                reply->deleteLater();
                m_myInstantsDownloading.remove(mp3Url);

                if (reply->error() != QNetworkReply::NoError) {
                    kLogger.warning()
                            << "myinstants download failed for" << displayName
                            << ":" << reply->errorString();
                    return;
                }

                QFile file(localPath);
                if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    kLogger.warning() << "Cannot write myinstants sample to"
                                      << localPath;
                    return;
                }
                file.write(reply->readAll());
                file.close();

                kLogger.info() << "myinstants sample saved:" << localPath;

                TrackRef ref = TrackRef::fromFilePath(localPath);
                TrackPointer pTrack =
                        m_pLibrary->trackCollectionManager()->getOrAddTrack(ref);
                if (pTrack) {
                    if (pTrack->getTitle().isEmpty()) {
                        pTrack->setTitle(displayName);
                    }
                    if (pTrack->getGenre().isEmpty()) {
                        pTrack->updateGenre(QStringLiteral("Meme Sound"));
                    }
                    // Load into the next available sampler slot so the sound
                    // lands in the SAMPLERS rack, not a main deck.
                    loadTrackToNextSampler(pTrack);
                }
            });
}

void YouTubeFeature::loadTrackToNextSampler(const TrackPointer& pTrack) {
    if (!pTrack) {
        return;
    }
    // Find the first empty sampler slot (1–32). A "one-shot" ControlProxy
    // with no parent gives us a lightweight read without a persistent
    // connection — we only need the current value.
    QString samplerGroup;
    for (int i = 1; i <= 32; ++i) {
        const QString g = QStringLiteral("[Sampler%1]").arg(i);
        ControlProxy cp(ConfigKey(g, QStringLiteral("track_loaded")));
        if (cp.get() == 0.0) {
            samplerGroup = g;
            break;
        }
    }
    if (samplerGroup.isEmpty()) {
        // All samplers occupied — fall back to the default deck-load path
        // so the user still gets the track rather than losing it silently.
        kLogger.info() << "All sampler slots occupied — loading to next deck instead";
        Q_EMIT loadTrack(pTrack);
        return;
    }
    kLogger.info() << "Loading sample" << pTrack->getTitle()
                   << "into sampler slot" << samplerGroup;
#ifdef __STEM__
    Q_EMIT loadTrackToPlayer(pTrack, samplerGroup, mixxx::StemChannelSelection(), false);
#else
    Q_EMIT loadTrackToPlayer(pTrack, samplerGroup, false);
#endif
}

#include "moc_youtubefeature.cpp"
