#include "library/aibro/aibrofeature.h"

#include <QSet>
#include <QTimer>
#include <cmath>

#include "control/controlproxy.h"
#include "engine/enginexfader.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "library/youtube/youtubefeature.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "track/track.h"
#include "util/logger.h"

namespace {
const mixxx::Logger kLogger("AIBroFeature");

// Timing constants
constexpr int kProgressIntervalMs = 1000; // check every second
constexpr int kBlendTickIntervalMs = 100; // blend animation tick
constexpr int kCrossfadeDurationMs = 6000; // 6 second crossfade
constexpr int kBlendSteps = 60;

// Blend start: begin transition when track is 60-85% through
// (picks a musically sensible point, not a fixed percentage)
constexpr double kBlendStartMin = 0.60;
constexpr double kBlendStartMax = 0.85;

// Similarity weights
constexpr double kWeightTitleOverlap = 0.30;
constexpr double kWeightArtistMatch = 0.25;
constexpr double kWeightGenreMatch = 0.15;
constexpr double kWeightBPMProximity = 0.15;
constexpr double kWeightDuration = 0.05;
constexpr double kWeightFreshness = 0.10;

// Ideal song duration range (seconds)
constexpr int kIdealDurationMin = 120; // 2 min
constexpr int kIdealDurationMax = 360; // 6 min

// BPM tolerance for matching
constexpr double kBPMTolerance = 15.0;

} // namespace

AIBroFeature::AIBroFeature(Library* pLibrary,
        UserSettingsPointer pConfig,
        PlayerManagerInterface* pPlayerManager,
        YouTubeFeature* pYouTubeFeature)
        : m_pProgressTimer(new QTimer(this)),
          m_pBlendTimer(new QTimer(this)),
          m_controlEnabled(ConfigKey("[AIBro]", "enabled")),
          m_downloading(false),
          m_blending(false),
          m_blendStep(0),
          m_currentBPM(0.0),
          m_pLibrary(pLibrary),
          m_pConfig(pConfig),
          m_pPlayerManager(pPlayerManager),
          m_pYouTubeFeature(pYouTubeFeature) {
}

AIBroFeature::~AIBroFeature() = default;

