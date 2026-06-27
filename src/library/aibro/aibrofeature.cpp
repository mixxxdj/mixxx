#include "library/aibro/aibrofeature.h"

#include <QDateTime>
#include <QRegularExpression>
#include <QSet>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <cmath>
#include <utility>

#include "control/controlproxy.h"
#include "library/aibro/musicmatcherclient.h"
#include "library/library.h"
#include "library/youtube/youtubefeature.h"
#include "mixer/deck.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "track/track.h"
#include "util/logger.h"

namespace {
const mixxx::Logger kLogger("AIBroFeature");

// Timing
constexpr int kProgressIntervalMs = 1000;
constexpr int kBlendTickIntervalMs = 80;

// Blend window — start blend at 90% of track
constexpr double kBlendStartMin = 0.90;

// Base blend duration: 8 seconds (genre rules multiply this)
// Used by kBlendTickIntervalMs to compute total blend steps
constexpr int kBlendSteps = 100;

// Similarity weights
constexpr double kWeightTitleOverlap = 0.10;
constexpr double kWeightSemanticWords = 0.15;
constexpr double kWeightRemixBonus = 0.10;
constexpr double kWeightGenreMatch = 0.10;
constexpr double kWeightDuration = 0.05;
constexpr double kWeightFreshness = 0.05;
constexpr double kWeightBPMProximity = 0.10;
constexpr double kWeightDifferentArtist = 0.15;
constexpr double kWeightGreekContent = 0.15;

// BPM tolerance
constexpr double kBPMToleranceMax = 15.0;

// Delays
constexpr int kLoadToBlendDelayMs = 1000;
constexpr int kRetryDelayMs = 3000;

// Ideal duration (seconds)
constexpr int kIdealDurationMin = 150;
constexpr int kIdealDurationMax = 480;

// Minimum score threshold
constexpr double kMinScoreThreshold = 0.20;

// Search rate limiting
constexpr int kMinSearchIntervalMs = 5000;

// Dedup threshold
constexpr double kDedupThreshold = 0.50;

// Phrase boundary: 8 beats per phrase
constexpr int kPhraseBeats = 8;

// Echo-out: number of beat-synced echoes
constexpr int kEchoBeats = 4;
constexpr double kEchoDecay = 0.4;

// 3-band fade speed multipliers (from ai-remixmate)
// Outgoing: highs vanish first (1.4x), bass lingers (0.65x)
// Incoming: bass arrives first (1.4x), highs bloom last (0.65x)
constexpr double kBandFadeHighOut = 1.4;
constexpr double kBandFadeMidOut = 1.0;
constexpr double kBandFadeLowOut = 0.65;
constexpr double kBandFadeHighIn = 0.65;
constexpr double kBandFadeMidIn = 1.0;
constexpr double kBandFadeLowIn = 1.4;

// Garbage title patterns
static const QStringList& garbagePatterns() {
    static const QStringList patterns = []() {
        QStringList list;
        list << "\U0001f602" << "\U0001f923" << "\U0001f480" << "\U0001f525"
             << "\U0001f62d" << "\U0001f97a" << "\U0001f4af" << "\U0001f44f"
             << "\U0001f64f" << "\U0001f495";
        list << "credit goes to" << "not my" << "fan edit" << "fan made"
             << "meme";
        list << "reaction" << "react to" << "vs battle" << "tier list"
             << "ranking";
        list << "top 10" << "top 20" << "top 50" << "top 100"
             << "compilation";
        list << "playlist" << "full album" << "album stream" << "anime"
             << "amv";
        list << "fancam" << "lyrics video" << "lyric video";
        list << "behind the scenes" << "making of" << "interview";
        list << "live performance" << "unplugged session";
        return list;
    }();
    return patterns;
}

// Greek music genre keywords for discovery
static const QStringList& greekGenreKeywords() {
    static const QStringList* k = []() {
        auto* list = new QStringList();
        static const char* words[] = {"ελληνικά",
                "ελληνικη",
                "ελληνικο",
                "greek",
                "greece",
                "ελλάδα",
                "δημοτικά",
                "δημοτικο",
                "λαϊκά",
                "λαϊκο",
                "ρεμπέτικο",
                "ρεμπετικο",
                "μπουζούκια",
                "μπουζουκια",
                "νέα",
                "νεα",
                "κομμάτι",
                "κομματι",
                "τραγούδι",
                "τραγουδι",
                "remix ελληνικό",
                "remix ελληνικο",
                "greek remix",
                "ελληνικό remix",
                "ελληνικο remix",
                "greek music",
                "ελληνική μουσική",
                "ελληνικη μουσικη"};
        for (const char* w : words) {
            *list << QString::fromUtf8(w);
        }
        return list;
    }();
    return *k;
}

// Stop words for semantic scoring
static const QSet<QString>& stopWords() {
    static const QSet<QString>* s = []() {
        auto* set = new QSet<QString>();
        static const char* words[] = {"the",
                "a",
                "an",
                "and",
                "or",
                "but",
                "in",
                "on",
                "at",
                "to",
                "for",
                "of",
                "with",
                "by",
                "from",
                "is",
                "it",
                "this",
                "that",
                "i",
                "you",
                "he",
                "she",
                "we",
                "they",
                "my",
                "your",
                "his",
                "her",
                "our",
                "their",
                "me",
                "him",
                "us",
                "them",
                "be",
                "was",
                "are",
                "been",
                "being",
                "have",
                "has",
                "had",
                "do",
                "does",
                "did",
                "will",
                "would",
                "could",
                "should",
                "may",
                "might",
                "can",
                "shall",
                "not",
                "no",
                "nor",
                "as",
                "if",
                "then",
                "than",
                "too",
                "very",
                "just",
                "about",
                "above",
                "after",
                "again",
                "all",
                "also",
                "am",
                "any",
                "because",
                "before",
                "between",
                "both",
                "each",
                "few",
                "get",
                "got",
                "how",
                "its",
                "let",
                "make",
                "more",
                "most",
                "much",
                "must",
                "new",
                "now",
                "only",
                "other",
                "our",
                "out",
                "own",
                "same",
                "so",
                "some",
                "still",
                "such",
                "take",
                "tell",
                "through",
                "under",
                "up",
                "use",
                "want",
                "way",
                "well",
                "what",
                "when",
                "where",
                "which",
                "while",
                "who",
                "why",
                "feat",
                "ft",
                "vs",
                "remix",
                "edit",
                "mix",
                "version",
                "official",
                "audio",
                "video",
                "lyrics",
                "lyric",
                "explicit",
                "clean",
                "radio",
                "album",
                "single",
                "cover",
                "live",
                "acoustic",
                "unplugged",
                "rework"};
        for (const char* w : words) {
            *set << QString::fromLatin1(w);
        }
        return set;
    }();
    return *s;
}

// Normalize a song title for fuzzy comparison
static QString normalizeSongTitle(const QString& title) {
    QString t = title.toLower().trimmed();
    static const QRegularExpression parenRe(
            QStringLiteral("\\s*\\([^)]*\\)"),
            QRegularExpression::CaseInsensitiveOption);
    t.remove(parenRe);
    static const QRegularExpression dashRe(
            QStringLiteral("\\s*[-–—]\\s*(remix|edit|mix|version|cover|live|"
                           "acoustic|instrumental|karaoke|lyrics?).*$"),
            QRegularExpression::CaseInsensitiveOption);
    t.remove(dashRe);
    static const QStringList keywords = {"remix",
            "edit",
            "mix",
            "version",
            "cover",
            "live",
            "acoustic",
            "instrumental",
            "karaoke",
            "lyrics",
            "lyric",
            "official",
            "audio",
            "video",
            "hd",
            "4k",
            "extended",
            "club",
            "radio",
            "original"};
    for (const QString& kw : keywords) {
        t.remove(kw);
    }
    t = t.trimmed();
    static const QRegularExpression spaceRe(QStringLiteral("\\s+"));
    t.replace(spaceRe, " ");
    return t;
}

// Word-overlap similarity (Jaccard-like)
static double titleSimilarity(const QString& a, const QString& b) {
    if (a.isEmpty() || b.isEmpty()) {
        return 0.0;
    }
    if (a == b) {
        return 1.0;
    }
    QStringList wordsA = a.split(' ');
    QStringList wordsB = b.split(' ');
    if (wordsA.isEmpty() || wordsB.isEmpty()) {
        return 0.0;
    }
    int matches = 0;
    for (const QString& w : std::as_const(wordsA)) {
        if (wordsB.contains(w)) {
            ++matches;
        }
    }
    return static_cast<double>(matches) /
            qMax(wordsA.size(), wordsB.size());
}

// Remix/extended keywords
static const QStringList& remixKeywords() {
    static const QStringList* k = []() {
        auto* list = new QStringList();
        static const char* words[] = {"remix",
                "extended",
                "mix",
                "edit",
                "version",
                "dub",
                "instrumental",
                "a cappella",
                "bootleg",
                "mashup",
                "flip",
                "rework",
                "VIP",
                "radio edit",
                "club mix",
                "extended mix",
                "original mix"};
        for (const char* w : words) {
            *list << QString::fromLatin1(w);
        }
        return list;
    }();
    return *k;
}

// Cosine ease-in-out: smooth S-curve for crossfade
static double easeInOut(double t) {
    return t * t * (3.0 - 2.0 * t);
}

// Cosine fade-out: 1.0 → 0.0
static double fadeOut(double t) {
    return 0.5 * (1.0 + std::cos(M_PI * t));
}

// Cosine fade-in: 0.0 → 1.0
static double fadeIn(double t) {
    return 0.5 * (1.0 - std::cos(M_PI * t));
}

} // namespace

