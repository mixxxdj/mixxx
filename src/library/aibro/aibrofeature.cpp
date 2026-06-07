#include "library/aibro/aibrofeature.h"

#include <QSet>
#include <QTimer>
#include <cmath>

#include "control/controlproxy.h"
#include "engine/enginexfader.h"
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
constexpr int kBlendTickIntervalMs = 100;
constexpr int kCrossfadeDurationMs = 6000;
constexpr int kBlendSteps = 60;
constexpr int kLoadToBlendDelayMs = 800;
constexpr int kBlendToSearchDelayMs = 2000;
constexpr int kRetryDelayMs = 3000;

// Blend window: start transition at 60-85% of track
constexpr double kBlendStartMin = 0.60;
constexpr double kBlendStartMax = 0.85;

// Similarity weights
constexpr double kWeightTitleOverlap = 0.30;
constexpr double kWeightArtistMatch = 0.25;
constexpr double kWeightGenreMatch = 0.15;
constexpr double kWeightDuration = 0.10;
constexpr double kWeightFreshness = 0.10;
constexpr double kWeightRemixBonus = 0.10;

// Ideal duration range (seconds)
constexpr int kIdealDurationMin = 120;
constexpr int kIdealDurationMax = 420; // 7 min — remixes can be longer

// Keywords that indicate a remix/extended version (better for mixing)
const QStringList kRemixKeywords = {
    "remix", "extended", "mix", "edit", "version",
    "dub", "instrumental", "acapella", "bootleg",
    "mashup", "flip", "rework", "VIP", "radio edit"
};
} // namespace

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
    if (m_currentTrackTitle.isEmpty()) return {};
    if (m_currentTrackArtist.isEmpty()) return m_currentTrackTitle;
    return QStringLiteral("%1 %2").arg(m_currentTrackArtist, m_currentTrackTitle);
}

