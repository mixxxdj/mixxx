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
#include "controlobjectthread.h"
#include "controlobjectslave.h"

class ControlPushButton;
class TrackCollection;

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
    virtual void loadTrack(TrackPointer pTrack);
    virtual void transitionTimeChanged(int time);
    virtual void autoDJStateChanged(AutoDJProcessor::AutoDJState state);

  private slots:
    void playerPositionChanged(int index);
    void playerPlayChanged(int index);

    void controlEnable(double value);
    void controlFadeNow(double value);
    void controlShuffle(double value);
    void controlSkipNext(double value);

  private:
    struct DeckAttributes {
        DeckAttributes(int index,
                       const QString& group,
                       EngineChannel::ChannelOrientation orientation)
                : index(index),
                  group(group),
                  orientation(orientation),
                  pPlayPos(new ControlObjectThread(group, "playposition")),
                  pPlay(new ControlObjectThread(group, "play")),
                  pRepeat(new ControlObjectSlave(group, "repeat")),
                  posThreshold(1.0),
                  fadeDuration(0.0) {
        }

        ~DeckAttributes() {
            delete pPlayPos;
            delete pPlay;
            delete pRepeat;
        }

        bool isLeft() const {
            return orientation == EngineChannel::LEFT;
        }

        bool isRight() const {
            return orientation == EngineChannel::RIGHT;
        }

        bool isPlaying() const {
            return pPlay->get() > 0.0;
        }

        void stop() {
            pPlay->set(0.0);
        }

        void play() {
            pPlay->set(1.0);
        }

        double playPosition() const {
            return pPlayPos->get();
        }

        void setPlayPosition(double playpos) {
            pPlayPos->set(playpos);
        }

        bool isRepeat() const {
            return pRepeat->get() > 0.0;
        }

        void setRepeat(bool enabled) {
            pRepeat->set(enabled ? 1.0 : 0.0);
        }

        int index;
        QString group;
        EngineChannel::ChannelOrientation orientation;
        ControlObjectThread* pPlayPos;
        ControlObjectThread* pPlay;
        ControlObjectSlave* pRepeat;
        double posThreshold;
        double fadeDuration;
    };

    // Gets or sets the crossfader position while normalizing it so that -1 is
    // all the way mixed to the left side and 1 is all the way mixed to the
    // right side. (prevents AutoDJ logic from having to check for hamster mode
    // every time)
    double getCrossfader() const;
    void setCrossfader(double value, bool right);

    TrackPointer getNextTrackFromQueue();
    bool loadNextTrackFromQueue();
    void playerPositionChanged(DeckAttributes* pAttributes);
    void playerPlayChanged(DeckAttributes* pAttributes);

    // Removes the track loaded to the player group from the top of the AutoDJ
    // queue if it is present.
    bool removeLoadedTrackFromTopOfQueue(const QString& group);

    ConfigObject<ConfigValue>* m_pConfig;
    PlaylistTableModel* m_pAutoDJTableModel;

    AutoDJState m_eState;
    int m_iTransitionTime;
    int m_iBackupTransitionTime;

    QList<DeckAttributes*> m_decks;

    QSignalMapper m_playPosMapper;
    QSignalMapper m_playMapper;
    QSignalMapper m_repeatMapper;

    ControlObjectSlave* m_pCOCrossfader;
    ControlObjectSlave* m_pCOCrossfaderReverse;
    ControlObjectSlave* m_pNumDecks;

    ControlPushButton* m_pSkipNext;
    ControlPushButton* m_pFadeNow;
    ControlPushButton* m_pShufflePlaylist;
    ControlPushButton* m_pEnabledAutoDJ;

    DISALLOW_COPY_AND_ASSIGN(AutoDJProcessor);
};

#endif /* AUTODJPROCESSOR_H */
