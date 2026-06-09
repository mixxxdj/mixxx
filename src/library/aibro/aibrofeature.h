#pragma once

#include <QList>
#include <QMap>
#include <QSet>
#include <QTimer>

#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "library/youtube/youtubeservice.h"
#include "preferences/configobject.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"

namespace mixxx {
struct YouTubeVideoInfo;
}

class Library;
class PlayerManager;
class YouTubeFeature;

/// AI Bro: Intelligent Auto-DJ that finds similar songs on YouTube,
/// auto-downloads them, and seamlessly blends them together using
/// real DJ mixing techniques.
///
/// DJ Techniques used (inspired by ai-remixmate + AI-DJ-Mixing-System):
/// - Camelot Wheel harmonic mixing for key compatibility
/// - Genre-aware transition point selection
/// - Remix/extended version preference (longer intros/outros)
/// - Echo-out transition (delay tail on outgoing track)
/// - EQ sweep crossfade (low/mid/high frequency blending)
/// - Volume fader + crossfader combined transitions
/// - BPM-aware sync and tempo matching
/// - Vocal overlap avoidance
/// - Semantic word scoring for lyrics similarity
/// - Vocal sync via position seeking
///
/// Control architecture (Mixxx control-based):
/// - [Master]crossfader for crossfade position
/// - [ChannelN]play for play/stop
/// - [ChannelN]sync_enabled for beat sync
/// - [ChannelN]eqLow/Mid/High for EQ
/// - [ChannelN]volume for volume
/// - [ChannelN]playposition for progress
/// - PlayerManager::slotLoadToDeck for track loading
///
/// User interaction model:
/// - AI Bro NEVER blocks user actions — you can load tracks manually
///   and it will detect and use them instead of searching YouTube
/// - All operations are non-blocking (timers, async signals)
/// - CPU usage is minimal (1s timer ticks, no audio analysis)
/// - If user loads a track while AI Bro is searching, the search
///   result is discarded and the user's track is used instead
class AIBroFeature : public QObject {
    Q_OBJECT
  public:
    AIBroFeature(Library* pLibrary,
            UserSettingsPointer pConfig,
            PlayerManager* pPlayerManager,
            YouTubeFeature* pYouTubeFeature);
    ~AIBroFeature() override;

    void init();
    bool isActive() const;

  public slots:
    void findNextSong();

  private slots:
    void slotToggle(bool newValue);
    void slotProgressTick();
    void slotSearchResultsReady(
            const QString& query,
            const QList<mixxx::YouTubeVideoInfo>& results);
    void slotDownloadFinished(
            const QString& videoId, const QString& localPath);
    void slotDownloadFailed(
            const QString& videoId, const QString& error);
    void slotBlendTick();
    void slotSearchFailed(
            const QString& query, const QString& error);

  private:
    // --- Song discovery ---
    QString buildSearchQuery();
    QStringList buildDiscoveryQueries();
    double scoreCandidate(const mixxx::YouTubeVideoInfo& candidate);
    mixxx::YouTubeVideoInfo pickBestCandidate(
            const QList<mixxx::YouTubeVideoInfo>& results);
    void downloadCandidate(const mixxx::YouTubeVideoInfo& candidate);

    // --- DJ Blending ---
    void loadAndBlend(const QString& localPath);
    void startBlend(int fromDeck, int toDeck);
    void stopBlend();

    // --- Control helpers (Mixxx control-based API) ---
    bool isDeckPlaying(int deckIndex) const;
    int countPlayingDecks() const;
    int findAvailableDeck() const;
    double getDeckPlayPosition(int deckIndex) const;
    void setSync(int deckIndex, bool enabled);
    void setEQ(int deckIndex, double low, double mid, double high);
    void setVolume(int deckIndex, double volume);
    void setPlay(int deckIndex, bool play);

    // --- Current track tracking ---
    void updateCurrentTrackInfo();

    // --- Vocal sync helpers ---
    double estimateVocalStartPosition(int deckIndex) const;

    // --- BPM helpers ---
    double getCurrentPlayingBPM() const;
    double getCandidateBPM(const mixxx::YouTubeVideoInfo& candidate) const;

    // --- Manual track detection ---
    QMap<int, QString> snapshotTrackLocations() const;
    QString findNewManualTrack();

    // --- State ---
    QTimer* m_pProgressTimer;
    QTimer* m_pBlendTimer;
    ConfigKey m_keyEnabled;
    ControlPushButton m_controlEnabled;
    ControlProxy m_coCrossfader;

    bool m_downloading;
    bool m_blending;
    int m_blendStep;
    int m_blendFromDeck;
    int m_blendToDeck;

    /// The deck index (0 or 1) that is currently the "active" playing deck.
    /// AI Bro blends from this deck to the other deck, then swaps.
    int m_iCurrentDeck;

    /// Timestamp of last search (for rate limiting)
    qint64 m_lastSearchTimeMs;

    QString m_currentTrackTitle;
    QString m_currentTrackArtist;

    /// Played video IDs this session.
    QSet<QString> m_playedVideoIds;
    /// Played song keys (title|uploader lowercased).
    QSet<QString> m_playedSongKeys;

    /// If user manually loads a track while we're searching, store it here
    /// and use it instead of the YouTube search result.
    QString m_manualTrackPath;
    int m_manualTrackDeck;
    bool m_hasManualTrack;

    /// Snapshot of track locations when search started (for manual override).
    QMap<int, QString> m_searchTrackSnapshot;

    Library* m_pLibrary;
    UserSettingsPointer m_pConfig;
    PlayerManager* m_pPlayerManager;
    YouTubeFeature* m_pYouTubeFeature;
};
