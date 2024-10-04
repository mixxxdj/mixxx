#pragma once

#include <QObject>
#include <QString>

#include "audio/frame.h"
#include "control/controlproxy.h"
#include "library/autodj/track/deckattributes.h"
#include "library/autodj/track/trackattributes.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"
#include "track/trackid.h"
#include "util/class.h"
#include "util/duration.h"

class ControlPushButton;
class TrackCollectionManager;
class PlayerManagerInterface;
class PlaylistTableModel;
typedef QList<QModelIndex> QModelIndexList;

class AutoDJProcessor : public QObject {
    Q_OBJECT
  public:
    enum AutoDJState {
        ADJ_IDLE = 0,
        ADJ_LEFT_FADING,
        ADJ_RIGHT_FADING,
        ADJ_ENABLE_P1LOADED,
        ADJ_ENABLE_P1PLAYING,
        ADJ_DISABLED
    };

    enum AutoDJError {
        ADJ_OK = 0,
        ADJ_IS_INACTIVE,
        ADJ_QUEUE_EMPTY,
        ADJ_BOTH_DECKS_PLAYING,
        ADJ_DECKS_3_4_PLAYING,
        ADJ_NOT_TWO_DECKS
    };

    enum class TransitionMode {
        FullIntroOutro,
        FadeAtOutroStart,
        FixedFullTrack,
        FixedSkipSilence
    };

    AutoDJProcessor(QObject* pParent,
                    UserSettingsPointer pConfig,
                    PlayerManagerInterface* pPlayerManager,
                    TrackCollectionManager* pTrackCollectionManager,
                    int iAutoDJPlaylistId);
    virtual ~AutoDJProcessor();

    AutoDJState getState() const {
        return m_eState;
    }

    double getTransitionTime() const {
        return m_transitionTime;
    }

    TransitionMode getTransitionMode() const {
        return m_transitionMode;
    }

    PlaylistTableModel* getTableModel() const {
        return m_pAutoDJTableModel;
    }

    /// Gets the total remaining duration of tracks in the AutoDJ playlist,
    /// excluding the track that is currently playing already.
    mixxx::Duration getQueueDuration() const {
        return m_queueDuration;
    }

    /// Gets the number of tracks remaining in the Auto DJ queue.
    int getQueueTrackCount() const;

    bool nextTrackLoaded();

    void setTransitionTime(int seconds);

    void setTransitionMode(TransitionMode newMode);

    AutoDJError shufflePlaylist(const QModelIndexList& selectedIndices);
    AutoDJError skipNext();
    void fadeNow();
    AutoDJError toggleAutoDJ(bool enable);

  signals:
    void loadTrackToPlayer(TrackPointer pTrack, const QString& group, bool play);
    void autoDJStateChanged(AutoDJProcessor::AutoDJState state);
    void autoDJError(AutoDJProcessor::AutoDJError error);
    void queueDurationChanged(int numTracks, mixxx::Duration duration);
    void transitionTimeChanged(int time);
    void randomTrackRequested(int tracksToAdd);

  private slots:
    void crossfaderChanged(double value);
    void playerPositionChanged(DeckAttributes* pDeck, double position);
    void playerPlayChanged(DeckAttributes* pDeck, bool playing);
    void playerIntroStartChanged(DeckAttributes* pDeck, double position);
    void playerIntroEndChanged(DeckAttributes* pDeck, double position);
    void playerOutroStartChanged(DeckAttributes* pDeck, double position);
    void playerOutroEndChanged(DeckAttributes* pDeck, double position);
    void playerTrackLoaded(DeckAttributes* pDeck, TrackPointer pTrack);
    void playerLoadingTrack(DeckAttributes* pDeck, TrackPointer pNewTrack, TrackPointer pOldTrack);
    void playerEmpty(DeckAttributes* pDeck);
    void playerRateChanged(DeckAttributes* pDeck);
    void playlistFirstTrackChanged();

    void playlistTracksChanged();
    void tracksChanged(const QSet<TrackId>& tracks);
    void multipleTracksChanged();
    void updateQueueDuration();

