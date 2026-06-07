#pragma once

#include <QList>
#include <QSet>
#include <QTimer>

#include "control/controlpushbutton.h"
#include "control/proxy/controlproxy.h"
#include "preferences/configobject.h"
#include "library/youtube/youtubeservice.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"

namespace mixxx {
struct YouTubeVideoInfo;
}

class Library;
class PlayerManager;
class YouTubeFeature;

/// AI Bro: Intelligent Auto-DJ that finds similar songs on YouTube,
/// auto-downloads them, and seamlessly blends them together.
///
/// Uses Mixxx's control-based architecture:
/// - Crossfader via [Master]crossfader ControlProxy
/// - Play/stop via [ChannelN]play ControlProxy
/// - Sync via [ChannelN]sync_enabled + [ChannelN]sync_master
/// - EQ via [ChannelN]eqLow/Mid/High ControlProxy
/// - Track loading via PlayerManager::slotLoadToDeck
///
/// Blending strategy:
/// - When current track is 60-85% through, find a similar song
/// - Download and load to inactive deck
/// - Set inactive deck as sync follower of the playing deck
/// - Crossfade over 6 seconds with EQ sweep
/// - Repeat until toggle is deactivated
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
    void slotCrossfaderChanged(double value);
    void slotProgressTick();
    void slotSearchResultsReady(const QString& query,
            const QList<mixxx::YouTubeVideoInfo>& results);
    void slotDownloadFinished(const QString& videoId, const QString& localPath);
    void slotDownloadFailed(const QString& videoId, const QString& error);
    void slotBlendTick();

  private:
    // --- Song discovery ---
    QString buildSearchQuery();
    QStringList buildDiscoveryQueries();
    double scoreCandidate(const mixxx::YouTubeVideoInfo& candidate);
    mixxx::YouTubeVideoInfo pickBestCandidate(
            const QList<mixxx::YouTubeVideoInfo>& results);
    void downloadCandidate(const mixxx::YouTubeVideoInfo& candidate);

    // --- Blending ---
    void loadAndBlend(const QString& localPath);
    void startBlend(int fromDeck, int toDeck);
    void stopBlend();

    // --- Helpers ---
    bool isDeckPlaying(int deckIndex) const;
    int countPlayingDecks() const;
    int findAvailableDeck() const;
    double getDeckPlayPosition(int deckIndex) const;
    void setSync(int deckIndex, bool enabled);
    void setEQ(int deckIndex, double low, double mid, double high);
    void setVolume(int deckIndex, double volume);

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
