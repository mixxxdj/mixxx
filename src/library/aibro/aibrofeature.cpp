#include "library/aibro/aibrofeature.h"

#include <QSet>
#include <QTimer>
#include <cmath>

#include "control/controlproxy.h"
#include "library/library.h"
#include "library/youtube/youtubefeature.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "track/track.h"
#include "util/logger.h"

namespace {
const mixxx::Logger kLogger("AIBroFeature");

// Timing
constexpr int kProgressIntervalMs = 1000;
constexpr int kBlendTickIntervalMs = 80;
constexpr int kCrossfadeDurationMs = 8000;
constexpr int kBlendSteps = 100;
constexpr int kLoadToBlendDelayMs = 1000;
constexpr int kBlendToSearchDelayMs = 2000;
constexpr int kRetryDelayMs = 3000;

// Blend window
constexpr double kBlendStartMin = 0.50;
constexpr double kBlendStartMax = 0.75;

// Similarity weights (inspired by ai-remixmate hybrid ranking:
// tempo 0.2 + audio 0.5 + lyrics 0.3, renormalized)
constexpr double kWeightTitleOverlap = 0.20;
constexpr double kWeightSemanticWords = 0.25;
constexpr double kWeightArtistMatch = 0.15;
constexpr double kWeightRemixBonus = 0.15;
constexpr double kWeightGenreMatch = 0.10;
constexpr double kWeightDuration = 0.05;
constexpr double kWeightFreshness = 0.05;
constexpr double kWeightKeyCompat = 0.05;

// Ideal duration (seconds) — remixes can be longer
constexpr int kIdealDurationMin = 150;
constexpr int kIdealDurationMax = 480;

// Common words to exclude from semantic scoring (stop words)
const QSet<QString> kStopWords = {
    "the", "a", "an", "and", "or", "but", "in", "on", "at", "to",
    "for", "of", "with", "by", "from", "is", "it", "this", "that",
    "i", "you", "he", "she", "we", "they", "my", "your", "his",
    "her", "our", "their", "me", "him", "us", "them", "be", "was",
    "are", "been", "being", "have", "has", "had", "do", "does",
    "did", "will", "would", "could", "should", "may", "might",
    "can", "shall", "not", "no", "nor", "as", "if", "then",
    "than", "too", "very", "just", "about", "above", "after",
    "again", "all", "also", "am", "any", "because", "before",
    "between", "both", "each", "few", "get", "got", "how",
    "its", "let", "make", "more", "most", "much", "must",
    "new", "now", "only", "other", "our", "out", "own",
    "same", "so", "some", "still", "such", "take", "tell",
    "through", "under", "up", "use", "want", "way", "well",
    "what", "when", "where", "which", "while", "who", "why",
    "feat", "ft", "vs", "remix", "edit", "mix", "version",
    "official", "audio", "video", "lyrics", "lyric",
    "explicit", "clean", "radio", "album", "single",
    "cover", "live", "acoustic", "unplugged", "rework"
};

// Camelot Wheel: compatible keys for harmonic mixing
// Built at runtime to avoid clang-format issues with large initializers
static QHash<QString, QStringList> buildCamelotMap() {
    QHash<QString, QStringList> m;
    // Major keys
    m["C"] = {"C", "Am", "G", "F"};
    m["G"] = {"G", "Em", "D", "C"};
    m["D"] = {"D", "Bm", "A", "G"};
    m["A"] = {"A", "F#m", "E", "D"};
    m["E"] = {"E", "C#m", "B", "A"};
    m["B"] = {"B", "G#m", "F#", "E"};
    m["F#"] = {"F#", "D#m", "C#", "B"};
    m["Db"] = {"Db", "Bbm", "Ab", "Gb"};
    m["Ab"] = {"Ab", "Fm", "Eb", "Db"};
    m["Eb"] = {"Eb", "Cm", "Bb", "Ab"};
    m["Bb"] = {"Bb", "Gm", "F", "Eb"};
    m["F"] = {"F", "Dm", "C", "Bb"};
    // Minor keys
    m["Am"] = {"Am", "C", "Em", "Dm"};
    m["Em"] = {"Em", "G", "Bm", "Am"};
    m["Bm"] = {"Bm", "D", "F#m", "Em"};
    m["F#m"] = {"F#m", "A", "C#m", "Bm"};
    m["C#m"] = {"C#m", "E", "G#m", "F#m"};
    m["G#m"] = {"G#m", "B", "D#m", "C#m"};
    m["D#m"] = {"D#m", "F#", "A#m", "G#m"};
    m["Bbm"] = {"Bbm", "Db", "Fm", "Ebm"};
    m["Fm"] = {"Fm", "Ab", "Cm", "Bbm"};
    m["Cm"] = {"Cm", "Eb", "Gm", "Fm"};
    m["Gm"] = {"Gm", "Bb", "Dm", "Cm"};
    m["Dm"] = {"Dm", "F", "Am", "Gm"};
    return m;
}

const QHash<QString, QStringList>& camelotMap() {
    static const auto* m = new QHash<QString, QStringList>(buildCamelotMap());
    return *m;
}

// Keywords indicating remix/extended (better for DJ mixing)
const QStringList kRemixKeywords = {
    "remix", "extended", "mix", "edit", "version", "dub",
    "instrumental", "a cappella", "bootleg", "mashup", "flip",
    "rework", "VIP", "radio edit", "club mix", "extended mix",
    "original mix"};

// Genre-specific transition preferences
struct GenreRule {
    QString preferredTransition;
    QString energyPref;
    double crossfadeMultiplier;
};

static QHash<QString, GenreRule> buildGenreRules() {
    QHash<QString, GenreRule> r;
    r["house"] = {"breakdown", "smooth", 1.0};
    r["techno"] = {"breakdown", "energetic", 0.8};
    r["trance"] = {"buildup", "energetic", 1.2};
    r["dubstep"] = {"drop", "energetic", 0.7};
    r["drum and bass"] = {"breakdown", "energetic", 0.8};
    r["hip hop"] = {"verse_end", "smooth", 1.0};
    r["r&b"] = {"verse_end", "smooth", 1.1};
    r["pop"] = {"chorus_end", "energetic", 1.0};
    r["edm"] = {"pre_drop", "energetic", 0.9};
    r["dancehall"] = {"verse_end", "energetic", 0.9};
    return r;
}

const QHash<QString, GenreRule>& genreRules() {
    static const auto* r = new QHash<QString, GenreRule>(buildGenreRules());
    return *r;
}

}  // namespace

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
          m_manualTrackPath(),
          m_manualTrackDeck(-1),
          m_hasManualTrack(false),
          m_pLibrary(pLibrary),
          m_pConfig(pConfig),
          m_pPlayerManager(pPlayerManager),
          m_pYouTubeFeature(pYouTubeFeature) {
}