    void controlEnableChangeRequest(double value);
    void controlFadeNow(double value);
    void controlShuffle(double value);
    void controlSkipNext(double value);
    void controlAddRandomTrack(double value);

  protected:
    // The following virtual signal wrappers are used for testing
    virtual void emitLoadTrackToPlayer(TrackPointer pTrack, const QString& group, bool play) {
        emit loadTrackToPlayer(pTrack, group, play);
    }
    virtual void emitAutoDJStateChanged(AutoDJProcessor::AutoDJState state) {
        emit autoDJStateChanged(state);
    }

  private:
    // Gets or sets the crossfader position while normalizing it so that -1 is
    // all the way mixed to the left side and 1 is all the way mixed to the
    // right side. (prevents AutoDJ logic from having to check for hamster mode
    // every time)
    double getCrossfader() const;
    void setCrossfader(double value);

    // Following functions return seconds computed from samples or -1 if
    // track in deck has invalid sample rate (<= 0)
    double getIntroStartSecond(const TrackOrDeckAttributes& track);
    double getIntroEndSecond(const TrackOrDeckAttributes& track);
    double getOutroStartSecond(const TrackOrDeckAttributes& track);
    double getOutroEndSecond(const TrackOrDeckAttributes& track);
    double getFirstSoundSecond(const TrackOrDeckAttributes& track);
    double getLastSoundSecond(const TrackOrDeckAttributes& track);
    double getEndSecond(const TrackOrDeckAttributes& track);
    double framePositionToSeconds(mixxx::audio::FramePos position,
            const TrackOrDeckAttributes& track);

    TrackPointer getNextTrackFromQueue();
    bool loadNextTrackFromQueue(const DeckAttributes& pDeck, bool play = false);
    void calculateTransition(
            DeckAttributes* pFromDeck,
            DeckAttributes* pToDeck,
            bool seekToStartPoint);
    void calculateTransitionImpl(
            FadeableTrackOrDeckAttributes& pFromDeck,
            FadeableTrackOrDeckAttributes& pToDeck,
            bool seekToStartPoint);
    void useFixedFadeTime(
            FadeableTrackOrDeckAttributes& fromTrack,
            FadeableTrackOrDeckAttributes& toTrack,
            double fromDeckSecond,
            double fadeEndSecond,
            double toDeckStartSecond);
    DeckAttributes* getLeftDeck();
    DeckAttributes* getRightDeck();
    DeckAttributes* getOtherDeck(const DeckAttributes* pThisDeck);
    DeckAttributes* getFromDeck();

    /// Calculates the total remaining duration of tracks in the AutoDJ playlist,
    /// excluding the track that is currently playing already.
    mixxx::Duration calculateQueueDuration();

    // Removes the track loaded to the player group from the top of the AutoDJ
    // queue if it is present.
    bool removeLoadedTrackFromTopOfQueue(const DeckAttributes& deck);

    // Removes the provided track from the top of the AutoDJ queue if it is
    // present.
    bool removeTrackFromTopOfQueue(TrackPointer pTrack);
    void maybeFillRandomTracks();
    UserSettingsPointer m_pConfig;
    PlaylistTableModel* m_pAutoDJTableModel;

    AutoDJState m_eState;
    double m_transitionProgress;
    double m_transitionTime; // the desired value set by the user
    TransitionMode m_transitionMode;

    QList<DeckAttributes*> m_decks;

    ControlProxy* m_pCOCrossfader;
    ControlProxy* m_pCOCrossfaderReverse;

    ControlPushButton* m_pSkipNext;
    ControlPushButton* m_pAddRandomTrack;
    ControlPushButton* m_pFadeNow;
    ControlPushButton* m_pShufflePlaylist;
    ControlPushButton* m_pEnabledAutoDJ;

    ControlObject* m_pQueueRemainingTracks;
    ControlObject* m_pQueueRemainingDuration;
    mixxx::Duration m_queueDuration;

    DISALLOW_COPY_AND_ASSIGN(AutoDJProcessor);
};
