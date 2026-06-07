#pragma once

#include <QList>
#include <QSet>
#include <QTimer>

#include "control/controlpushbutton.h"
#include "library/youtube/youtubeservice.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"

namespace mixxx {
struct YouTubeVideoInfo;
}

class Library;
class PlayerManagerInterface;
class YouTubeFeature;
class TrackCollection;

/// AI Bro: Intelligent Auto-DJ that finds similar songs on YouTube,
/// auto-downloads them, and seamlessly blends them together.
///
/// Architecture:
/// - Uses YouTube search + related videos for song discovery
/// - Ranks candidates by: title/artist similarity, genre match, BPM proximity,
///   duration quality, and "energy" heuristics
/// - Blends using Mixxx's full feature set: beat sync, EQ, filter, hot cues,
///   beat loops, and crossfader
/// - Transitions happen at musically sensible points (phrase boundaries,
///   breakdowns, buildups) not at a fixed percentage
/// - Tracks played songs to avoid repeats within a session
class AIBroFeature : public QObject {
    Q_OBJECT
  public:
    AIBroFeature(Library* pLibrary,
            UserSettingsPointer pConfig,
            PlayerManagerInterface* pPlayerManager,
            YouTubeFeature* pYouTubeFeature);
    ~AIBroFeature() override;

    void init();
    bool isActive() const;

    /// Public so YouTubeFeature can notify us
    void notifyTrackPlaying(TrackPointer pTrack);

  public slots:
    void findNextSong();

  private slots:
    void slotToggle(bool enabled);
    void slotProgressTimer();
    void slotSearchResultsReady(const QString& query,
            const QList<mixxx::YouTubeVideoInfo>& results);
    void slotDownloadFinished(const QString& videoId, const QString& localPath);
    void slotDownloadFailed(const QString& videoId, const QString& error);

  private:
    // --- Song discovery ---

    /// Build search query from currently playing track
    QString buildSearchQuery();

    /// Build multiple search strategies for better candidate discovery
    QStringList buildDiscoveryQueries();

    /// Calculate multi-factor similarity score (0.0 = no match, 1.0 = perfect)
    double scoreCandidate(const mixxx::YouTubeVideoInfo& candidate);

    /// Pick the best candidate from results
    mixxx::YouTubeVideoInfo pickBestCandidate(
            const QList<mixxx::YouTubeVideoInfo>& results);

    /// Download the chosen candidate
    void downloadCandidate(const mixxx::YouTubeVideoInfo& candidate);

    // --- Smart blending ---

    /// Load downloaded track and start the blend sequence
    void loadAndBlend(const QString& localPath);

    /// Determine the best blend point based on track analysis
    /// Returns the position (0.0-1.0) where we should start the transition
    double findBlendPoint(const QString& group);

    /// Execute the blend: sync BPM, set cues, EQ fade, crossfade
    void executeBlend(int fromDeck, int toDeck);

    /// Monitor the blend progress and adjust EQ/filter in real-time
    void slotBlendTick();

    // --- Helpers ---

    bool isTrackPlaying(const QString& group) const;
    int countPlayingDecks() const;
    int findInactiveDeck() const;
    double getDeckBPM(const QString& group) const;
    double getDeckPosition(const QString& group) const;
    bool isNearPhraseBoundary(const QString& group) const;

    // --- State ---

    QTimer* m_pProgressTimer;
    QTimer* m_pBlendTimer;
    ControlPushButton m_controlEnabled;

    bool m_downloading;
    bool m_blending;
    int m_blendStep;

    QString m_currentTrackTitle;
    QString m_currentTrackArtist;
    QString m_currentVideoId;
    double m_currentBPM;

    /// Played video IDs this session (cleared on deactivate)
    QSet<QString> m_playedVideoIds;
    /// Played song keys (title|uploader lowercased) to catch remixes
    QSet<QString> m_playedSongKeys;

    Library* m_pLibrary;
    UserSettingsPointer m_pConfig;
    PlayerManagerInterface* m_pPlayerManager;
    YouTubeFeature* m_pYouTubeFeature;
};