AIBroFeature::~AIBroFeature() = default;

void AIBroFeature::init() {
    m_controlEnabled.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_controlEnabled.connectValueChangeRequest(
            this, &AIBroFeature::slotToggle);
    connect(m_pProgressTimer,
            &QTimer::timeout,
            this,
            &AIBroFeature::slotProgressTick);
    m_pProgressTimer->setInterval(kProgressIntervalMs);
    connect(m_pBlendTimer,
            &QTimer::timeout,
            this,
            &AIBroFeature::slotBlendTick);
    m_pBlendTimer->setInterval(kBlendTickIntervalMs);
}

bool AIBroFeature::isActive() const {
    return m_controlEnabled.toBool();
}

// ---------------------------------------------------------------------------
// Toggle
// ---------------------------------------------------------------------------

void AIBroFeature::slotToggle(bool newValue) {
    if (newValue) {
        kLogger.info() << "AI Bro: activated";
        m_downloading = false;
        m_blending = false;
        m_blendStep = 0;
        m_playedVideoIds.clear();
        m_playedSongKeys.clear();
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
    }
}

// ---------------------------------------------------------------------------
// Song discovery
// ---------------------------------------------------------------------------

QString AIBroFeature::buildSearchQuery() {
    if (m_currentTrackTitle.isEmpty()) {
        return {};
    }
    if (m_currentTrackArtist.isEmpty()) {
        return m_currentTrackTitle;
    }
    return QStringLiteral("%1 %2").arg(m_currentTrackArtist,
            m_currentTrackTitle);
}

