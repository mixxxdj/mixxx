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
constexpr double kBlendStartMin = 0.55;
constexpr double kBlendStartMax = 0.80;

// Similarity weights
constexpr double kWeightTitleOverlap = 0.25;
constexpr double kWeightArtistMatch = 0.20;
constexpr double kWeightRemixBonus = 0.20;
constexpr double kWeightGenreMatch = 0.10;
constexpr double kWeightDuration = 0.10;
constexpr double kWeightFreshness = 0.05;
constexpr double kWeightKeyCompat = 0.10;

// Ideal duration (seconds) — remixes can be longer
constexpr int kIdealDurationMin = 150;
constexpr int kIdealDurationMax = 480;

// Camelot Wheel: compatible keys for harmonic mixing
// Format: key -> {same_key, relative_minor/major, dominant, subdominant}
// This is a simplified version — full Camelot has 12 major + 12 minor
const QHash<QString, QStringList> kCamelotCompatible = {
    // Major keys
    {"C", {"C", "Am", "G", "F"}},
    {"G", {"G", "Em", "D", "C"}},
    {"D", {"D", "Bm", "A", "G"}},
    {"A", {"A", "F#m", "E", "D"}},
    {"E", {"E", "C#m", "B", "A"}},
    {"B", {"B", "G#m", "F#", "E"}},
    {"F#", {"F#", "D#m", "C#", "B"}},
    {"Db", {"Db", "Bbm", "Ab", "Gb"}},
    {"Ab", {"Ab", "Fm", "Eb", "Db"}},
    {"Eb", {"Eb", "Cm", "Bb", "Ab"}},
    {"Bb", {"Bb", "Gm", "F", "Eb"}},
    {"F", {"F", "Dm", "C", "Bb"}},
    // Minor keys
    {"Am", {"Am", "C", "Em", "Dm"}},
    {"Em", {"Em", "G", "Bm", "Am"}},
    {"Bm", {"Bm", "D", "F#m", "Em"}},
    {"F#m", {"F#m", "A", "C#m", "Bm"}},
    {"C#m", {"C#m", "E", "G#m", "F#m"}},
    {"G#m", {"G#m", "B", "D#m", "C#m"}},
    {"D#m", {"D#m", "F#", "A#m", "G#m"}},
    {"Bbm", {"Bbm", "Db", "Fm", "Ebm"}},
    {"Fm", {"Fm", "Ab", "Cm", "Bbm"}},
    {"Cm", {"Cm", "Eb", "Gm", "Fm"}},
    {"Gm", {"Gm", "Bb", "Dm", "Cm"}},
    {"Dm", {"Dm", "F", "Am", "Gm"}},
};

// Keywords indicating remix/extended (better for DJ mixing)
const QStringList kRemixKeywords = {
    "remix", "extended", "mix", "edit", "version",
    "dub", "instrumental", "a cappella", "bootleg",
    "mashup", "flip", "rework", "VIP", "radio edit",
    "club mix", "extended mix", "original mix"
};

// Genre-specific transition preferences
struct GenreRule {
    QString preferredTransition;  // "breakdown", "chorus_end", "verse_end", "drop"
    QString energyPref;           // "smooth", "energetic", "build"
    double crossfadeMultiplier;   // adjust crossfade duration
};

const QHash<QString, GenreRule> kGenreRules = {
    {"house", {"breakdown", "smooth", 1.0}},
    {"techno", {"breakdown", "energetic", 0.8}},
    {"trance", {"buildup", "energetic", 1.2}},
    {"dubstep", {"drop", "energetic", 0.7}},
    {"drum and bass", {"breakdown", "energetic", 0.8}},
    {"hip hop", {"verse_end", "smooth", 1.0}},
    {"r&b", {"verse_end", "smooth", 1.1}},
    {"pop", {"chorus_end", "energetic", 1.0}},
    {"edm", {"pre_drop", "energetic", 0.9}},
    {"dancehall", {"verse_end", "energetic", 0.9}},
};

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
    //    (longer intros/outros = easier to blend)
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

    // --- Title word overlap (Jaccard) ---
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

    // --- Artist match in uploader ---
    if (!currentA.isEmpty() && !videoU.isEmpty()) {
        if (videoU.contains(currentA) || currentA.contains(videoU)) {
            score += kWeightArtistMatch;
        }
    }

    // --- Remix/extended version bonus ---
    // Remixes and extended versions have longer intros/outros,
    // making them much easier to DJ-mix
    for (const QString& kw : kRemixKeywords) {
        if (videoT.contains(kw)) {
            score += kWeightRemixBonus;
            break;
        }
    }

    // --- Genre/vibe heuristics ---
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

    // --- Duration quality ---
    // Prefer 2.5-8 min tracks (ideal for DJ mixing)
    if (candidate.durationSec > 0) {
        if (candidate.durationSec >= kIdealDurationMin &&
                candidate.durationSec <= kIdealDurationMax) {
            score += kWeightDuration;
        } else if (candidate.durationSec < 90 ||
                candidate.durationSec > 600) {
            score -= 0.1;
        }
    }

    // --- Freshness: prefer official channels ---
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
// DJ Blending — real DJ mixing techniques
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

    // Wait for track to load, then start blend
    m_blendFromDeck = fromDeck;
    m_blendToDeck = toDeck;
    QTimer::singleShot(kLoadToBlendDelayMs, this, [this]() {
        if (!isActive()) {
            m_downloading = false;
            return;
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
    // This locks the BPM so beats align during crossfade
    setSync(toDeck, true);

    // DJ Technique 2: Start target deck playing (beat-synced)
    setPlay(toDeck, true);

    // DJ Technique 3: Initial EQ on target — cut lows and highs
    // Creates a "muffled" sound that gradually opens up
    // This is the classic DJ "EQ sweep" technique
    setEQ(toDeck, 0.0, 0.5, 0.2);

    // DJ Technique 4: Start with target volume slightly lower
    // Then fade in during crossfade
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
    double eased = progress * progress * (3.0 - 2.0 * progress);

    // --- Crossfade ---
    // Map eased 0..1 to crossfade position
    // fromDeck left (-1) to toDeck right (+1) or vice versa
    double crossfadeValue = 0.0;
    if (m_blendFromDeck < m_blendToDeck) {
        crossfadeValue = -1.0 + 2.0 * eased;
    } else {
        crossfadeValue = 1.0 - 2.0 * eased;
    }
    m_coCrossfader.set(crossfadeValue);

    // --- DJ Technique: EQ Sweep ---
    // Target deck: gradually open up all frequencies
    // Source deck: gradually close frequencies (echo-out effect)
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

    // --- DJ Technique: Volume fade ---
    // Target deck: fade in from 0.85 to 1.0
    // Source deck: fade out from 1.0 to 0.0
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
