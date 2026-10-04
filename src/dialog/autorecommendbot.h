#pragma once

#include <QObject>
#include <QSet>
#include <QString>

#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
#include "dialog/autorecommendationengine.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"
#include "track/trackid.h"
#include "util/parented_ptr.h"

class PlaylistDAO;
class PlayerManager;
class TrackCollectionManager;

// Controls the state of the Auto-Recommendation bot and stores its
// settings in the Mixxx configuration. When enabled, the bot watches
// the currently playing track and keeps the AutoDJ queue topped up
// with the best matching tracks from the library, determined by the
// recommendation algorithm in autorecommendationengine.
//
// Independently of AutoDJ, the bot can also preload tracks into idle
// decks: whenever a deck stops playing, the best matching track is
// loaded into the first empty (stopped) deck, or into the deck that
// just stopped if no empty deck exists. This is skipped while AutoDJ
// is enabled so both features never fight over the decks.
//
// With fade/transition awareness enabled, the queue order is
// sequenced by each track's duration and BPM: consecutive queue
// entries are ordered for smooth fades (tempo compatibility including
// half/double time) and a usable fade window (track length).
class AutoRecommendBot : public QObject {
    Q_OBJECT

  public:
    static constexpr int kDefaultQueueSize = 5;
    static constexpr int kMaxQueueSize = 20;

    AutoRecommendBot(
            TrackCollectionManager* pTrackCollectionManager,
            PlayerManager* pPlayerManager,
            UserSettingsPointer pConfig,
            QObject* pParent = nullptr);

    bool isEnabled() const {
        return m_enabled;
    }

    bool isDeckPreloadingEnabled() const {
        return m_deckPreloadingEnabled;
    }

    // True if the queue order is sequenced for fades/transitions
    // using track duration and BPM.
    bool isTransitionAware() const {
        return m_transitionAware;
    }

    // Current bot settings, loaded from and persisted to the config.
    mixxx::AutoRecommendationWeights weights() const;
    int queueSize() const;

    // Runs one top-up cycle: scores the library against the given
    // reference track and appends the best matches to the AutoDJ
    // queue. Returns the number of appended tracks.
    int topUpQueue(const TrackPointer& pReferenceTrack);

    // Connects the bot to the deck play controls. Called by the main
    // window once the player manager has created all decks.
    void bindDeckPlayControls(int numDecks);

  public slots:
    void setEnabled(bool enabled);
    void setDeckPreloadingEnabled(bool enabled);
    void setTransitionAware(bool transitionAware);
    void setWeights(const mixxx::AutoRecommendationWeights& weights);
    void setQueueSize(int queueSize);
    // Tops up the AutoDJ queue whenever the track of the currently
    // playing deck changes.
    void slotCurrentPlayingTrackChanged(TrackPointer pTrack);
    // Preloads the top recommendation into an idle deck when the deck
    // with the given index stops playing.
    void slotDeckPlayChanged(int deckIndex, double value);

  signals:
    void enabledChanged(bool enabled);
    void deckPreloadingEnabledChanged(bool enabled);
    void transitionAwareChanged(bool transitionAware);
    void weightsChanged(const mixxx::AutoRecommendationWeights& weights);
    void queueSizeChanged(int queueSize);
    // Emitted after each top-up with the number of appended tracks.
    void recommended(int count);
    // Emitted whenever a track has been preloaded into an idle deck.
    void trackPreloaded(const QString& deckGroup, const QString& trackTitle);

  private:
    QSet<TrackId> queuedTrackIds() const;
    QSet<TrackId> loadedTrackIds() const;
    TrackPointer bestRecommendation(const TrackPointer& pReferenceTrack);
    void preloadIntoIdleDeck(
            const TrackPointer& pReferenceTrack,
            int fallbackDeckIndex);

    TrackCollectionManager* const m_pTrackCollectionManager;
    PlayerManager* const m_pPlayerManager;
    const UserSettingsPointer m_pConfig;

    bool m_enabled;
    bool m_deckPreloadingEnabled;
    bool m_transitionAware;
    mixxx::AutoRecommendationWeights m_weights;
    int m_queueSize;

    // Remembers whether each deck was playing before the most recent
    // play-control change, to detect playing -> stopped transitions.
    // The play-control watchers themselves are owned by the Qt parent
    // child tree and do not need to be stored.
    QList<bool> m_deckWasPlaying;
};