QStringList AIBroFeature::buildDiscoveryQueries() {
    QStringList queries;
    queries.reserve(12);

    const QString title = m_currentTrackTitle;
    const QString artist = m_currentTrackArtist;
    if (title.isEmpty()) {
        return queries;
    }

    // 1. Direct artist + title
    if (!artist.isEmpty()) {
        queries << QStringLiteral("%1 %2").arg(artist, title);
    }

    // 2. Extended/remix versions — PREFER these for DJ mixing
    if (!artist.isEmpty()) {
        queries << QStringLiteral("%1 %2 extended mix").arg(artist, title);
        queries << QStringLiteral("%1 %2 remix").arg(artist, title);
        queries << QStringLiteral("%1 %2 club mix").arg(artist, title);
    }
    queries << QStringLiteral("%1 extended").arg(title);
    queries << QStringLiteral("%1 remix").arg(title);

    // 3. Official audio for quality
    queries << QStringLiteral("%1 official audio").arg(title);

    // 4. Similar songs
    queries << QStringLiteral("songs like %1").arg(title);
    if (!artist.isEmpty()) {
        queries << QStringLiteral("similar to %1").arg(artist);
    }

    // 5. DJ mix versions
    queries << QStringLiteral("%1 DJ mix").arg(title);

    return queries;
}

// ---------------------------------------------------------------------------
// Scoring — hybrid similarity inspired by ai-remixmate
// ---------------------------------------------------------------------------