// ===================================================================
// Construction / Destruction
// ===================================================================

AIBroFeature::AIBroFeature(Library* pLibrary,
        UserSettingsPointer pConfig,
        PlayerManager* pPlayerManager,
        YouTubeFeature* pYouTubeFeature)
        : m_pProgressTimer(new QTimer(this)),
          m_pBlendTimer(new QTimer(this)),
          m_keyEnabled("[AIBro]", "enabled"),
          m_controlEnabled(m_keyEnabled),
          m_coCrossfader(ConfigKey("[Master]", "crossfader")),
          m_downloading(false),
          m_blending(false),
          m_blendStep(0),
          m_blendFromDeck(-1),
          m_blendToDeck(-1),
          m_iCurrentDeck(0),
          m_lastSearchTimeMs(0),
          m_manualTrackPath(),
          m_manualTrackDeck(-1),
          m_hasManualTrack(false),
          m_blendCount(0),
          m_sessionBPM(0.0),
          m_pLibrary(pLibrary),
          m_pConfig(pConfig),
          m_pPlayerManager(pPlayerManager),
          m_pYouTubeFeature(pYouTubeFeature),
          m_pMusicMatcher(new mixxx::MusicMatcherClient(this)) {
}

AIBroFeature::~AIBroFeature() = default;

// ===================================================================
// Init
// ===================================================================

void AIBroFeature::init() {
    m_controlEnabled.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_controlEnabled.connectValueChangeRequest(
            this, &AIBroFeature::slotToggle);
    connect(m_pProgressTimer, &QTimer::timeout, this, &AIBroFeature::slotProgressTick);
    m_pProgressTimer->setInterval(kProgressIntervalMs);
    connect(m_pBlendTimer, &QTimer::timeout, this, &AIBroFeature::slotBlendTick);
    m_pBlendTimer->setInterval(kBlendTickIntervalMs);

    // YouTube service signals
    if (m_pYouTubeFeature) {
        mixxx::YouTubeService* pService = m_pYouTubeFeature->service();
        if (pService) {
            bool ok1 = connect(pService,
                    &mixxx::YouTubeService::downloadFinished,
                    this,
                    &AIBroFeature::slotDownloadFinished);
            bool ok2 = connect(pService,
                    &mixxx::YouTubeService::downloadFailed,
                    this,
                    &AIBroFeature::slotDownloadFailed);
            bool ok3 = connect(pService,
                    &mixxx::YouTubeService::searchResultsReady,
                    this,
                    &AIBroFeature::slotSearchResultsReady);
            bool ok4 = connect(pService,
                    &mixxx::YouTubeService::searchFailed,
                    this,
                    &AIBroFeature::slotSearchFailed);
            kLogger.info()
                    << "AI Bro: signal connections: downloadFinished=" << ok1
                    << " downloadFailed=" << ok2
                    << " searchResultsReady=" << ok3
                    << " searchFailed=" << ok4;
        } else {
            kLogger.warning() << "AI Bro: YouTubeService is null!";
        }
    } else {
        kLogger.warning() << "AI Bro: YouTubeFeature is null!";
    }

    if (m_pMusicMatcher) {
        bool m1 = connect(m_pMusicMatcher,
                &mixxx::MusicMatcherClient::suggestionsReady,
                this,
                &AIBroFeature::slotMusicMatcherSuggestionsReady);
        bool m2 = connect(m_pMusicMatcher,
                &mixxx::MusicMatcherClient::searchFailed,
                this,
                &AIBroFeature::slotMusicMatcherSearchFailed);
        kLogger.info() << "AI Bro: MusicMatcher connections: suggestionsReady=" << m1
                       << " searchFailed=" << m2;
    }
}

bool AIBroFeature::isActive() const {
    return m_controlEnabled.toBool();
}

// ===================================================================
// Toggle
// ===================================================================

void AIBroFeature::slotToggle(bool newValue) {
    m_controlEnabled.setAndConfirm(newValue);
    if (newValue) {
        kLogger.info() << "AI Bro: activated";
        m_downloading = false;
        m_blending = false;
        m_blendStep = 0;
        m_blendCount = 0;
        m_playedVideoIds.clear();
        m_playedSongKeys.clear();
        m_playedArtists.clear();
        m_sessionBPM = 0.0;
        m_pProgressTimer->start();
        findNextSong();
    } else {
        kLogger.info() << "AI Bro: deactivated";
        m_pProgressTimer->stop();
        m_pBlendTimer->stop();
        m_blending = false;
        m_downloading = false;
        m_playedVideoIds.clear();
        m_playedSongKeys.clear();
        m_playedArtists.clear();
    }
}

// ===================================================================
// Genre-aware mixing rules (from AI-DJ-Mixing-System)
// ===================================================================

