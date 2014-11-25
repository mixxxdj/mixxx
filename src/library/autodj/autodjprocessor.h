#ifndef AUTODJPROCESSOR_H
#define AUTODJPROCESSOR_H

#include <QObject>
#include <QString>
#include <QModelIndexList>

#include "util.h"
#include "trackinfoobject.h"
#include "configobject.h"
#include "library/playlisttablemodel.h"

class ControlObjectThread;
class ControlPushButton;
class ControlObjectSlave;
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
    void loadTrack(TrackPointer pTrack);
    void transitionTimeChanged(int time);
    void autoDJStateChanged(AutoDJProcessor::AutoDJState state);

  private slots:
    void player1PositionChanged(double value);
    void player2PositionChanged(double value);
    void player1PlayChanged(double value);
    void player2PlayChanged(double value);

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
    bool loadNextTrackFromQueue();
    bool removePlayingTrackFromQueue(const QString& group);

    ConfigObject<ConfigValue>* m_pConfig;
    PlaylistTableModel* m_pAutoDJTableModel;

    AutoDJState m_eState;
    double m_posThreshold1;
    double m_posThreshold2;
    double m_fadeDuration1;
    double m_fadeDuration2;
    int m_iTransitionTime;
    int m_iBackupTransitionTime;

    ControlObjectThread* m_pCOPlayPos1;
    ControlObjectThread* m_pCOPlayPos2;
    ControlObjectThread* m_pCOPlay1;
    ControlObjectThread* m_pCOPlay2;
    ControlObjectSlave* m_pCORepeat1;
    ControlObjectSlave* m_pCORepeat2;
    ControlObjectSlave* m_pCOCrossfader;
    ControlObjectSlave* m_pCOCrossfaderReverse;

    ControlPushButton* m_pSkipNext;
    ControlPushButton* m_pFadeNow;
    ControlPushButton* m_pShufflePlaylist;
    ControlPushButton* m_pEnabledAutoDJ;

    DISALLOW_COPY_AND_ASSIGN(AutoDJProcessor);
};

#endif /* AUTODJPROCESSOR_H */
