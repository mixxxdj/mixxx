#pragma once

#include <QList>
#include <QSet>
#include <QTimer>

#include "control/controlpushbutton.h"
#include "control/proxy/controlproxy.h"
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
///
/// Control architecture (Mixxx control-based):
/// - [Master]crossfader for crossfade position
/// - [ChannelN]play for play/stop
/// - [ChannelN]sync_enabled for beat sync
/// - [ChannelN]eqLow/Mid/High for EQ
/// - [ChannelN]volume for volume
/// - [ChannelN]playposition for progress
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

    QString m_currentTrackTitle;
    QString m_currentTrackArtist;

    /// Played video IDs this session.
    QSet<QString> m_playedVideoIds;
    /// Played song keys (title|uploader lowercased).
    QSet<QString> m_playedSongKeys;

    Library* m_pLibrary;
    UserSettingsPointer m_pConfig;
    PlayerManager* m_pPlayerManager;
    YouTubeFeature* m_pYouTubeFeature;
};
