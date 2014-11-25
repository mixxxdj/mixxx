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
    // Gets or sets the crossfader position while normalizing it so that -1 is
    // all the way mixed to the left side and 1 is all the way mixed to the
    // right side. (prevents AutoDJ logic from having to check for hamster mode
    // every time)
    double getCrossfader() const;
    void setCrossfader(double value, bool right);

    TrackPointer getNextTrackFromQueue();
    bool loadNextTrackFromQueue();

    // Removes the track loaded to the player group from the top of the AutoDJ
    // queue if it is present.
    bool removeLoadedTrackFromTopOfQueue(const QString& group);

    ConfigObject<ConfigValue>* m_pConfig;
    PlaylistTableModel* m_pAutoDJTableModel;

    AutoDJState m_eState;
    double m_posThreshold[2];
    double m_fadeDuration[2];
    int m_iTransitionTime;
    int m_iBackupTransitionTime;

    QList<ControlObjectThread*> m_pCOPlayPos;
    QSignalMapper m_playPosMapper;
    QList<ControlObjectThread*> m_pCOPlay;
    QSignalMapper m_playMapper;
    QList<ControlObjectSlave*> m_pCORepeat;
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
