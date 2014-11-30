#ifndef AUTODJPROCESSOR_H
#define AUTODJPROCESSOR_H

#include <QObject>
#include <QString>
#include <QModelIndexList>

#include "util.h"
#include "trackinfoobject.h"
#include "configobject.h"
#include "library/playlisttablemodel.h"
#include "engine/enginechannel.h"
#include "controlobjectslave.h"

class ControlPushButton;
class TrackCollection;
class PlayerManagerInterface;
class BaseTrackPlayer;

class DeckAttributes : public QObject {
    Q_OBJECT
  public:
    DeckAttributes(int index,
                   BaseTrackPlayer* pPlayer,
                   EngineChannel::ChannelOrientation orientation);
    virtual ~DeckAttributes();

    bool isLeft() const {
        return m_orientation == EngineChannel::LEFT;
    }

    bool isRight() const {
        return m_orientation == EngineChannel::RIGHT;
    }

    bool isPlaying() const {
        return m_play.toBool();
    }

    void stop() {
        m_play.set(0.0);
    }

    void play() {
        m_play.set(1.0);
    }

    double playPosition() const {
        return m_playPos.get();
    }

    void setPlayPosition(double playpos) {
        m_playPos.set(playpos);
    }

    bool isRepeat() const {
        return m_repeat.toBool();
    }

    void setRepeat(bool enabled) {
        m_repeat.set(enabled ? 1.0 : 0.0);
    }

    TrackPointer getLoadedTrack() const;

  signals:
    void playChanged(DeckAttributes* deck, bool playing);
    void playPositionChanged(DeckAttributes* deck, double playPosition);
    void trackLoaded(DeckAttributes* deck, TrackPointer pTrack);
    void trackLoadFailed(DeckAttributes* deck, TrackPointer pTrack);
    void trackUnloaded(DeckAttributes* deck, TrackPointer pTrack);

  private slots:
    void slotPlayPosChanged(double v);
    void slotPlayChanged(double v);
    void slotTrackLoaded(TrackPointer pTrack);
    void slotTrackLoadFailed(TrackPointer pTrack);
    void slotTrackUnloaded(TrackPointer pTrack);

  public:
    int index;
    QString group;
    double posThreshold;
    double fadeDuration;

  private:
    EngineChannel::ChannelOrientation m_orientation;
    ControlObjectSlave m_playPos;
    ControlObjectSlave m_play;
    ControlObjectSlave m_repeat;
    BaseTrackPlayer* m_pPlayer;
};

class AutoDJProcessor : public QObject {
    Q_OBJECT
  public:
    enum AutoDJState {
        ADJ_IDLE = 0,
        ADJ_P1FADING,
        ADJ_P2FADING,
        ADJ_ENABLE_P1LOADED,
        ADJ_ENABLE_P1PLAYING,
        ADJ_DISABLED
    };

    enum AutoDJError {
        ADJ_OK = 0,
        ADJ_IS_INACTIVE,
        ADJ_QUEUE_EMPTY,
        ADJ_BOTH_DECKS_PLAYING
    };

    AutoDJProcessor(QObject* pParent,
                    ConfigObject<ConfigValue>* pConfig,
                    PlayerManagerInterface* pPlayerManager,
                    int iAutoDJPlaylistId,
                    TrackCollection* pCollection);
    virtual ~AutoDJProcessor();

    AutoDJState getState() const {
        return m_eState;
    }

    int getTransitionTime() const {
        return m_iTransitionTime;
    }

    PlaylistTableModel* getTableModel() const {
        return m_pAutoDJTableModel;
    }

  public slots:
    void setTransitionTime(int seconds);

    AutoDJError shufflePlaylist(const QModelIndexList& selectedIndices);
    AutoDJError skipNext();
    AutoDJError fadeNow();
    AutoDJError toggleAutoDJ(bool enable);

  signals:
    virtual void loadTrackToPlayer(TrackPointer pTrack, QString group,
                                   bool play);
    virtual void transitionTimeChanged(int time);
    virtual void autoDJStateChanged(AutoDJProcessor::AutoDJState state);

  private slots:
    void playerPositionChanged(DeckAttributes* pDeck, double position);
    void playerPlayChanged(DeckAttributes* pDeck, bool playing);
    void playerTrackLoaded(DeckAttributes* pDeck, TrackPointer pTrack);
    void playerTrackLoadFailed(DeckAttributes* pDeck, TrackPointer pTrack);
    void playerTrackUnloaded(DeckAttributes* pDeck, TrackPointer pTrack);

    void controlEnable(double value);
    void controlFadeNow(double value);
    void controlShuffle(double value);
    void controlSkipNext(double value);

  private:
    // Gets or sets the crossfader position while normalizing it so that -1 is
    // all the way mixed to the left side and 1 is all the way mixed to the
    // right side. (prevents AutoDJ logic from having to check for hamster mode
    // every time)
    double getCrossfader() const;
    void setCrossfader(double value, bool right);

    TrackPointer getNextTrackFromQueue();
    bool loadNextTrackFromQueue(const DeckAttributes& pDeck, bool play = false);
    void calculateFadeThresholds(DeckAttributes* pAttributes);

    // Removes the track loaded to the player group from the top of the AutoDJ
    // queue if it is present.
    bool removeLoadedTrackFromTopOfQueue(const DeckAttributes& deck);

    // Removes the provided track from the top of the AutoDJ queue if it is
    // present.
    bool removeTrackFromTopOfQueue(TrackPointer pTrack);

    ConfigObject<ConfigValue>* m_pConfig;
    PlayerManagerInterface* m_pPlayerManager;
    PlaylistTableModel* m_pAutoDJTableModel;

    AutoDJState m_eState;
    int m_iTransitionTime;

    QList<DeckAttributes*> m_decks;

    ControlObjectSlave* m_pCOCrossfader;
    ControlObjectSlave* m_pCOCrossfaderReverse;

    ControlPushButton* m_pSkipNext;
    ControlPushButton* m_pFadeNow;
    ControlPushButton* m_pShufflePlaylist;
    ControlPushButton* m_pEnabledAutoDJ;

    DISALLOW_COPY_AND_ASSIGN(AutoDJProcessor);
};

#endif /* AUTODJPROCESSOR_H */