AIBroFeature::MixingRules AIBroFeature::getMixingRules() const {
    // Detect genre from current track title/artist
    QString context = (m_currentTrackTitle + " " + m_currentTrackArtist).toLower();

    // Check for Greek/laiko/rebetiko
    for (const QString& kw : greekGenreKeywords()) {
        if (context.contains(kw)) {
            // Greek music: moderate overlap (1.0x), standard EQ
            return {1.0, 1.0, kEchoDecay, kEchoBeats};
        }
    }

    // EDM/electronic
    if (context.contains("edm") || context.contains("electronic") ||
            context.contains("techno") || context.contains("trance") ||
            context.contains("house")) {
        return {2.0, 1.5, 0.35, 6}; // Long blends, strong EQ
    }

    // Hip-hop/rap
    if (context.contains("hip hop") || context.contains("hip-hop") ||
            context.contains("rap") || context.contains("trap")) {
        return {0.5, 0.8, 0.5, 2}; // Quick cuts, light EQ
    }

    // Pop
    if (context.contains("pop")) {
        return {1.0, 1.0, kEchoDecay, kEchoBeats};
    }

    // Rock
    if (context.contains("rock")) {
        return {0.75, 0.9, 0.45, 3};
    }

    // Default (Greek music falls here too)
    return {1.0, 1.0, kEchoDecay, kEchoBeats};
}

// ===================================================================
// 3-band frequency-selective fade (from ai-remixmate)
// ===================================================================

AIBroFeature::BandFades AIBroFeature::computeBandFades(
        double progress, bool outgoing) const {
    BandFades fades;
    if (outgoing) {
        // Exiting: highs vanish first, bass lingers
        double tHigh = std::min(1.0, progress * kBandFadeHighOut);
        double tMid = std::min(1.0, progress * kBandFadeMidOut);
        double tLow = std::min(1.0, progress * kBandFadeLowOut);
        fades.high = 0.5 * (1.0 + std::cos(M_PI * tHigh));
        fades.mid = 0.5 * (1.0 + std::cos(M_PI * tMid));
        fades.low = 0.5 * (1.0 + std::cos(M_PI * tLow));
    } else {
        // Entering: bass arrives first, highs bloom last
        double tHigh = std::min(1.0, progress * kBandFadeHighIn);
        double tMid = std::min(1.0, progress * kBandFadeMidIn);
        double tLow = std::min(1.0, progress * kBandFadeLowIn);
        fades.high = 0.5 * (1.0 - std::cos(M_PI * tHigh));
        fades.mid = 0.5 * (1.0 - std::cos(M_PI * tMid));
        fades.low = 0.5 * (1.0 - std::cos(M_PI * tLow));
    }
    return fades;
}

// ===================================================================
// Stem-aware crossfade curves (from ai-remixmate)
// ===================================================================

AIBroFeature::CrossfadeCurve AIBroFeature::computeCrossfadeCurve(
        double progress) const {
    CrossfadeCurve curve;

    // Estimate track similarity from BPM proximity
    double currentBPM = getCurrentPlayingBPM();
    double similarity = 0.5; // Default: medium similarity

    if (currentBPM > 0 && m_sessionBPM > 0) {
        double bpmRatio = m_sessionBPM / currentBPM;
        double bpmDiff = std::abs(bpmRatio - 1.0);
        // Map BPM difference to similarity: 0% diff = 1.0, >15% = 0.0
        similarity = qBound(0.0, 1.0 - (bpmDiff / 0.15), 1.0);
    }

    if (similarity >= 0.75) {
        // Extended blend: A holds until 40%, fades by 90%
        //                 B enters from 10%, full by 60%
        if (progress < 0.40) {
            curve.fadeOut = 1.0;
        } else if (progress < 0.90) {
            curve.fadeOut = 1.0 - (progress - 0.40) / 0.50;
        } else {
            curve.fadeOut = 0.0;
        }
        if (progress < 0.10) {
            curve.fadeIn = 0.0;
        } else if (progress < 0.60) {
            curve.fadeIn = (progress - 0.10) / 0.50;
        } else {
            curve.fadeIn = 1.0;
        }
    } else if (similarity >= 0.40) {
        // Standard cosine crossfade
        curve.fadeOut = fadeOut(progress);
        curve.fadeIn = fadeIn(progress);
    } else {
        // Sharp handoff: A exits by 50%, B enters after 50%
        if (progress < 0.50) {
            curve.fadeOut = 1.0 - progress / 0.50;
            curve.fadeIn = 0.0;
        } else {
            curve.fadeOut = 0.0;
            curve.fadeIn = (progress - 0.50) / 0.50;
        }
    }

    return curve;
}

// ===================================================================
// Energy arc (from ai-remixmate setlist_planner)
// ===================================================================

double AIBroFeature::computeEnergyArc() const {
    // Mountain arc: peak at 70% of session
    // We use blendCount as a proxy for session progress
    // Assume a "session" is ~20 blends
    double t = qBound(0.0, m_blendCount / 20.0, 1.0);
    double peak = 0.70;
    double energy;
    if (t <= peak) {
        energy = 0.2 + 0.8 * (t / peak);
    } else {
        energy = 1.0 - 0.6 * ((t - peak) / (1.0 - peak));
    }
    // Map energy to blend intensity: 0.5 - 1.5x
    return 0.5 + energy;
}

// ===================================================================
// BPM correction (from AI-DJ-Mixing-System)
// ===================================================================

double AIBroFeature::correctBPM(double detectedBPM, double expectedBPM) const {
    if (expectedBPM <= 0) {
        return detectedBPM;
    }
    // Common librosa detection error ratios
    static const double ratios[] = {1.0, 2.0, 0.5, 1.5, 0.75, 1.333, 0.666};
    double bestRatio = 1.0;
    double bestDiff = std::abs(detectedBPM - expectedBPM);
    for (double ratio : ratios) {
        double corrected = detectedBPM / ratio;
        double diff = std::abs(corrected - expectedBPM);
        if (diff < bestDiff) {
            bestDiff = diff;
            bestRatio = ratio;
        }
    }
    double result = detectedBPM / bestRatio;
    // If still >10% off, trust expected
    if (std::abs(result - expectedBPM) / expectedBPM > 0.10) {
        return expectedBPM;
    }
    return result;
}

// ===================================================================
// Phrase-boundary alignment
// ===================================================================

double AIBroFeature::alignToPhraseBoundary(double positionSec) const {
    double bpm = getCurrentPlayingBPM();
    if (bpm <= 0) {
        bpm = 120.0;
    }
    double beatInterval = 60.0 / bpm;
    double phraseInterval = beatInterval * kPhraseBeats;
    // Round up to next phrase boundary
    double phrases = std::ceil(positionSec / phraseInterval);
    return phrases * phraseInterval;
}

// ===================================================================
// Song discovery
// ===================================================================

QString AIBroFeature::buildSearchQuery() {
    if (m_currentTrackTitle.isEmpty()) {
        return {};
    }
    if (m_currentTrackArtist.isEmpty()) {
        return m_currentTrackTitle;
    }
    return QStringLiteral("%1 %2").arg(m_currentTrackArtist, m_currentTrackTitle);
}

