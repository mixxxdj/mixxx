#ifndef DLGAUTODJ_H
#define DLGAUTODJ_H

#include <QItemSelection>
#include "ui_dlgautodj.h"
#include "configobject.h"
#include "controlpushbutton.h"
#include "trackinfoobject.h"
#include "library/dao/playlistdao.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"
#include "mixxxkeyboard.h"

class PlaylistTableModel;
class WTrackTableView;
class AnalyserQueue;
class QSqlTableModel;
class ControlObjectThreadMain;

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
    void setCrossfader(double value);

    TrackPointer getNextTrackFromQueue();
    bool loadNextTrackFromQueue();
    bool removePlayingTrackFromQueue(QString group);

    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    WTrackTableView* m_pTrackTableView;
    PlaylistTableModel* m_pAutoDJTableModel;

    // Makes our Auto DJ logic assume the next track that should be played is
    // already loaded. We need this flag to make our
    // first-track-gets-loaded-but- not-removed-from-the-queue behaviour work.
    bool m_bNextTrackAlreadyLoaded;

    bool m_bFadeNow;
    enum ADJstates m_eState;
    float m_posThreshold1;
    float m_posThreshold2;
    float m_fadeDuration1;
    float m_fadeDuration2;
    int m_backUpTransition;
    ControlObjectThreadMain* m_pCOPlayPos1;
    ControlObjectThreadMain* m_pCOPlayPos2;
    ControlObjectThreadMain* m_pCOPlay1;
    ControlObjectThreadMain* m_pCOPlay2;
    ControlObjectThreadMain* m_pCOPlay1Fb;
    ControlObjectThreadMain* m_pCOPlay2Fb;
    ControlObjectThreadMain* m_pCORepeat1;
    ControlObjectThreadMain* m_pCORepeat2;
    ControlObjectThreadMain* m_pCOCrossfader;
    ControlObjectThreadMain* m_pCOCrossfaderReverse;
    ControlObjectThreadMain* m_pCOTSkipNext;
    ControlObjectThreadMain* m_pCOTFadeNow;
    ControlObjectThreadMain* m_pCOTShufflePlaylist;
    ControlObjectThreadMain* m_pCOTEnabledAutoDJ;
    ControlPushButton* m_pCOSkipNext;
    ControlPushButton* m_pCOFadeNow;
    ControlPushButton* m_pCOShufflePlaylist;
    ControlPushButton* m_pCOEnabledAutoDJ;
};

#endif //DLGAUTODJ_H













#define _blah if ((QDate::currentDate().day() == 1) && (QDate::currentDate().month() == 4)) \
        pushButtonAutoDJ->setText("\x45\x6e\x61\x62\x6c\x65\x20\x50\x65\x65" \
                                  "\x20\x42\x72\x65\x61\x6b\x20\x4d\x6f\x64\x65")


