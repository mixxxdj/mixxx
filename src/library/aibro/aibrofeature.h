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
class MusicMatcherClient;
struct MusicMatcherSuggestion;
} // namespace mixxx

class Library;
class PlayerManager;
class YouTubeFeature;

/// AI Bro: Intelligent Auto-DJ that finds similar songs on YouTube,
/// auto-downloads them, and seamlessly blends them together using
/// professional DJ mixing techniques.
///
/// DJ Techniques (inspired by ai-remixmate + AI-DJ-Mixing-System):
/// - 3-band frequency-selective fade (bass/mid/high fade at different rates)
/// - Genre-aware transition rules (overlap duration, EQ strength)
/// - Stem-aware crossfade curves (similarity-based blend shapes)
/// - Beat-synced echo-out (delay echoes timed to BPM)
/// - BPM correction (double-time/half-time detection)
/// - Energy arc (vary blend intensity over session)
/// - Phrase-boundary alignment (8-beat phrase grid)
/// - Vocal sync via position seeking
///
/// Control architecture (Mixxx control-based):
/// - [Master]crossfader for crossfade position
/// - [ChannelN]play for play/stop
/// - [ChannelN]eqLow/Mid/High for EQ
/// - [ChannelN]volume for volume
/// - [ChannelN]playposition for progress
/// - [ChannelN]rate/pitch for tempo
/// - PlayerManager::slotLoadToDeck for track loading
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
    void slotMusicMatcherSuggestionsReady(
            const QList<mixxx::MusicMatcherSuggestion>& suggestions);
    void slotMusicMatcherSearchFailed(
            const QString& error);

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

    // --- Genre-aware mixing rules ---
    struct MixingRules {
        double overlapMultiplier; // Blend duration multiplier
        double eqStrength;        // EQ filter strength
        double echoDecay;         // Echo-out decay factor
        int echoBeats;            // Number of echo beats
    };
    MixingRules getMixingRules() const;

    // --- 3-band frequency-selective fade ---
    // Returns per-band fade multipliers for a given progress (0-1)
    struct BandFades {
        double low;
        double mid;
        double high;
    };
    BandFades computeBandFades(double progress, bool outgoing) const;

    // --- Stem-aware crossfade curves ---
    // Returns fade-out and fade-in multipliers based on track similarity
    struct CrossfadeCurve {
        double fadeOut;
        double fadeIn;
    };
    CrossfadeCurve computeCrossfadeCurve(double progress) const;

    // --- Energy arc ---
    // Returns blend intensity multiplier based on session progress (0-1)
    double computeEnergyArc() const;

    // --- BPM correction ---
    double correctBPM(double detectedBPM, double expectedBPM) const;

    // --- Phrase alignment ---
    double alignToPhraseBoundary(double positionSec) const;

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
    int m_iCurrentDeck;

    /// Timestamp of last search (for rate limiting)
    qint64 m_lastSearchTimeMs;

    QString m_currentTrackTitle;
    QString m_currentTrackArtist;

    /// Played video IDs this session.
    QSet<QString> m_playedVideoIds;
    /// Played song keys (title lowercased).
    QSet<QString> m_playedSongKeys;

    /// If user manually loads a track while we're searching
    QString m_manualTrackPath;
    int m_manualTrackDeck;
    bool m_hasManualTrack;

    /// Snapshot of track locations when search started.
    QMap<int, QString> m_searchTrackSnapshot;

    QSet<QString> m_playedArtists;
    QString m_downloadingTitle;
    QString m_downloadingUploader;

    /// Session blend count (for energy arc)
    int m_blendCount;
    /// Current session BPM (for beat-synced echo)
    double m_sessionBPM;

    Library* m_pLibrary;
    UserSettingsPointer m_pConfig;
    PlayerManager* m_pPlayerManager;
    YouTubeFeature* m_pYouTubeFeature;
    mixxx::MusicMatcherClient* m_pMusicMatcher;
};