QStringList AIBroFeature::buildDiscoveryQueries() {
    QStringList queries;
    const QString title = m_currentTrackTitle;
    const QString artist = m_currentTrackArtist;
    if (title.isEmpty() || title.length() < 3) {
        return queries;
    }
    const QString currentLower = title.toLower();
    for (const QString& pattern : garbagePatterns()) {
        if (currentLower.contains(pattern)) {
            return queries;
        }
    }
    if (!artist.isEmpty()) {
        queries << QStringLiteral("%1 remix ελληνικά").arg(artist);
        queries << QStringLiteral("ελληνικά remix 2024 2025");
    } else {
        queries << title;
        queries << QStringLiteral("%1 ελληνικά").arg(title);
        queries << QStringLiteral("%1 remix").arg(title);
        queries << QStringLiteral("ελληνικά remix");
    }
    return queries;
}

double AIBroFeature::scoreCandidate(
        const mixxx::YouTubeVideoInfo& candidate) {
    if (candidate.title.isEmpty()) {
        return -1.0;
    }

    // Hard filter: already played
    if (m_playedVideoIds.contains(candidate.id)) {
        return -1.0;
    }

    // Hard filter: similar title already played
    QString normTitle = normalizeSongTitle(candidate.title);
    for (const QString& playedKey : std::as_const(m_playedSongKeys)) {
        if (titleSimilarity(normTitle, normalizeSongTitle(playedKey)) > kDedupThreshold) {
            return -1.0;
        }
    }

    const bool trendingMode = m_currentTrackTitle.isEmpty() ||
            m_currentTrackTitle.length() < 3;

    // Hard filter: same song as currently playing
    if (!trendingMode) {
        const QString candLower = candidate.title.toLower().trimmed();
        const QString currLower = m_currentTrackTitle.toLower().trimmed();
        if (candLower == currLower) {
            return -1.0;
        }
        if (candLower.contains(currLower) || currLower.contains(candLower)) {
            if (candLower.length() < currLower.length() + 15) {
                return -1.0;
            }
        }
    }

    // Hard filter: reject songs by the SAME artist (we want variety)
    // Compare uploader name with current track artist
    if (!m_currentTrackArtist.isEmpty() && !candidate.uploader.isEmpty()) {
        const QString currArtLower = m_currentTrackArtist.toLower().trimmed();
        const QString uploaderLower = candidate.uploader.toLower().trimmed();
        // Check if uploader matches artist (handles "Artist", "Artist - Topic", etc.)
        if (uploaderLower.contains(currArtLower) || currArtLower.contains(uploaderLower)) {
            return -1.0;
        }
    }

    // Hard filter: live streams
    if (candidate.isLive) {
        return -1.0;
    }

    // Hard filter: compilations, playlists, "best of", mixes, collections
    {
        const QString lowerTitle = candidate.title.toLower();
        if (lowerTitle.contains("compilation") ||
                lowerTitle.contains("best of") ||
                lowerTitle.contains("greatest hits") ||
                lowerTitle.contains("top hits") ||
                lowerTitle.contains("playlist") ||
                lowerTitle.contains("full album") ||
                lowerTitle.contains("album mix") ||
                lowerTitle.contains("vol.") ||
                lowerTitle.contains("volume ")) {
            return -1.0;
        }
    }

    // Hard filter: remixes (often low quality)
    {
        const QString lowerTitle = candidate.title.toLower();
        if (lowerTitle.contains("remix") ||
                lowerTitle.contains("edit") ||
                lowerTitle.contains("bootleg") ||
                lowerTitle.contains("mashup") ||
                lowerTitle.contains("rework")) {
            return -1.0;
        }
    }

    // Hard filter: garbage titles
    {
        const QString lowerTitle = candidate.title.toLower();
        for (const QString& pattern : garbagePatterns()) {
            if (lowerTitle.contains(pattern)) {
                return -1.0;
            }
        }
        if (candidate.title.length() < 3 || candidate.title.length() > 100) {
            return -1.0;
        }
        int specialCharCount = 0;
        for (const QChar& c : candidate.title) {
            if (c.unicode() > 127 && !c.isLetterOrNumber()) {
                specialCharCount++;
            }
        }
        if (specialCharCount > 3) {
            return -1.0;
        }
    }

    // Hard filter: label/compilation uploaders (e.g. "QualityControlVEVO", "Various Artists")
    {
        const QString uploaderLower = candidate.uploader.toLower();
        if (uploaderLower.contains("quality control") ||
                uploaderLower.contains("qualitycontrol") ||
                uploaderLower.contains("various artists") ||
                (uploaderLower.contains("various") && candidate.title.contains("-"))) {
            return -1.0;
        }
    }

    // Hard filter: current track is garbage
    if (!m_currentTrackTitle.isEmpty() && m_currentTrackTitle.length() >= 3) {
        const QString currentLower = m_currentTrackTitle.toLower();
        for (const QString& pattern : garbagePatterns()) {
            if (currentLower.contains(pattern)) {
                return -1.0;
            }
        }
    }

    double score = 0.0;
    const QString currentT = m_currentTrackTitle.toLower().trimmed();
    const QString currentA = m_currentTrackArtist.toLower().trimmed();
    const QString videoT = candidate.title.toLower().trimmed();
    const QString videoU = candidate.uploader.toLower().trimmed();

    // 1. Title word overlap
    const QStringList titleList = currentT.split(' ', Qt::SkipEmptyParts);
    const QSet<QString> titleWords(titleList.begin(), titleList.end());
    const QStringList videoList = videoT.split(' ', Qt::SkipEmptyParts);
    const QSet<QString> videoWords(videoList.begin(), videoList.end());
    if (!titleWords.isEmpty() && !videoWords.isEmpty()) {
        int common = 0;
        for (const QString& w : titleWords) {
            if (videoWords.contains(w)) {
                ++common;
            }
        }
        double jaccard = static_cast<double>(common) /
                qMax(titleWords.size(), videoWords.size());
        score += kWeightTitleOverlap * jaccard;
    }

    // 2. Semantic word similarity
    if (!titleWords.isEmpty() && !videoWords.isEmpty()) {
        double semanticScore = 0.0;
        int meaningfulWords = 0;
        for (const QString& w : titleWords) {
            if (stopWords().contains(w)) {
                continue;
            }
            ++meaningfulWords;
            if (videoWords.contains(w)) {
                semanticScore += 1.0;
            } else {
                for (const QString& vw : videoWords) {
                    if (vw.contains(w) || w.contains(vw)) {
                        semanticScore += 0.5;
                        break;
                    }
                }
            }
        }
        if (meaningfulWords > 0) {
            semanticScore /= meaningfulWords;
        }
        score += kWeightSemanticWords * semanticScore;
    }

    // 3. Different artist bonus (same artist songs are already filtered out above)
    if (!currentA.isEmpty() && !videoU.isEmpty()) {
        score += kWeightDifferentArtist;
    }

    // 4. Remix/extended bonus
    for (const QString& kw : remixKeywords()) {
        if (videoT.contains(kw)) {
            score += kWeightRemixBonus;
            break;
        }
    }

    // 5. Greek content bonus
    for (const QString& gk : greekGenreKeywords()) {
        if (videoT.contains(gk) || videoU.contains(gk)) {
            score += kWeightGreekContent;
            break;
        }
    }

    // 6. Genre/vibe heuristics
    static const QStringList kEnergyWords = {"remix",
            "mix",
            "edit",
            "extended",
            "club",
            "festival",
            "live",
            "session",
            "bootleg",
            "mashup",
            "flip",
            "rework"};
    int energyMatches = 0;
    for (const QString& ew : kEnergyWords) {
        if (videoT.contains(ew)) {
            ++energyMatches;
        }
    }
    if (energyMatches > 0) {
        score += kWeightGenreMatch * qMin(energyMatches, 3) / 3.0;
    }

    // 7. Duration quality
    if (candidate.durationSec > 0) {
        if (candidate.durationSec >= kIdealDurationMin &&
                candidate.durationSec <= kIdealDurationMax) {
            score += kWeightDuration;
        } else if (candidate.durationSec < 90 || candidate.durationSec > 600) {
            score -= 0.1;
        }
    }

    // 8. Freshness
    if (!videoU.isEmpty()) {
        if (videoU.contains("vevo") || videoU.contains("official")) {
            score += kWeightFreshness;
        }
    }

    // 9. BPM proximity
    double currentBPM = getCurrentPlayingBPM();
    double candidateBPM = getCandidateBPM(candidate);
    if (currentBPM > 0 && candidateBPM > 0) {
        double bpmRatio = candidateBPM / currentBPM;
        double bpmDiff = std::abs(bpmRatio - 1.0);
        double halfDiff = std::abs(bpmRatio - 0.5);
        double doubleDiff = std::abs(bpmRatio - 2.0);
        double bestDiff = std::min({bpmDiff, halfDiff, doubleDiff});
        double bpmScore = 1.0 - (bestDiff / (kBPMToleranceMax / 100.0));
        score += kWeightBPMProximity * qBound(0.0, bpmScore, 1.0);
    }

    // Trending mode base score
    if (trendingMode && score < kMinScoreThreshold) {
        score = kMinScoreThreshold + 0.01;
    }

    return qBound(0.0, score, 1.0);
}