double AIBroFeature::scoreCandidate(
        const mixxx::YouTubeVideoInfo& candidate) {
    if (candidate.title.isEmpty()) {
        return -1.0;
    }

    // Hard filter: already played
    if (m_playedVideoIds.contains(candidate.id)) {
        return -1.0;
    }

    QString songKey = QStringLiteral("%1|%2")
                              .arg(candidate.title.toLower().trimmed(),
                                      candidate.uploader.toLower().trimmed());
    if (m_playedSongKeys.contains(songKey)) {
        return -1.0;
    }

    // Hard filter: live streams
    if (candidate.isLive) {
        return -1.0;
    }

    double score = 0.0;
    const QString currentT = m_currentTrackTitle.toLower().trimmed();
    const QString currentA = m_currentTrackArtist.toLower().trimmed();
    const QString videoT = candidate.title.toLower().trimmed();
    const QString videoU = candidate.uploader.toLower().trimmed();

    // --- 1. Title word overlap (Jaccard) ---
    const QSet<QString> titleWords =
            currentT.split(' ', Qt::SkipEmptyParts).toSet();
    const QSet<QString> videoWords =
            videoT.split(' ', Qt::SkipEmptyParts).toSet();
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

    // --- 2. Semantic word similarity (the "words" mixing technique) ---
    // Inspired by ai-remixmate's lyrics semantic similarity (SBERT).
    // We use title words as a proxy for lyrics content.
    // Weight meaning-carrying words more heavily than stop words.
    // This captures "similar vibe/lyrics" even when titles differ.
    if (!titleWords.isEmpty() && !videoWords.isEmpty()) {
        double semanticScore = 0.0;
        int meaningfulWords = 0;
        for (const QString& w : titleWords) {
            if (kStopWords.contains(w)) {
                continue;
            }
            ++meaningfulWords;
            if (videoWords.contains(w)) {
                // Exact match of meaningful word
                semanticScore += 1.0;
            } else {
                // Partial match: check if any video word contains this word
                // or vice versa (catches "running" vs "run", "love" vs "loving")
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

    // --- 3. Artist match in uploader ---
    if (!currentA.isEmpty() && !videoU.isEmpty()) {
        if (videoU.contains(currentA) || currentA.contains(videoU)) {
            score += kWeightArtistMatch;
        }
    }

    // --- 4. Remix/extended version bonus ---
    for (const QString& kw : kRemixKeywords) {
        if (videoT.contains(kw)) {
            score += kWeightRemixBonus;
            break;
        }
    }

    // --- 5. Genre/vibe heuristics ---
    static const QStringList kEnergyWords = {
        "remix", "mix", "edit", "extended", "club", "festival",
        "live", "session", "bootleg", "mashup", "flip", "rework"
    };
    int energyMatches = 0;
    for (const QString& ew : kEnergyWords) {
        if (videoT.contains(ew)) {
            ++energyMatches;
        }
    }
    if (energyMatches > 0) {
        score += kWeightGenreMatch * qMin(energyMatches, 3) / 3.0;
    }

    // --- 6. Duration quality ---
    if (candidate.durationSec > 0) {
        if (candidate.durationSec >= kIdealDurationMin &&
                candidate.durationSec <= kIdealDurationMax) {
            score += kWeightDuration;
        } else if (candidate.durationSec < 90 ||
                candidate.durationSec > 600) {
            score -= 0.1;
        }
    }

    // --- 7. Freshness: prefer official channels ---
    if (!videoU.isEmpty()) {
        if (videoU.contains("vevo") || videoU.contains("official")) {
            score += kWeightFreshness;
        }
    }

    return qBound(0.0, score, 1.0);
}

mixxx::YouTubeVideoInfo AIBroFeature::pickBestCandidate(
        const QList<mixxx::YouTubeVideoInfo>& results) {
    double bestScore = -1;
    int bestIdx = -1;

    for (int i = 0; i < results.size(); ++i) {
        double score = scoreCandidate(results[i]);
        if (score > bestScore) {
            bestScore = score;
            bestIdx = i;
        }
    }

    if (bestIdx < 0 && !results.isEmpty()) {
        bestIdx = 0;
    }
    return results[bestIdx];
}

void AIBroFeature::downloadCandidate(
        const mixxx::YouTubeVideoInfo& candidate) {
    if (!m_pYouTubeFeature) {
        return;
    }

    m_playedVideoIds.insert(candidate.id);
    QString songKey = QStringLiteral("%1|%2")
                              .arg(candidate.title.toLower().trimmed(),
                                      candidate.uploader.toLower().trimmed());
    m_playedSongKeys.insert(songKey);

    kLogger.info() << "AI Bro: downloading [" << candidate.id << "]"
                   << candidate.title << "by" << candidate.uploader
                   << "(session:" << m_playedVideoIds.size() << "played)";
    m_pYouTubeFeature->requestDownload(candidate.id);
}

// ---------------------------------------------------------------------------
// Song finding orchestration
// ---------------------------------------------------------------------------

void AIBroFeature::findNextSong() {
    if (!isActive() || m_downloading) {
        return;
    }

    if (m_currentTrackTitle.isEmpty()) {
        kLogger.info() << "AI Bro: no track context, fetching trending";
        m_downloading = true;
        if (m_pYouTubeFeature) {
            QString region = m_pYouTubeFeature->resolvedTrendingRegion();
            m_pYouTubeFeature->searchAndActivate(
                    QStringLiteral("trending music %1").arg(region));
        }
        return;
    }

    QStringList queries = buildDiscoveryQueries();
    if (queries.isEmpty()) {
        kLogger.warning() << "AI Bro: no discovery queries built";
        return;
    }

    kLogger.info() << "AI Bro: discovering with" << queries.size()
                   << "queries";
    // Snapshot current track locations so we can detect manual loads
    m_searchTrackSnapshot = snapshotTrackLocations();
    m_downloading = true;
    for (const QString& q : queries) {
        if (m_pYouTubeFeature) {
            m_pYouTubeFeature->searchAndActivate(q);
        }
    }
}

// ---------------------------------------------------------------------------
// Progress monitoring
// ---------------------------------------------------------------------------

void AIBroFeature::slotProgressTick() {
    // Performance guard: minimal work per tick (1Hz)
    // All operations are O(1) — no loops over large collections
    if (!isActive() || m_blending) {
        return;
    }
    if (!m_pPlayerManager) {
        return;
    }

    int playingCount = countPlayingDecks();

    // Nothing playing? Fetch trending
    if (playingCount == 0 && !m_downloading) {
        kLogger.info() << "AI Bro: idle, fetching trending";
        m_downloading = true;
        if (m_pYouTubeFeature) {
            QString region = m_pYouTubeFeature->resolvedTrendingRegion();
            m_pYouTubeFeature->searchAndActivate(
                    QStringLiteral("trending music %1").arg(region));
        }
        return;
    }

    // Check each deck for blend opportunity
    for (int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
        if (!isDeckPlaying(i)) {
            continue;
        }

        double pos = getDeckPlayPosition(i);
        if (pos >= kBlendStartMin && !m_downloading) {
            kLogger.info() << "AI Bro: blend point at" << pos
                           << "on deck" << (i + 1);
            m_downloading = true;
            findNextSong();
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Search results
// ---------------------------------------------------------------------------

void AIBroFeature::slotSearchResultsReady(
        const QString& query,
        const QList<mixxx::YouTubeVideoInfo>& results) {
    Q_UNUSED(query);
    if (!isActive() || results.isEmpty()) {
        return;
    }

    kLogger.info() << "AI Bro: received" << results.size() << "results";

    // --- Manual track override ---
    // If user loaded a track manually while we were searching,
    // use that instead of the YouTube result.
    QString manualPath = findNewManualTrack();
    if (!manualPath.isEmpty()) {
        kLogger.info()
                << "AI Bro: user loaded manual track, using that instead";
        m_downloading = false;
        loadAndBlend(manualPath);
        return;
    }

    auto candidate = pickBestCandidate(results);
    double score = scoreCandidate(candidate);
    kLogger.info() << "AI Bro: selected [" << candidate.id << "]"
                   << candidate.title << "score:" << score;
    downloadCandidate(candidate);
}

// ---------------------------------------------------------------------------
// Download handlers
// ---------------------------------------------------------------------------

void AIBroFeature::slotDownloadFinished(
        const QString& videoId, const QString& localPath) {
    if (!isActive()) {
        return;
    }
    Q_UNUSED(videoId);
    kLogger.info() << "AI Bro: download ready:" << localPath;
    loadAndBlend(localPath);
}

void AIBroFeature::slotDownloadFailed(
        const QString& videoId, const QString& error) {
    if (!isActive()) {
        return;
    }
    Q_UNUSED(videoId);
    kLogger.warning() << "AI Bro: download failed:" << error;
    m_downloading = false;
    QTimer::singleShot(kRetryDelayMs, this, [this]() {
        if (isActive()) {
            findNextSong();
        }
    });
}

// ---------------------------------------------------------------------------
// DJ Blending — real DJ mixing techniques from ai-remixmate
// ---------------------------------------------------------------------------

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

    // Find the playing deck (source)
    int fromDeck = -1;
    for (int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
        if (i == toDeck) {
            continue;
        }
        if (isDeckPlaying(i)) {
            fromDeck = i;
            break;
        }
    }

    if (fromDeck < 0) {
        // No source deck — just load and play
        kLogger.info() << "AI Bro: no source, loading to deck"
                       << (toDeck + 1);
        m_pPlayerManager->slotLoadToDeck(localPath, toDeck + 1);
        m_downloading = false;
        return;
    }

    kLogger.info() << "AI Bro: loading to deck" << (toDeck + 1)
                   << "blending from deck" << (fromDeck + 1);

    // Load track to target deck (don't play yet)
    m_pPlayerManager->slotLoadToDeck(localPath, toDeck + 1);

    // Vocal sync: wait for track to load, then seek to estimated
    // vocal start position before starting the blend.
    // This aligns the "words" of the incoming track with the
    // outgoing track's transition point.
    m_blendFromDeck = fromDeck;
    m_blendToDeck = toDeck;
    QTimer::singleShot(kLoadToBlendDelayMs, this, [this]() {
        if (!isActive()) {
            m_downloading = false;
            return;
        }
        // Seek incoming track to estimated vocal start
        // (skip the intro so vocals begin at the right moment)
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

    // DJ Technique 1: Enable sync on target deck (follower)
    setSync(toDeck, true);

    // DJ Technique 2: Start target deck playing (beat-synced)
    setPlay(toDeck, true);

    // DJ Technique 3: Initial EQ on target — cut lows and highs
    // Creates a "muffled" sound that gradually opens up
    setEQ(toDeck, 0.0, 0.5, 0.2);

    // DJ Technique 4: Start with target volume slightly lower
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

    // Smooth ease-in-out curve (ai-remixmate uses crossfade_curve param)
    double eased = progress * progress * (3.0 - 2.0 * progress);

    // --- Crossfade ---
    double crossfadeValue = 0.0;
    if (m_blendFromDeck < m_blendToDeck) {
        crossfadeValue = -1.0 + 2.0 * eased;
    } else {
        crossfadeValue = 1.0 - 2.0 * eased;
    }
    m_coCrossfader.set(crossfadeValue);

    // --- DJ Technique: EQ Sweep (frequency-based transition) ---
    // Target deck: gradually open up all frequencies
    double targetLow = eased;
    double targetMid = 0.5 + 0.5 * eased;
    double targetHigh = 0.2 + 0.8 * eased;
    setEQ(m_blendToDeck, targetLow, targetMid, targetHigh);

    // Source deck: reduce highs first (classic DJ technique)
    // then mids, then lows — creates a "fading into distance" effect
    double sourceHigh = 1.0 - eased * 0.9;
    double sourceMid = 1.0 - eased * 0.5;
    double sourceLow = 1.0 - eased * 0.3;
    setEQ(m_blendFromDeck, sourceLow, sourceMid, sourceHigh);

    // --- DJ Technique: Sidechain-like vocal ducking ---
    // During the middle of the crossfade (30-70%), duck the source deck's
    // mids slightly to make room for the incoming track's vocals
    // This mimics ai-remixmate's sidechain compression
    if (progress > 0.3 && progress < 0.7) {
        double duckProgress = 1.0 - std::abs(progress - 0.5) / 0.2;
        double duckAmount = 0.15 * duckProgress;
        ConfigKey eqMidKey(
                QStringLiteral("[Channel%1]").arg(m_blendFromDeck + 1),
                "eqMid");
        double duckedMid = sourceMid - duckAmount;
        ControlObject::set(eqMidKey, qMax(0.0, duckedMid));
    }

    // --- DJ Technique: Volume fade ---
    double targetVol = 0.85 + 0.15 * eased;
    double sourceVol = 1.0 - eased;
    setVolume(m_blendToDeck, targetVol);
    setVolume(m_blendFromDeck, sourceVol);

    // --- DJ Technique: Echo-out on source (last 20% of blend) ---
    // Reduce source volume faster and cut highs for "echo" effect
    if (progress > 0.8) {
        double echoProgress = (progress - 0.8) / 0.2;
        double echoHigh = sourceHigh * (1.0 - echoProgress);
        ConfigKey eqHighKey(
                QStringLiteral("[Channel%1]").arg(m_blendFromDeck + 1),
                "eqHigh");
        ControlObject::set(eqHighKey, echoHigh);

        // Also reduce volume faster for echo effect
        double echoVol = sourceVol * (1.0 - echoProgress * 0.5);
        ConfigKey volKey(
                QStringLiteral("[Channel%1]").arg(m_blendFromDeck + 1),
                "volume");
        ControlObject::set(volKey, echoVol);
    }
}

void AIBroFeature::stopBlend() {
    m_pBlendTimer->stop();
    m_blending = false;
    m_downloading = false;

    // Ensure crossfade is fully on target deck
    if (m_blendFromDeck < m_blendToDeck) {
        m_coCrossfader.set(1.0);
    } else {
        m_coCrossfader.set(-1.0);
    }

    // Restore source deck EQ and volume
    setEQ(m_blendFromDeck, 1.0, 1.0, 1.0);
    setVolume(m_blendFromDeck, 1.0);

    // Disable sync on source deck
    setSync(m_blendFromDeck, false);

    // Ensure target deck is at full EQ and volume
    setEQ(m_blendToDeck, 1.0, 1.0, 1.0);
    setVolume(m_blendToDeck, 1.0);

    kLogger.info() << "AI Bro: blend complete";

    // Schedule next song search
    QTimer::singleShot(kBlendToSearchDelayMs, this, [this]() {
        if (isActive()) {
            findNextSong();
        }
    });
}

// ---------------------------------------------------------------------------
// Control helpers — Mixxx control-based API
// ---------------------------------------------------------------------------

void AIBroFeature::setSync(int deckIndex, bool enabled) {
    ConfigKey syncKey(
            QStringLiteral("[Channel%1]").arg(deckIndex + 1),
            "sync_enabled");
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

// ---------------------------------------------------------------------------
// Deck helpers
// ---------------------------------------------------------------------------

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

int AIBroFeature::findAvailableDeck() const {
    if (!m_pPlayerManager) {
        return -1;
    }
    for (int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
        if (!isDeckPlaying(i)) {
            return i;
        }
    }
    return -1;
}

double AIBroFeature::getDeckPlayPosition(int deckIndex) const {
    QString group = QStringLiteral("[Channel%1]").arg(deckIndex + 1);
    ConfigKey posKey(group, "playposition");
    return ControlObject::get(posKey);
}

// ---------------------------------------------------------------------------
// Vocal sync — estimate where vocals start in a track
// ---------------------------------------------------------------------------
double AIBroFeature::estimateVocalStartPosition(int deckIndex) const {
    if (!m_pPlayerManager) {
        return 0.0;
    }
    auto* pPlayer = m_pPlayerManager->getDeck(deckIndex);
    if (!pPlayer) {
        return 0.0;
    }
    auto* pTrack = pPlayer->getLoadedTrack();
    if (!pTrack) {
        return 0.0;
    }

    // Get track duration in seconds
    double durationSec = pTrack->getDuration();
    if (durationSec <= 0.0) {
        return 0.0;
    }

    // Heuristic: estimate vocal start based on track title keywords
    // and duration. This is a proxy for actual lyrics analysis.
    QString title = pTrack->getTitle().toLower();
    QString artist = pTrack->getArtist().toLower();

    // Extended/remix versions have longer intros (instrumental buildup)
    // Typical structure: intro (25-40%) → verse → chorus → ...
    bool isRemix = false;
    bool isExtended = false;
    for (const QString& kw : kRemixKeywords) {
        if (title.contains(kw)) {
            isRemix = true;
            if (kw == "extended" || kw == "extended mix" ||
                    kw == "club mix") {
                isExtended = true;
            }
            break;
        }
    }

    double vocalStartPercent = 0.15;  // Default: vocals at 15%

    if (isExtended) {
        // Extended mixes: long instrumental intro (30-40%)
        // Perfect for DJ mixing — lots of time to blend
        vocalStartPercent = 0.30;
    } else if (isRemix) {
        // Remixes: moderate intro (20-30%)
        vocalStartPercent = 0.22;
    } else if (durationSec > 300.0) {
        // Long tracks (>5 min): likely have longer intros
        vocalStartPercent = 0.20;
    } else if (durationSec < 180.0) {
        // Short tracks (<3 min): vocals start earlier
        vocalStartPercent = 0.10;
    }

    // For very long tracks (>7 min), could be a mix/medley
    if (durationSec > 420.0) {
        vocalStartPercent = 0.35;
    }

    kLogger.info() << "AI Bro: vocal sync estimate — title:" << title
                   << "duration:" << durationSec << "s"
                   << "vocal start:" << (vocalStartPercent * 100.0)
                   << "% (remix:" << isRemix
                   << ", extended:" << isExtended << ")";

    return vocalStartPercent;
}

// ---------------------------------------------------------------------------
// Manual track detection — user can load tracks while AI Bro runs
// ---------------------------------------------------------------------------

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
        auto* pTrack = pPlayer->getLoadedTrack();
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
        // Check if this deck has a different track than when we started
        // searching, and the deck is not currently playing (user loaded
        // it to an inactive deck for us to mix in)
        if (m_searchTrackSnapshot.contains(deck) &&
                m_searchTrackSnapshot[deck] == location) {
            continue;  // Same track as before
        }
        if (!m_searchTrackSnapshot.contains(deck) && location.isEmpty()) {
            continue;  // Was empty, still empty
        }
        // Found a new track! But only use it if the deck is not already
        // playing (user loaded it to an inactive deck)
        if (!isDeckPlaying(deck)) {
            kLogger.info()
                    << "AI Bro: detected manual track on deck" << (deck + 1)
                    << ":" << location;
            return location;
        }
    }
    return {};
}