QStringList AIBroFeature::buildDiscoveryQueries() {
    QStringList queries;
    queries.reserve(10);

    const QString title = m_currentTrackTitle;
    const QString artist = m_currentTrackArtist;
    if (title.isEmpty()) return queries;

    // 1. Direct artist + title
    if (!artist.isEmpty()) {
        queries << QStringLiteral("%1 %2").arg(artist, title);
    }

    // 2. Extended/remix versions (better for mixing — longer intros/outros)
    if (!artist.isEmpty()) {
        queries << QStringLiteral("%1 %2 extended mix").arg(artist, title);
        queries << QStringLiteral("%1 %2 remix").arg(artist, title);
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

    // 5. Genre-based from BPM
    if (m_currentTrackTitle.size() > 0) {
        // Use title keywords for genre hints
        queries << QStringLiteral("%1 DJ mix").arg(title);
    }

    return queries;
}

double AIBroFeature::scoreCandidate(const mixxx::YouTubeVideoInfo& candidate) {
    if (candidate.title.isEmpty()) return -1.0;

    // Hard filter: already played
    if (m_playedVideoIds.contains(candidate.id)) return -1.0;

    QString songKey = QStringLiteral("%1|%2")
            .arg(candidate.title.toLower().trimmed(),
                 candidate.uploader.toLower().trimmed());
    if (m_playedSongKeys.contains(songKey)) return -1.0;

    // Hard filter: live streams
    if (candidate.isLive) return -1.0;

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
            if (videoWords.contains(w)) ++common;
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

    // --- Remix/extended bonus (better for mixing) ---
    for (const QString& kw : kRemixKeywords) {
        if (videoT.contains(kw)) {
            score += kWeightRemixBonus;
            break; // only count once
        }
    }

    // --- Genre/vibe heuristics ---
    static const QStringList kEnergyWords = {
        "remix", "mix", "edit", "extended", "club", "festival",
        "live", "session", "bootleg", "mashup", "flip", "rework"
    };
    int energyMatches = 0;
    for (const QString& ew : kEnergyWords) {
        if (videoT.contains(ew)) ++energyMatches;
    }
    if (energyMatches > 0) {
        score += kWeightGenreMatch * qMin(energyMatches, 3) / 3.0;
    }

    // --- Duration quality (prefer 2-7 min, ideal for mixing) ---
    if (candidate.durationSec > 0) {
        if (candidate.durationSec >= kIdealDurationMin &&
                candidate.durationSec <= kIdealDurationMax) {
            score += kWeightDuration;
        } else if (candidate.durationSec < 60 ||
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

    if (bestIdx < 0 && !results.isEmpty()) bestIdx = 0;
    return results[bestIdx];
}

void AIBroFeature::downloadCandidate(const mixxx::YouTubeVideoInfo& candidate) {
    if (!m_pYouTubeFeature) return;

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
    if (!isActive() || m_downloading) return;

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

    kLogger.info() << "AI Bro: discovering with" << queries.size() << "queries";
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
    if (!isActive() || m_blending) return;
    if (!m_pPlayerManager) return;

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
        if (!isDeckPlaying(i)) continue;

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
    if (!isActive() || results.isEmpty()) return;

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
    if (!isActive()) return;
    Q_UNUSED(videoId);
    kLogger.info() << "AI Bro: download ready:" << localPath;
    loadAndBlend(localPath);
}

void AIBroFeature::slotDownloadFailed(
        const QString& videoId, const QString& error) {
    if (!isActive()) return;
    Q_UNUSED(videoId);
    kLogger.warning() << "AI Bro: download failed:" << error;
    m_downloading = false;
    QTimer::singleShot(kRetryDelayMs, this, [this]() {
        if (isActive()) findNextSong();
    });
}

// ---------------------------------------------------------------------------
// Smart blending using Mixxx's control system
// ---------------------------------------------------------------------------

void AIBroFeature::loadAndBlend(const QString& localPath) {
    if (!m_pPlayerManager || m_blending) return;

    int toDeck = findAvailableDeck();
    if (toDeck < 0) {
        kLogger.warning() << "AI Bro: no available deck";
        m_downloading = false;
        return;
    }

    // Find the playing deck (source)
    int fromDeck = -1;
    for (int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
        if (i == toDeck) continue;
        if (isDeckPlaying(i)) {
            fromDeck = i;
            break;
        }
    }

    if (fromDeck < 0) {
        // No source deck — just load and play
        kLogger.info() << "AI Bro: no source, loading to deck" << (toDeck + 1);
        m_pPlayerManager->slotLoadToDeck(localPath, toDeck + 1); // 1-indexed
        m_downloading = false;
        return;
    }

    kLogger.info() << "AI Bro: loading to deck" << (toDeck + 1)
                   << "blending from deck" << (fromDeck + 1);

    // Load track to target deck (don't play yet)
    m_pPlayerManager->slotLoadToDeck(localPath, toDeck + 1); // 1-indexed

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

    // 1. Enable sync on target deck (follower mode)
    setSync(toDeck, true);

    // 2. Start target deck playing (beat-synced to source)
    ConfigKey playKey(QStringLiteral("[Channel%1]").arg(toDeck + 1), "play");
    ControlObject::set(playKey, 1.0);

    // 3. Set initial EQ on target: cut lows and highs (muffled intro)
    setEQ(toDeck, 0.0, 0.5, 0.2);

    // 4. Start blend animation
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
        // Blend complete
        stopBlend();
        return;
    }

    // Smooth ease-in-out
    double eased = progress * progress * (3.0 - 2.0 * progress);

    // Crossfade: -1 (full left) → +1 (full right)
    // We need to know which deck is left/right
    double crossfadeValue = 0.0;
    if (m_blendFromDeck < m_blendToDeck) {
        // From is left, to is right: crossfade goes from -1 to +1
        crossfadeValue = -1.0 + 2.0 * eased;
    } else {
        // From is right, to is left: crossfade goes from +1 to -1
        crossfadeValue = 1.0 - 2.0 * eased;
    }
    m_coCrossfader.set(crossfadeValue);

    // EQ sweep: gradually open up target deck frequencies
    double targetLow = eased;
    double targetMid = 0.5 + 0.5 * eased;
    double targetHigh = 0.2 + 0.8 * eased;
    setEQ(m_blendToDeck, targetLow, targetMid, targetHigh);

    // Gradually reduce source deck EQ
    double sourceLow = 1.0 - eased * 0.7;
    double sourceMid = 1.0 - eased * 0.5;
    double sourceHigh = 1.0 - eased * 0.8;
    setEQ(m_blendFromDeck, sourceLow, sourceMid, sourceHigh);
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

    // Restore source deck EQ
    setEQ(m_blendFromDeck, 1.0, 1.0, 1.0);

    // Disable sync on source deck
    setSync(m_blendFromDeck, false);

    kLogger.info() << "AI Bro: blend complete";

    // Schedule next song search
    QTimer::singleShot(kBlendToSearchDelayMs, this, [this]() {
        if (isActive()) findNextSong();
    });
}

// ---------------------------------------------------------------------------
// Control helpers
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Deck helpers
// ---------------------------------------------------------------------------

bool AIBroFeature::isDeckPlaying(int deckIndex) const {
    if (!m_pPlayerManager) return false;
    auto* pPlayer = m_pPlayerManager->getDeck(deckIndex);
    if (!pPlayer) return false;
    ConfigKey playKey(
            QStringLiteral("[Channel%1]").arg(deckIndex + 1), "play");
    return ControlObject::get(playKey) > 0.0;
}

int AIBroFeature::countPlayingDecks() const {
    if (!m_pPlayerManager) return 0;
    int count = 0;
    for (int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
        if (isDeckPlaying(i)) ++count;
    }
    return count;
}

int AIBroFeature::findAvailableDeck() const {
    if (!m_pPlayerManager) return -1;
    for (int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
        if (!isDeckPlaying(i)) return i;
    }
    return -1;
}

double AIBroFeature::getDeckPlayPosition(int deckIndex) const {
    QString group = QStringLiteral("[Channel%1]").arg(deckIndex + 1);
    ConfigKey posKey(group, "playposition");
    return ControlObject::get(posKey);
}