mixxx::YouTubeVideoInfo AIBroFeature::pickBestCandidate(
        const QList<mixxx::YouTubeVideoInfo>& results) {
    double bestScore = -1;
    int bestIdx = -1;
    for (int i = 0; i < results.size(); ++i) {
        double s = scoreCandidate(results[i]);
        if (s > bestScore) {
            bestScore = s;
            bestIdx = i;
        }
    }
    if (bestIdx < 0 || bestScore < kMinScoreThreshold) {
        kLogger.warning()
                << "AI Bro: no suitable candidate (best:" << bestScore
                << "threshold:" << kMinScoreThreshold << ")";
        return {};
    }
    return results[bestIdx];
}

void AIBroFeature::downloadCandidate(
        const mixxx::YouTubeVideoInfo& candidate) {
    if (!m_pYouTubeFeature) {
        return;
    }
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastSearchTimeMs < kMinSearchIntervalMs) {
        kLogger.info() << "AI Bro: rate limited, skipping";
        return;
    }
    m_lastSearchTimeMs = now;
    kLogger.info() << "AI Bro: downloading [" << candidate.id << "] "
                   << candidate.title << " by " << candidate.uploader
                   << " (session:" << m_playedVideoIds.size() << " played)";
    m_downloadingTitle = candidate.title;
    m_downloadingUploader = candidate.uploader;
    m_pYouTubeFeature->requestDownload(candidate.id);
}

// ===================================================================
// Song finding orchestration
// ===================================================================

void AIBroFeature::findNextSong() {
    if (!isActive() || m_downloading) {
        return;
    }

    updateCurrentTrackInfo();

    // Validate current track
    const QString currentLower = m_currentTrackTitle.toLower();
    bool currentIsGarbage = m_currentTrackTitle.isEmpty() ||
            m_currentTrackTitle.length() < 3;
    if (!currentIsGarbage) {
        for (const QString& pattern : garbagePatterns()) {
            if (currentLower.contains(pattern)) {
                currentIsGarbage = true;
                break;
            }
        }
    }

    if (currentIsGarbage) {
        kLogger.info() << "AI Bro: track empty/garbage, fetching trending";
        m_downloading = true;
        if (m_pYouTubeFeature) {
            m_pYouTubeFeature->searchAndActivate(
                    QStringLiteral("popular greek music 2024 2025"));
        }
        return;
    }

    const QString& artist = m_currentTrackArtist;
    const QString& title = m_currentTrackTitle;

    m_downloading = true;
    m_searchTrackSnapshot = snapshotTrackLocations();

    if (m_pMusicMatcher) {
        kLogger.info() << "AI Bro: Querying Deezer MusicMatcher for similar recommendations of:" << artist << "-" << title;
        m_pMusicMatcher->findSimilar(artist, title);
    } else {
        slotMusicMatcherSearchFailed(QStringLiteral("MusicMatcherClient unavailable"));
    }
}

// ===================================================================
// Progress monitoring
// ===================================================================

void AIBroFeature::slotProgressTick() {
    if (!isActive() || m_blending || !m_pPlayerManager) {
        return;
    }
    if (isDeckPlaying(m_iCurrentDeck)) {
        double pos = getDeckPlayPosition(m_iCurrentDeck);
        if (pos >= kBlendStartMin && !m_downloading) {
            kLogger.info() << "AI Bro: blend point at" << pos
                           << "on deck" << (m_iCurrentDeck + 1);
            updateCurrentTrackInfo();
            findNextSong();
        }
    }
}

// ===================================================================
// Search results
// ===================================================================

void AIBroFeature::slotSearchResultsReady(
        const QString& query,
        const QList<mixxx::YouTubeVideoInfo>& results) {
    Q_UNUSED(query);
    if (!m_downloading || results.isEmpty()) {
        return;
    }

    kLogger.info() << "AI Bro:" << results.size() << "results";

    // Manual track override
    QString manualPath = findNewManualTrack();
    if (!manualPath.isEmpty()) {
        kLogger.info() << "AI Bro: manual track detected, using that";
        m_downloading = false;
        loadAndBlend(manualPath);
        return;
    }

    auto candidate = pickBestCandidate(results);
    if (candidate.id.isEmpty()) {
        kLogger.warning() << "AI Bro: no suitable candidate, retrying with broader query";
        m_downloading = false;
        // Retry with a broader genre search instead of the same query
        QTimer::singleShot(kRetryDelayMs, this, [this]() {
            if (!isActive()) {
                return;
            }
            // Fallback: search for genre terms to get variety (no remixes)
            QString fallbackQuery = QStringLiteral("ελληνικά 2024 2025");
            kLogger.info() << "AI Bro: fallback search:" << fallbackQuery;
            m_downloading = true;
            if (m_pYouTubeFeature) {
                m_pYouTubeFeature->searchAndActivate(fallbackQuery);
            }
        });
        return;
    }
    double score = scoreCandidate(candidate);
    kLogger.info() << "AI Bro: selected [" << candidate.id << "]"
                   << candidate.title << "score:" << score;
    downloadCandidate(candidate);
}

// ===================================================================
// Download handlers
// ===================================================================

void AIBroFeature::slotDownloadFinished(
        const QString& videoId, const QString& localPath) {
    Q_UNUSED(videoId);
    kLogger.info() << "AI Bro: download ready:" << localPath;

    m_playedVideoIds.insert(videoId);
    if (!m_downloadingTitle.isEmpty()) {
        m_playedSongKeys.insert(m_downloadingTitle.toLower().trimmed());
        m_playedSongKeys.insert(normalizeSongTitle(m_downloadingTitle));
    }
    if (!m_downloadingUploader.isEmpty()) {
        m_playedArtists.insert(m_downloadingUploader.toLower().trimmed());
    }
    m_downloadingTitle.clear();
    m_downloadingUploader.clear();
    m_downloading = false;

    if (!isActive()) {
        kLogger.info() << "AI Bro: download finished but deactivated";
        return;
    }

    loadAndBlend(localPath);
}

