#include "library/aibro/aibrofeature.h"

#include <QSet>
#include <QTimer>

#include "control/controlproxy.h"
#include "library/library.h"
#include "library/youtube/youtubefeature.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "track/track.h"
#include "util/logger.h"

namespace {
const mixxx::Logger kLogger("AIBroFeature");
constexpr double kLookAheadProgress = 0.75;
constexpr int kProgressIntervalMs = 2000;
constexpr int kCrossfadeDurationMs = 8000;
constexpr int kCrossfadeSteps = 80;
constexpr int kLoadToCrossfadeDelayMs = 1000;
constexpr int kCrossfadeToSearchDelayMs = 3000;
constexpr int kRetryDelayMs = 2000;
} // namespace

AIBroFeature::AIBroFeature(Library* pLibrary,
        UserSettingsPointer pConfig,
        PlayerManagerInterface* pPlayerManager,
        YouTubeFeature* pYouTubeFeature)
    : m_pProgressTimer(new QTimer(this)),
      m_controlEnabled(ConfigKey("[AIBro]", "enabled")),
      m_downloading(false),
      m_crossfading(false),
      m_pLibrary(pLibrary),
      m_pConfig(pConfig),
      m_pPlayerManager(pPlayerManager),
      m_pYouTubeFeature(pYouTubeFeature) {
}

AIBroFeature::~AIBroFeature() = default;

void AIBroFeature::init() {
    m_controlEnabled.connectValueChanged(this, &AIBroFeature::slotToggle);
    connect(m_pProgressTimer, &QTimer::timeout,
            this, &AIBroFeature::slotProgressTimer);
    m_pProgressTimer->setInterval(kProgressIntervalMs);
}

bool AIBroFeature::isActive() const {
    return m_controlEnabled.toBool();
}

void AIBroFeature::slotToggle(bool enabled) {
    if (enabled) {
        kLogger.info() << "AI Bro: activated";
        m_downloading = false;
        m_crossfading = false;
        m_playedVideoIds.clear();
        m_playedSongKeys.clear();
        m_pProgressTimer->start();
        findNextSong();
    } else {
        kLogger.info() << "AI Bro: deactivated";
        m_pProgressTimer->stop();
        m_playedVideoIds.clear();
        m_playedSongKeys.clear();
    }
}

QString AIBroFeature::buildSearchQuery() {
    if (m_currentTrackTitle.isEmpty()) return {};
    if (m_currentTrackArtist.isEmpty()) return m_currentTrackTitle;
    return QStringLiteral("%1 %2").arg(m_currentTrackArtist, m_currentTrackTitle);
}

QStringList AIBroFeature::buildSimilarityQueries() {
    QStringList queries;
    queries.reserve(5);
    const QString title = m_currentTrackTitle;
    const QString artist = m_currentTrackArtist;
    if (title.isEmpty()) return queries;

    if (!artist.isEmpty()) {
        queries << QStringLiteral("%1 %2").arg(artist, title);
    }
    queries << QStringLiteral("%1 official audio").arg(title);
    if (!artist.isEmpty()) {
        queries << QStringLiteral("%1 mix").arg(artist);
    }
    queries << QStringLiteral("similar to %1").arg(title);
    queries << title;
    return queries;
}

double AIBroFeature::calculateSimilarity(
        const mixxx::YouTubeVideoInfo& candidate) {
    if (m_currentTrackTitle.isEmpty() || candidate.title.isEmpty()) {
        return 0.0;
    }

    // Skip if this exact video was already played
    if (m_playedVideoIds.contains(candidate.id)) {
        return -1.0;
    }

    // Skip if same title+artist combo was played (catches remixes)
    QString songKey = QStringLiteral("%1|%2")
            .arg(candidate.title.toLower().trimmed(),
                 candidate.uploader.toLower().trimmed());
    if (m_playedSongKeys.contains(songKey)) {
        return -1.0;
    }

    const QString currentT = m_currentTrackTitle.toLower().trimmed();
    const QString currentA = m_currentTrackArtist.toLower().trimmed();
    const QString videoT = candidate.title.toLower().trimmed();
    const QString videoUploader = candidate.uploader.toLower().trimmed();

    double score = 0.0;

    // Title word overlap (Jaccard-like)
    const QSet<QString> titleWords =
            currentT.split(' ', Qt::SkipEmptyParts).toSet();
    const QSet<QString> videoWords =
            videoT.split(' ', Qt::SkipEmptyParts).toSet();
    if (!titleWords.isEmpty() && !videoWords.isEmpty()) {
        int common = 0;
        for (const QString& w : titleWords) {
            if (videoWords.contains(w)) ++common;
        }
        score += 0.4 * static_cast<double>(common) /
                qMax(titleWords.size(), videoWords.size());
    }

    // Artist match in uploader
    if (!currentA.isEmpty() && !videoUploader.isEmpty()) {
        if (videoUploader.contains(currentA) || currentA.contains(videoUploader)) {
            score += 0.3;
        }
    }

    // Title containment
    if (videoT.contains(currentT) || currentT.contains(videoT)) {
        score += 0.2;
    }

    // Penalize very short/long tracks
    if (candidate.durationSec > 0) {
        if (candidate.durationSec < 60 || candidate.durationSec > 600) {
            score -= 0.1;
        }
    }

    // Penalize live streams
    if (candidate.isLive) score -= 0.3;

    return qBound(0.0, score, 1.0);
}

mixxx::YouTubeVideoInfo AIBroFeature::pickBestCandidate(
        const QList<mixxx::YouTubeVideoInfo>& results) {
    double bestScore = -1;
    int bestIdx = -1;

    for (int i = 0; i < results.size(); ++i) {
        double score = calculateSimilarity(results[i]);
        if (score > bestScore) {
            bestScore = score;
            bestIdx = i;
        }
    }

    if (bestIdx < 0 && !results.isEmpty()) bestIdx = 0;
    return results[bestIdx];
}