void AIBroFeature::init() {
    m_controlEnabled.connectValueChanged(this, &AIBroFeature::slotToggle);
    connect(m_pProgressTimer,
            &QTimer::timeout,
            this,
            &AIBroFeature::slotProgressTimer);
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

void AIBroFeature::notifyTrackPlaying(TrackPointer pTrack) {
    if (!pTrack) return;
    m_currentTrackTitle = pTrack->getTitle();
    m_currentTrackArtist = pTrack->getArtist();
    m_currentBPM = pTrack->getBpm();
    kLogger.info() << "AI Bro: tracking track"
                   << m_currentTrackTitle << "by" << m_currentTrackArtist
                   << "BPM:" << m_currentBPM;
}

// ---------------------------------------------------------------------------
// Toggle
// ---------------------------------------------------------------------------

void AIBroFeature::slotToggle(bool enabled) {
    if (enabled) {
        kLogger.info() << "AI Bro: activated ▶";
        m_downloading = false;
        m_blending = false;
        m_blendStep = 0;
        m_playedVideoIds.clear();
        m_playedSongKeys.clear();
        m_pProgressTimer->start();
        findNextSong();
    } else {
        kLogger.info() << "AI Bro: deactivated ■";
        m_pProgressTimer->stop();
        m_pBlendTimer->stop();
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
    queries.reserve(8);

    const QString title = m_currentTrackTitle;
    const QString artist = m_currentTrackArtist;
    if (title.isEmpty()) return queries;

    // 1. Direct artist + title (most relevant)
    if (!artist.isEmpty()) {
        queries << QStringLiteral("%1 %2").arg(artist, title);
    }

    // 2. Title + "official audio" for quality
    queries << QStringLiteral("%1 official audio").arg(title);

    // 3. Artist + "mix" for DJ-friendly versions
    if (!artist.isEmpty()) {
        queries << QStringLiteral("%1 DJ mix").arg(artist);
        queries << QStringLiteral("%1 remix").arg(artist);
    }

    // 4. "Similar to" for discovery
    queries << QStringLiteral("songs like %1").arg(title);
    if (!artist.isEmpty()) {
        queries << QStringLiteral("similar to %1 %2").arg(artist, title);
    }

    // 5. Genre-based: if we know the BPM, search for similar energy
    if (m_currentBPM > 0) {
        if (m_currentBPM >= 120 && m_currentBPM <= 130) {
            queries << QStringLiteral("house music %1").arg(title);
        } else if (m_currentBPM >= 130 && m_currentBPM <= 150) {
            queries << QStringLiteral("techno %1").arg(title);
        } else if (m_currentBPM >= 170 && m_currentBPM <= 180) {
            queries << QStringLiteral("drum and bass %1").arg(title);
        } else if (m_currentBPM < 100) {
            queries << QStringLiteral("chill %1").arg(title);
        }
    }

    // 6. Wordplay: extract key words from title for broader matching
    QStringList titleWords = title.toLower().split(' ', Qt::SkipEmptyParts);
    if (titleWords.size() >= 2) {
        // Use just the most meaningful word (skip common words)
        for (const QString& word : titleWords) {
            if (word.length() > 3 &&
                    word != "the" && word != "and" && word != "feat" &&
                    word != "ft." && word != "remix" && word != "edit") {
                queries << QStringLiteral("%1 music").arg(word);
                break;
            }
        }
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

    // --- Title overlap (Jaccard similarity on words) ---
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

    // --- Genre/vibe heuristics from title keywords ---
    // Check for genre indicators in the video title
    static const QStringList kEnergyWords = {
        "remix", "mix", "edit", "extended", "club", "festival",
        "live", "session", "bootleg", "mashup", "flip"
    };
    int energyMatches = 0;
    for (const QString& ew : kEnergyWords) {
        if (videoT.contains(ew)) ++energyMatches;
    }
    if (energyMatches > 0) {
        score += kWeightGenreMatch * qMin(energyMatches, 3) / 3.0;
    }

    // --- Duration quality ---
    if (candidate.durationSec > 0) {
        if (candidate.durationSec >= kIdealDurationMin &&
                candidate.durationSec <= kIdealDurationMax) {
            score += kWeightDuration;
        } else if (candidate.durationSec < 60 ||
                candidate.durationSec > 600) {
            score -= 0.1; // penalize very short/long
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
    m_currentVideoId = candidate.id;

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
        // No track context — fetch trending as a starting point
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
// Progress monitoring — find the right moment to blend
// ---------------------------------------------------------------------------

void AIBroFeature::slotProgressTimer() {
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
        auto* pPlayer = m_pPlayerManager->getDeck(i);
        if (!pPlayer) continue;
        const QString group = pPlayer->getGroup();
        if (!isTrackPlaying(group)) continue;

        double pos = getDeckPosition(group);

        // Start looking for blend point in the transition window
        if (pos >= kBlendStartMin && !m_downloading) {
            // Check if we're near a phrase boundary for a clean blend
            if (isNearPhraseBoundary(group) || pos >= kBlendStartMax) {
                kLogger.info() << "AI Bro: blend point at" << pos
                               << "on deck" << i;
                m_downloading = true;
                findNextSong();
                break;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Search results handler
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
    // Retry after delay
    QTimer::singleShot(kRetryDelayMs, this, [this]() {
        if (isActive()) findNextSong();
    });
}

// ---------------------------------------------------------------------------
// Smart blending using all Mixxx features
// ---------------------------------------------------------------------------

void AIBroFeature::loadAndBlend(const QString& localPath) {
    if (!m_pPlayerManager || m_blending) return;

    int targetDeck = findInactiveDeck();
    if (targetDeck < 0) {
        kLogger.warning() << "AI Bro: no inactive deck available";
        m_downloading = false;
        return;
    }

    kLogger.info() << "AI Bro: loading to deck" << targetDeck;
    m_pPlayerManager->slotLoadToDeck(localPath, targetDeck);

    // Wait for track to load, then start blend
    QTimer::singleShot(800, this, [this, targetDeck]() {
        if (!isActive()) {
            m_downloading = false;
            return;
        }
        m_blending = true;
        m_blendStep = 0;

        // Find the currently playing deck (the one we're blending from)
        int fromDeck = -1;
        for (int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
            if (i == targetDeck) continue;
            auto* pPlayer = m_pPlayerManager->getDeck(i);
            if (pPlayer && isTrackPlaying(pPlayer->getGroup())) {
                fromDeck = i;
                break;
            }
        }

        if (fromDeck < 0) {
            // No other deck playing — just start the new track
            kLogger.info() << "AI Bro: no source deck, starting fresh";
            m_blending = false;
            m_downloading = false;
            return;
        }

        executeBlend(fromDeck, targetDeck);
    });
}

double AIBroFeature::findBlendPoint(const QString& group) {
    // In a real implementation, we'd analyze the track's beat grid
    // to find the nearest phrase boundary (typically every 16 or 32 beats).
    // For now, use a heuristic based on the current position.
    double pos = getDeckPosition(group);
    // Round to nearest 0.05 (roughly a 1/20th of the track = ~2 bars at 4/4)
    return std::round(pos * 20.0) / 20.0;
}

void AIBroFeature::executeBlend(int fromDeck, int toDeck) {
    kLogger.info() << "AI Bro: blending from deck" << fromDeck
                   << "to deck" << toDeck;

    auto* pFromPlayer = m_pPlayerManager->getDeck(fromDeck);
    auto* pToPlayer = m_pPlayerManager->getDeck(toDeck);
    if (!pFromPlayer || !pToPlayer) {
        m_blending = false;
        m_downloading = false;
        return;
    }

    // 1. Sync BPM: set the target deck to follow the source deck's BPM
    //    This ensures the beats are aligned for a smooth transition
    pToPlayer->setSyncMode(SyncMode::Follower);

    // 2. Start the target deck playing (beat-synced)
    pToPlayer->play();

    // 3. Set initial EQ: target deck starts with low-pass filter engaged
    //    (muffled sound that gradually opens up)
    //    We use the EQ controls to create a smooth frequency transition
    pToPlayer->setEQ(EngineChannel::LOW, 0.0);   // cut lows initially
    pToPlayer->setEQ(EngineChannel::MID, 0.5);
    pToPlayer->setEQ(EngineChannel::HIGH, 0.3);  // reduce highs

    // 4. Start the blend timer for smooth crossfade + EQ sweep
    m_blendStep = 0;
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
        m_pBlendTimer->stop();
        m_blending = false;
        m_downloading = false;
        kLogger.info() << "AI Bro: blend complete ✓";

        // Schedule next song search
        QTimer::singleShot(kCrossfadeToSearchDelayMs, this, [this]() {
            if (isActive()) findNextSong();
        });
        return;
    }

    // Smooth easing function (ease-in-out)
    double eased = progress * progress * (3.0 - 2.0 * progress);

    // Crossfade: move from source deck to target deck
    if (m_pPlayerManager) {
        m_pPlayerManager->setCrossfaderPosition(eased);
    }

    // EQ sweep: gradually open up the target deck's frequencies
    // Source deck's EQ gradually closes
    double targetLow = eased;           // 0→1 (opens up)
    double targetMid = 0.5 + 0.5 * eased;
    double targetHigh = 0.3 + 0.7 * eased;

    // Find the target deck (the one that was loaded last)
    for (int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
        auto* pPlayer = m_pPlayerManager->getDeck(i);
        if (!pPlayer) continue;
        // The target deck is the one with the newer track
        // (we just loaded to it in loadAndBlend)
        pPlayer->setEQ(EngineChannel::LOW, targetLow);
        pPlayer->setEQ(EngineChannel::MID, targetMid);
        pPlayer->setEQ(EngineChannel::HIGH, targetHigh);
    }

    // At 50% progress, start reducing the source deck's volume
    if (progress > 0.5) {
        double sourceFade = 1.0 - (progress - 0.5) * 2.0;
        // Source deck volume reduction would go here
        // (using the deck's gain control)
        Q_UNUSED(sourceFade);
    }
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

bool AIBroFeature::isTrackPlaying(const QString& group) const {
    if (!m_pPlayerManager) return false;
    auto* pPlayer = m_pPlayerManager->getPlayer(group);
    return pPlayer && pPlayer->isPlaying();
}

int AIBroFeature::countPlayingDecks() const {
    if (!m_pPlayerManager) return 0;
    int count = 0;
    for (int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
        auto* pPlayer = m_pPlayerManager->getDeck(i);
        if (pPlayer && isTrackPlaying(pPlayer->getGroup())) ++count;
    }
    return count;
}

int AIBroFeature::findInactiveDeck() const {
    if (!m_pPlayerManager) return -1;
    for (int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
        auto* pPlayer = m_pPlayerManager->getDeck(i);
        if (pPlayer && !isTrackPlaying(pPlayer->getGroup())) return i;
    }
    return -1;
}

double AIBroFeature::getDeckBPM(const QString& group) const {
    if (!m_pPlayerManager) return 0.0;
    auto* pPlayer = m_pPlayerManager->getPlayer(group);
    if (!pPlayer) return 0.0;
    return pPlayer->getBPM();
}

double AIBroFeature::getDeckPosition(const QString& group) const {
    return PlayerInfo::instance()->getEngineBuffer(group);
}

bool AIBroFeature::isNearPhraseBoundary(const QString& group) const {
    // Heuristic: check if the current position is near a "clean" beat boundary
    // A phrase is typically 16 beats = 4 bars at 4/4 time
    // We approximate by checking if the position modulo ~0.0625 (1/16) is small
    double pos = getDeckPosition(group);
    double frac = pos - std::floor(pos);
    // Check if we're near a 1/16th boundary
    double mod = std::fmod(frac, 0.0625);
    return (mod < 0.01 || mod > 0.055);
}