void AIBroFeature::slotDownloadFailed(
        const QString& videoId, const QString& error) {
    Q_UNUSED(videoId);
    kLogger.warning() << "AI Bro: download failed:" << error;
    m_downloading = false;
    m_downloadingTitle.clear();
    m_downloadingUploader.clear();

    // Clear garbage track
    const QString currentLower = m_currentTrackTitle.toLower();
    for (const QString& pattern : garbagePatterns()) {
        if (currentLower.contains(pattern)) {
            m_currentTrackTitle.clear();
            m_currentTrackArtist.clear();
            break;
        }
    }

    QTimer::singleShot(kRetryDelayMs * 2, this, [this]() {
        if (isActive()) {
            findNextSong();
        }
    });
}

void AIBroFeature::slotSearchFailed(
        const QString& query, const QString& error) {
    if (!m_downloading) {
        return;
    }
    Q_UNUSED(query);
    kLogger.warning() << "AI Bro: search failed:" << error;
    m_downloading = false;
    QTimer::singleShot(kRetryDelayMs, this, [this]() {
        if (isActive()) {
            findNextSong();
        }
    });
}

// ===================================================================
// DJ Blending — professional techniques
// ===================================================================

void AIBroFeature::loadAndBlend(const QString& localPath) {
    if (!m_pPlayerManager || m_blending) {
        return;
    }

    int toDeck = findAvailableDeck();
    if (toDeck < 0) {
        kLogger.warning() << "AI Bro: no available deck";
        m_downloading = false;
        return;
    }

    int fromDeck = m_iCurrentDeck;
    if (fromDeck < 0) {
        kLogger.info() << "AI Bro: no source, loading to deck" << (toDeck + 1);
        m_pPlayerManager->slotLoadToDeck(localPath, toDeck + 1);
        m_downloading = false;
        return;
    }

    kLogger.info() << "AI Bro: loading to deck" << (toDeck + 1)
                   << "blending from deck" << (fromDeck + 1);

    m_pPlayerManager->slotLoadToDeck(localPath, toDeck + 1);

    m_blendFromDeck = fromDeck;
    m_blendToDeck = toDeck;
    QTimer::singleShot(kLoadToBlendDelayMs, this, [this]() {
        if (!isActive()) {
            m_downloading = false;
            return;
        }
        // Vocal sync: seek incoming track to estimated vocal start
        double vocalStart = estimateVocalStartPosition(m_blendToDeck);
        if (vocalStart > 0.0) {
            ConfigKey posKey(
                    QStringLiteral("[Channel%1]").arg(m_blendToDeck + 1),
                    "playposition");
            ControlObject::set(posKey, vocalStart);
            kLogger.info() << "AI Bro: vocal sync — seeked to"
                           << (vocalStart * 100.0) << "%";
        }
        startBlend(m_blendFromDeck, m_blendToDeck);
    });
}

void AIBroFeature::startBlend(int fromDeck, int toDeck) {
    kLogger.info() << "AI Bro: starting blend from deck" << (fromDeck + 1)
                   << "to deck" << (toDeck + 1);

    m_blending = true;
    m_blendStep = 0;
    m_blendFromDeck = fromDeck;
    m_blendToDeck = toDeck;

    // Get genre-specific mixing rules
    MixingRules rules = getMixingRules();
    kLogger.info() << "AI Bro: mixing rules — overlap:" << rules.overlapMultiplier
                   << "x, EQ:" << rules.eqStrength;

    // Store session BPM for crossfade curve computation
    m_sessionBPM = getCurrentPlayingBPM();
    if (m_sessionBPM <= 0) {
        m_sessionBPM = 120.0;
    }

    // NO BPM sync — pure crossfade blending
    // Set rate range for potential manual tempo matching
    {
        ConfigKey rangeKey(
                QStringLiteral("[Channel%1]").arg(toDeck + 1), "rateRange");
        ControlObject::set(rangeKey, 0.5);
    }

    // Start target deck playing
    setPlay(toDeck, true);

    // Initial EQ on target — cut bass and highs (prevent frequency clash)
    setEQ(toDeck, 0.0, 0.5, 0.2);

    // Start with target volume slightly lower
    setVolume(toDeck, 0.85);

    // Start blend animation
    m_pBlendTimer->start();
}

void AIBroFeature::slotBlendTick() {
    if (!isActive() || !m_blending) {
        m_pBlendTimer->stop();
        return;
    }

    ++m_blendStep;
    double progress = static_cast<double>(m_blendStep) / kBlendSteps;

    if (progress >= 1.0) {
        stopBlend();
        return;
    }

    // Smooth ease-in-out curve
    double eased = easeInOut(progress);

    // --- Crossfader ---
    double crossfadeValue = 0.0;
    if (m_blendFromDeck < m_blendToDeck) {
        crossfadeValue = -1.0 + 2.0 * eased;
    } else {
        crossfadeValue = 1.0 - 2.0 * eased;
    }
    m_coCrossfader.set(crossfadeValue);

    // --- 3-Band Frequency-Selective Fade (ai-remixmate technique) ---
    // Outgoing: highs vanish first (1.4x), bass lingers (0.65x)
    // Incoming: bass arrives first (1.4x), highs bloom last (0.65x)
    BandFades sourceFades = computeBandFades(progress, true);
    BandFades targetFades = computeBandFades(progress, false);

    // Apply EQ with genre-specific strength
    MixingRules rules = getMixingRules();
    double eqStr = rules.eqStrength;

    // Source deck (outgoing): per-band fade
    setEQ(m_blendFromDeck,
            sourceFades.low * eqStr,
            sourceFades.mid * eqStr,
            sourceFades.high * eqStr);

    // Target deck (incoming): per-band fade
    setEQ(m_blendToDeck,
            targetFades.low * eqStr,
            targetFades.mid * eqStr,
            targetFades.high * eqStr);

    // --- Stem-aware crossfade curves ---
    CrossfadeCurve curve = computeCrossfadeCurve(progress);

    // --- Energy arc: vary blend intensity ---
    double energyArc = computeEnergyArc();

    // --- Volume fade with crossfade curve + energy arc ---
    double targetVol = (0.85 + 0.15 * curve.fadeIn) * energyArc;
    double sourceVol = curve.fadeOut * energyArc;
    setVolume(m_blendToDeck, qBound(0.0, targetVol, 1.5));
    setVolume(m_blendFromDeck, qBound(0.0, sourceVol, 1.5));

    // --- Sidechain-like vocal ducking (30-70% of blend) ---
    if (progress > 0.3 && progress < 0.7) {
        double duckProgress = 1.0 - std::abs(progress - 0.5) / 0.2;
        double duckAmount = 0.15 * duckProgress;
        ConfigKey eqMidKey(
                QStringLiteral("[Channel%1]").arg(m_blendFromDeck + 1),
                "eqMid");
        double baseMid = sourceFades.mid * eqStr;
        ControlObject::set(eqMidKey, qMax(0.0, baseMid - duckAmount));
    }

    // --- Beat-synced echo-out on source (last 20% of blend) ---
    // From ai-remixmate: beat-sync'd ping-pong delay
    if (progress > 0.8) {
        double echoProgress = (progress - 0.8) / 0.2;
        // Increase echo effect as we approach the end
        double echoStrength = echoProgress * rules.echoDecay;

        // Cut highs faster for echo effect
        double echoHigh = sourceFades.high * (1.0 - echoProgress);
        ConfigKey eqHighKey(
                QStringLiteral("[Channel%1]").arg(m_blendFromDeck + 1),
                "eqHigh");
        ControlObject::set(eqHighKey, echoHigh);

        // Reduce volume faster for echo effect
        double echoVol = sourceVol * (1.0 - echoProgress * 0.5);
        ConfigKey volKey(
                QStringLiteral("[Channel%1]").arg(m_blendFromDeck + 1),
                "volume");
        ControlObject::set(volKey, qMax(0.0, echoVol));

        // Log echo activity
        if (m_blendStep % 10 == 0) {
            kLogger.info() << "AI Bro: echo-out strength:" << echoStrength;
        }
    }
}