void AIBroFeature::findNextSong() {
    if (!isActive()) return;
    if (m_downloading) return;

    if (m_currentTrackTitle.isEmpty()) {
        // No track info - fetch trending as starting point
        kLogger.info() << "AI Bro: no current track, fetching trending";
        m_downloading = true;
        if (m_pYouTubeFeature) {
            QString region = m_pYouTubeFeature->resolvedTrendingRegion();
            m_pYouTubeFeature->searchAndActivate(
                    QStringLiteral("trending music %1").arg(region));
        }
        return;
    }

    QStringList queries = buildSimilarityQueries();
    if (queries.isEmpty()) {
        kLogger.warning() << "AI Bro: no search queries could be built";
        return;
    }

    kLogger.info() << "AI Bro: searching with" << queries.size() << "strategies";
    m_downloading = true;
    for (const QString& q : queries) {
        if (m_pYouTubeFeature) {
            m_pYouTubeFeature->searchAndActivate(q);
        }
    }
}

void AIBroFeature::slotProgressTimer() {
    if (!isActive()) return;
    if (m_crossfading) return;
    if (!m_pPlayerManager) return;

    int playingCount = countPlayingDecks();

    if (playingCount == 0 && !m_downloading) {
        kLogger.info() << "AI Bro: no decks playing, fetching trending";
        m_downloading = true;
        if (m_pYouTubeFeature) {
            QString region = m_pYouTubeFeature->resolvedTrendingRegion();
            m_pYouTubeFeature->searchAndActivate(
                    QStringLiteral("trending music %1").arg(region));
        }
        return;
    }

    for (int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
        auto* pPlayer = m_pPlayerManager->getDeck(i);
        if (!pPlayer) continue;
        const QString group = pPlayer->getGroup();
        if (!isTrackPlaying(group)) continue;

        double position = PlayerInfo::instance()->getEngineBuffer(group);
        if (position >= kLookAheadProgress && !m_downloading) {
            kLogger.info() << "AI Bro: deck" << i << "at" << position << "- finding next";
            m_downloading = true;
            findNextSong();
            break;
        }
    }
}

void AIBroFeature::slotSearchResultsReady(
        const QString& query,
        const QList<mixxx::YouTubeVideoInfo>& results) {
    Q_UNUSED(query);
    if (!isActive()) return;
    if (results.isEmpty()) return;

    kLogger.info() << "AI Bro: got" << results.size() << "results";

    auto candidate = pickBestCandidate(results);
    kLogger.info() << "AI Bro: best candidate:" << candidate.title
                   << "uploader:" << candidate.uploader;
    downloadCandidate(candidate);
}

void AIBroFeature::downloadCandidate(const mixxx::YouTubeVideoInfo& candidate) {
    if (!m_pYouTubeFeature) return;

    m_playedVideoIds.insert(candidate.id);
    QString songKey = QStringLiteral("%1|%2")
            .arg(candidate.title.toLower().trimmed(),
                 candidate.uploader.toLower().trimmed());
    m_playedSongKeys.insert(songKey);

    kLogger.info() << "AI Bro: downloading" << candidate.id
                   << candidate.title
                   << "(played:" << m_playedVideoIds.size() << "songs)";
    m_pYouTubeFeature->requestDownload(candidate.id);
}

void AIBroFeature::slotDownloadFinished(
        const QString& videoId, const QString& localPath) {
    if (!isActive()) return;
    Q_UNUSED(videoId);
    kLogger.info() << "AI Bro: download finished:" << localPath;
    loadAndCrossfade(localPath);
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

void AIBroFeature::loadAndCrossfade(const QString& localPath) {
    if (!m_pPlayerManager) return;
    if (m_crossfading) return;
    m_crossfading = true;

    int playingDeck = -1, nextDeck = -1;
    for (int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
        auto* pPlayer = m_pPlayerManager->getDeck(i);
        if (!pPlayer) continue;
        if (isTrackPlaying(pPlayer->getGroup())) {
            if (playingDeck == -1) playingDeck = i;
            else if (nextDeck == -1) nextDeck = i;
        }
    }

    if (nextDeck == -1) nextDeck = (playingDeck == 0) ? 1 : 0;
    if (nextDeck == -1 || nextDeck >= m_pPlayerManager->numberOfDecks()) {
        kLogger.warning() << "AI Bro: no available deck";
        m_crossfading = false;
        m_downloading = false;
        return;
    }

    kLogger.info() << "AI Bro: loading" << localPath << "to deck" << nextDeck;
    m_pPlayerManager->slotLoadToDeck(localPath, nextDeck);

    QTimer::singleShot(kLoadToCrossfadeDelayMs, this, [this]() {
        if (!isActive()) {
            m_crossfading = false;
            m_downloading = false;
            return;
        }
        startCrossfade();
    });
}

void AIBroFeature::startCrossfade() {
    const int stepMs = kCrossfadeDurationMs / kCrossfadeSteps;
    for (int i = 0; i <= kCrossfadeSteps; ++i) {
        QTimer::singleShot(i * stepMs, this, [this, i]() {
            if (!isActive()) return;
            double pos = static_cast<double>(i) / kCrossfadeSteps;
            if (m_pPlayerManager) {
                m_pPlayerManager->setCrossfaderPosition(pos);
            }
            if (i == kCrossfadeSteps) {
                m_crossfading = false;
                m_downloading = false;
                kLogger.info() << "AI Bro: crossfade complete";
                QTimer::singleShot(kCrossfadeToSearchDelayMs, this, [this]() {
                    if (isActive()) findNextSong();
                });
            }
        });
    }
}

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
