#ifndef DLGAUTODJ_H
#define DLGAUTODJ_H

#include <QItemSelection>
#include "ui_dlgautodj.h"
#include "configobject.h"
#include "controlpushbutton.h"
#include "trackinfoobject.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"
#include "library/dao/playlistdao.h"
#include "mixxxkeyboard.h"

class PlaylistTableModel;
class WTrackTableView;
class AnalyserQueue;
class QSqlTableModel;
class ControlObjectThread;
class ControlObjectSlave;

class DlgAutoDJ : public QWidget, public Ui::DlgAutoDJ, public LibraryView {
    Q_OBJECT
  public:
    DlgAutoDJ(QWidget *parent, ConfigObject<ConfigValue>* pConfig,
              TrackCollection* pTrackCollection, MixxxKeyboard* pKeyboard);
    virtual ~DlgAutoDJ();

    void onShow();
    void loadSelectedTrack();
    void onSearch(const QString& text);
    void loadSelectedTrackToGroup(QString group, bool play);
    void moveSelection(int delta);

  public slots:
    void shufflePlaylistButton(bool buttonChecked);
    void skipNextButton(bool buttonChecked);
    void fadeNowButton(bool buttonChecked);
    void toggleAutoDJButton(bool enable);
    void enableAutoDJCo(double value);
    void shufflePlaylist(double value);
    void skipNext(double value);
    void fadeNow(double value);
    void player1PositionChanged(double value);
    void player2PositionChanged(double value);
    void player1PlayChanged(double value);
    void player2PlayChanged(double value);
    void transitionValueChanged(int value);
    void enableRandomButton(bool enabled);

  signals:
    void addRandomButton(bool buttonChecked);
    void loadTrack(TrackPointer tio);
    void loadTrackToPlayer(TrackPointer tio, QString group, bool);
    void trackSelected(TrackPointer pTrack);

  private:
    enum ADJstates {
        ADJ_IDLE = 0,
        ADJ_P1FADING,
        ADJ_P2FADING,
        ADJ_ENABLE_P1LOADED,
        ADJ_ENABLE_P1PLAYING,
        ADJ_DISABLED
    };

    // Gets or sets the crossfader position while normalizing it so that -1 is
    // all the way mixed to the left side and 1 is all the way mixed to the
    // right side. (prevents AutoDJ logic from having to check for hamster mode
    // every time)
    double getCrossfader() const;
    void setCrossfader(double value, bool right);

    TrackPointer getNextTrackFromQueue();
    bool loadNextTrackFromQueue();
    bool removePlayingTrackFromQueue(QString group);

    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    WTrackTableView* m_pTrackTableView;
    PlaylistTableModel* m_pAutoDJTableModel;

    bool m_bFadeNow;
    enum ADJstates m_eState;
    float m_posThreshold1;
    float m_posThreshold2;
    float m_fadeDuration1;
    float m_fadeDuration2;
    int m_backUpTransition;
    ControlObjectThread* m_pCOPlayPos1;
    ControlObjectThread* m_pCOPlayPos2;
    ControlObjectThread* m_pCOPlay1;
    ControlObjectThread* m_pCOPlay2;
    ControlObjectSlave* m_pCORepeat1;
    ControlObjectSlave* m_pCORepeat2;
    ControlObjectSlave* m_pCOCrossfader;
    ControlObjectSlave* m_pCOCrossfaderReverse;
    ControlObjectThread* m_pCOTSkipNext;
    ControlObjectThread* m_pCOTFadeNow;
    ControlObjectThread* m_pCOTShufflePlaylist;
    ControlObjectThread* m_pCOTEnabledAutoDJ;
    ControlPushButton* m_pCOSkipNext;
    ControlPushButton* m_pCOFadeNow;
    ControlPushButton* m_pCOShufflePlaylist;
    ControlPushButton* m_pCOEnabledAutoDJ;
};

#endif //DLGAUTODJ_H