void AIBroFeature::stopBlend() {
    m_pBlendTimer->stop();
    m_blending = false;
    m_downloading = false;
    ++m_blendCount;

    // Ensure crossfade is fully on target deck
    if (m_blendFromDeck < m_blendToDeck) {
        m_coCrossfader.set(1.0);
    } else {
        m_coCrossfader.set(-1.0);
    }

    // Restore source deck
    setEQ(m_blendFromDeck, 1.0, 1.0, 1.0);
    setVolume(m_blendFromDeck, 1.0);
    setSync(m_blendFromDeck, false);
    {
        QString group = QStringLiteral("[Channel%1]").arg(m_blendFromDeck + 1);
        ControlObject::set(ConfigKey(group, "rate"), 0.0);
        ControlObject::set(ConfigKey(group, "pitch"), 0.0);
    }

    // Restore target deck
    setEQ(m_blendToDeck, 1.0, 1.0, 1.0);
    setVolume(m_blendToDeck, 1.0);
    {
        QString group = QStringLiteral("[Channel%1]").arg(m_blendToDeck + 1);
        ControlObject::set(ConfigKey(group, "rate"), 0.0);
        ControlObject::set(ConfigKey(group, "pitch"), 0.0);
    }

    kLogger.info() << "AI Bro: blend complete (blend #" << m_blendCount << ")";

    // Swap current deck
    m_iCurrentDeck = m_blendToDeck;
    kLogger.info() << "AI Bro: swapped current deck to" << (m_iCurrentDeck + 1);

    // Let slotProgressTick() handle next search at 50%
}

// ===================================================================
// Control helpers
// ===================================================================

void AIBroFeature::setSync(int deckIndex, bool enabled) {
    ConfigKey syncKey(
            QStringLiteral("[Channel%1]").arg(deckIndex + 1), "sync_enabled");
    ControlObject::set(syncKey, enabled ? 1.0 : 0.0);
}

void AIBroFeature::setEQ(int deckIndex, double low, double mid, double high) {
    QString group = QStringLiteral("[Channel%1]").arg(deckIndex + 1);
    ControlObject::set(ConfigKey(group, "eqLow"), low);
    ControlObject::set(ConfigKey(group, "eqMid"), mid);
    ControlObject::set(ConfigKey(group, "eqHigh"), high);
}

void AIBroFeature::setVolume(int deckIndex, double volume) {
    ConfigKey volKey(
            QStringLiteral("[Channel%1]").arg(deckIndex + 1), "volume");
    ControlObject::set(volKey, volume);
}

void AIBroFeature::setPlay(int deckIndex, bool play) {
    ConfigKey playKey(
            QStringLiteral("[Channel%1]").arg(deckIndex + 1), "play");
    ControlObject::set(playKey, play ? 1.0 : 0.0);
}

// ===================================================================
// Deck helpers
// ===================================================================

bool AIBroFeature::isDeckPlaying(int deckIndex) const {
    if (!m_pPlayerManager) {
        return false;
    }
    auto* pPlayer = m_pPlayerManager->getDeck(deckIndex);
    if (!pPlayer) {
        return false;
    }
    ConfigKey playKey(
            QStringLiteral("[Channel%1]").arg(deckIndex + 1), "play");
    return ControlObject::get(playKey) > 0.0;
}

int AIBroFeature::countPlayingDecks() const {
    if (!m_pPlayerManager) {
        return 0;
    }
    int count = 0;
    for (int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
        if (isDeckPlaying(i)) {
            ++count;
        }
    }
    return count;
}

void AIBroFeature::updateCurrentTrackInfo() {
    m_currentTrackTitle.clear();
    m_currentTrackArtist.clear();
    if (!m_pPlayerManager) {
        return;
    }
    auto* pPlayer = m_pPlayerManager->getDeck(m_iCurrentDeck);
    if (!pPlayer) {
        return;
    }
    TrackPointer pTrack = pPlayer->getLoadedTrack();
    if (!pTrack) {
        return;
    }
    m_currentTrackTitle = pTrack->getTitle();
    m_currentTrackArtist = pTrack->getArtist();
}

int AIBroFeature::findAvailableDeck() const {
    if (!m_pPlayerManager) {
        return -1;
    }
    return (m_iCurrentDeck == 0) ? 1 : 0;
}

double AIBroFeature::getDeckPlayPosition(int deckIndex) const {
    QString group = QStringLiteral("[Channel%1]").arg(deckIndex + 1);
    ConfigKey posKey(group, "playposition");
    return ControlObject::get(posKey);
}

// ===================================================================
// Vocal sync
// ===================================================================

double AIBroFeature::estimateVocalStartPosition(int deckIndex) const {
    if (!m_pPlayerManager) {
        return 0.0;
    }
    auto* pPlayer = m_pPlayerManager->getDeck(deckIndex);
    if (!pPlayer) {
        return 0.0;
    }
    TrackPointer pTrack = pPlayer->getLoadedTrack();
    if (!pTrack) {
        return 0.0;
    }

    double durationSec = pTrack->getDuration();
    if (durationSec <= 0.0) {
        return 0.0;
    }

    QString title = pTrack->getTitle().toLower();
    bool isRemix = false;
    bool isExtended = false;
    for (const QString& kw : remixKeywords()) {
        if (title.contains(kw)) {
            isRemix = true;
            if (kw == "extended" || kw == "extended mix" || kw == "club mix") {
                isExtended = true;
            }
            break;
        }
    }

    double vocalStartPercent = 0.15;
    if (isExtended) {
        vocalStartPercent = 0.30;
    } else if (isRemix) {
        vocalStartPercent = 0.22;
    } else if (durationSec > 300.0) {
        vocalStartPercent = 0.20;
    } else if (durationSec < 180.0) {
        vocalStartPercent = 0.10;
    }
    if (durationSec > 420.0) {
        vocalStartPercent = 0.35;
    }

    kLogger.info() << "AI Bro: vocal sync — title:" << title
                   << "duration:" << durationSec << "s"
                   << "vocal start:" << (vocalStartPercent * 100.0)
                   << "% (remix:" << isRemix
                   << ", extended:" << isExtended << ")";

    return vocalStartPercent;
}

