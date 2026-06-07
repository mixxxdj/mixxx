#pragma once

#include <QList>
#include <QObject>
#include <QTimer>

#include "control/controlpushbutton.h"
#include "library/youtube/youtubeservice.h"
#include "preferences/usersettings.h"

namespace mixxx {
struct YouTubeVideoInfo;
}

class Library;
class PlayerManagerInterface;
class YouTubeFeature;

/// AI Bro: Intelligent Auto-DJ that finds similar songs on YouTube,
/// auto-downloads them, and seamlessly blends them together.
///
/// Workflow:
/// 1. User clicks "AI Bro" button in the toolbar to activate
/// 2. If no track is playing, fetches a trending song from YouTube
/// 3. When current track is ~75% through, searches for similar songs
/// 4. Ranks candidates by title/artist similarity
/// 5. Auto-downloads the best match
/// 6. Loads to inactive deck, syncs BPM, crossfades over 8 seconds
/// 7. Repeats infinitely until user deactivates
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
    QString buildSearchQuery();
    QStringList buildSimilarityQueries();
    double calculateSimilarity(const mixxx::YouTubeVideoInfo& candidate);
    mixxx::YouTubeVideoInfo pickBestCandidate(
            const QList<mixxx::YouTubeVideoInfo>& results);
    void downloadCandidate(const mixxx::YouTubeVideoInfo& candidate);
    void loadAndCrossfade(const QString& localPath);
    void startCrossfade();
    bool isTrackPlaying(const QString& group) const;
    int countPlayingDecks() const;

    QTimer* m_pProgressTimer;
    ControlPushButton m_controlEnabled;

    bool m_downloading;
    bool m_crossfading;

    QString m_currentTrackTitle;
    QString m_currentTrackArtist;

    /// Set of video IDs already played during this AI Bro session.
    QSet<QString> m_playedVideoIds;

    /// Set of song title+artist combos played (lowercased) to catch remixes.
    /// Cleared when AI Bro is deactivated.
    QSet<QString> m_playedSongKeys;

    Library* m_pLibrary;
    UserSettingsPointer m_pConfig;
    PlayerManagerInterface* m_pPlayerManager;
    YouTubeFeature* m_pYouTubeFeature;
};