// ===================================================================
// Manual track detection
// ===================================================================

QMap<int, QString> AIBroFeature::snapshotTrackLocations() const {
    QMap<int, QString> snapshot;
    if (!m_pPlayerManager) {
        return snapshot;
    }
    for (int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
        auto* pPlayer = m_pPlayerManager->getDeck(i);
        if (!pPlayer) {
            continue;
        }
        TrackPointer pTrack = pPlayer->getLoadedTrack();
        if (pTrack) {
            snapshot[i] = pTrack->getLocation();
        }
    }
    return snapshot;
}

QString AIBroFeature::findNewManualTrack() {
    if (!m_pPlayerManager) {
        return {};
    }
    QMap<int, QString> current = snapshotTrackLocations();
    for (auto it = current.begin(); it != current.end(); ++it) {
        int deck = it.key();
        const QString& location = it.value();
        if (m_searchTrackSnapshot.contains(deck) &&
                m_searchTrackSnapshot[deck] == location) {
            continue;
        }
        if (!m_searchTrackSnapshot.contains(deck) && location.isEmpty()) {
            continue;
        }
        if (!isDeckPlaying(deck)) {
            kLogger.info() << "AI Bro: manual track on deck" << (deck + 1)
                           << ":" << location;
            return location;
        }
    }
    return {};
}

// ===================================================================
// BPM detection
// ===================================================================

double AIBroFeature::getCurrentPlayingBPM() const {
    if (!m_pPlayerManager) {
        return 0.0;
    }
    for (int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
        if (!isDeckPlaying(i)) {
            continue;
        }
        auto* pPlayer = m_pPlayerManager->getDeck(i);
        if (!pPlayer) {
            continue;
        }
        TrackPointer pTrack = pPlayer->getLoadedTrack();
        if (pTrack) {
            double bpm = pTrack->getBpm();
            if (bpm > 0) {
                return bpm;
            }
        }
    }
    return 0.0;
}

double AIBroFeature::getCandidateBPM(
        const mixxx::YouTubeVideoInfo& candidate) const {
    QString title = candidate.title.toLower();
    if (title.contains("drum and bass") || title.contains("dnb") ||
            title.contains("drum & bass")) {
        return 174.0;
    }
    if (title.contains("dubstep") || title.contains("dub step")) {
        return 140.0;
    }
    if (title.contains("techno")) {
        return 130.0;
    }
    if (title.contains("trance")) {
        return 138.0;
    }
    if (title.contains("house")) {
        return 124.0;
    }
    if (title.contains("deep house")) {
        return 122.0;
    }
    if (title.contains("tech house")) {
        return 126.0;
    }
    if (title.contains("progressive")) {
        return 128.0;
    }
    if (title.contains("electro")) {
        return 128.0;
    }
    if (title.contains("hip hop") || title.contains("hip-hop") ||
            title.contains("rap")) {
        return 90.0;
    }
    if (title.contains("r&b") || title.contains("rnb") ||
            title.contains("rhythm and blues")) {
        return 85.0;
    }
    if (title.contains("pop")) {
        return 120.0;
    }
    if (title.contains("reggaeton") || title.contains("reggae")) {
        return 95.0;
    }
    if (title.contains("dancehall")) {
        return 100.0;
    }
    if (title.contains("afrobeats")) {
        return 110.0;
    }
    if (title.contains("ambient") || title.contains("chill")) {
        return 90.0;
    }
    if (title.contains("downtempo")) {
        return 85.0;
    }
    if (title.contains("breakbeat") || title.contains("breaks")) {
        return 135.0;
    }
    if (title.contains("garage")) {
        return 130.0;
    }
    if (title.contains("grime")) {
        return 140.0;
    }
    if (title.contains("jungle")) {
        return 170.0;
    }
    return 0.0;
}

void AIBroFeature::slotMusicMatcherSuggestionsReady(
        const QList<mixxx::MusicMatcherSuggestion>& suggestions) {
    if (!isActive() || !m_downloading) {
        return;
    }

    kLogger.info() << "AI Bro: Deezer suggested" << suggestions.size() << "related tracks";

    QList<mixxx::MusicMatcherSuggestion> filtered;

    for (const auto& s : suggestions) {
        // Skip empty or invalid
        if (!s.isValid())
            continue;

        // Skip same song
        if (s.title.toLower().trimmed() == m_currentTrackTitle.toLower().trimmed()) {
            continue;
        }

        // Avoid too many duplicates of the same artist (keep it fresh)
        if (s.artist.toLower().trimmed() == m_currentTrackArtist.toLower().trimmed()) {
            continue;
        }

        // Hard filter: similar title already played
        bool alreadyPlayed = false;
        QString normTitle = normalizeSongTitle(s.title);
        for (const QString& playedKey : std::as_const(m_playedSongKeys)) {
            if (titleSimilarity(normTitle, normalizeSongTitle(playedKey)) > kDedupThreshold) {
                alreadyPlayed = true;
                break;
            }
        }
        if (alreadyPlayed)
            continue;

        filtered.append(s);
    }

    if (filtered.isEmpty()) {
        kLogger.info() << "AI Bro: No suggestions left after strict filtering, trying loosely";
        for (const auto& s : suggestions) {
            if (s.isValid()) {
                filtered.append(s);
            }
        }
    }

    if (filtered.isEmpty()) {
        slotMusicMatcherSearchFailed(QStringLiteral("No valid suggestions from Deezer"));
        return;
    }

    // Pick randomly from the top 3 matching options for variety (or first if less)
    int maxIdx = qMin(filtered.size(), 3);
    int selectedIdx = (maxIdx > 1) ? (QDateTime::currentMSecsSinceEpoch() % maxIdx) : 0;
    const auto& selected = filtered[selectedIdx];

    QString query = QStringLiteral("%1 %2 official audio").arg(selected.artist, selected.title);
    kLogger.info() << "AI Bro: Selected Deezer recommended match:" << selected.artist << "-" << selected.title
                   << "(Searching YouTube for:" << query << ")";

    if (m_pYouTubeFeature) {
        m_pYouTubeFeature->searchAndActivate(query);
    }
}

void AIBroFeature::slotMusicMatcherSearchFailed(const QString& error) {
    if (!isActive() || !m_downloading) {
        return;
    }

    kLogger.warning() << "AI Bro: Deezer search failed (" << error << ") — falling back to standard YouTube search";

    const QString& artist = m_currentTrackArtist;
    const QString& title = m_currentTrackTitle;
    QString query;

    if (!artist.isEmpty() && artist.length() >= 3) {
        int style = m_blendCount % 3;
        switch (style) {
        case 0:
            query = QStringLiteral("%1 official audio").arg(artist);
            break;
        case 1:
            query = QStringLiteral("%1 lyrics").arg(artist);
            break;
        case 2:
        default:
            query = QStringLiteral("%1 popular songs").arg(artist);
            break;
        }
    } else if (!title.isEmpty() && title.length() >= 3) {
        query = title;
    } else {
        query = QStringLiteral("popular music 2025");
    }

    kLogger.info() << "AI Bro back-up query:" << query;
    if (m_pYouTubeFeature) {
        m_pYouTubeFeature->searchAndActivate(query);
    }
}

#include "moc_aibrofeature.cpp"
