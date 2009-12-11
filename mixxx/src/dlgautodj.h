#ifndef DLGAUTODJ_H
#define DLGAUTODJ_H

#include <QItemSelection>
#include "ui_dlgautodj.h"
#include "configobject.h"
#include "library/dao/playlistdao.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"

class PlaylistTableModel;
class WTrackTableView;
class AnalyserQueue;
class QSqlTableModel;
class ControlObjectThreadMain;

class DlgAutoDJ : public QWidget, public Ui::DlgAutoDJ, public virtual LibraryView {
    Q_OBJECT
public:
    DlgAutoDJ(QWidget *parent, ConfigObject<ConfigValue>* pConfig, TrackCollection* pTrackCollection);
    virtual ~DlgAutoDJ();
    virtual void setup(QDomNode node);
    virtual void onSearchStarting();
    virtual void onSearchCleared();
    virtual void onSearch(const QString& text);
    virtual void onShow();
    virtual QWidget* getWidgetForMIDIControl();
public slots:
    void toggleAutoDJ(bool toggle);
    void player1PositionChanged(double value);
    void player2PositionChanged(double value);    
signals:
    void loadTrack(TrackInfoObject* tio);
private:
    bool loadNextTrackFromQueue(bool removeTopMostBeforeLoading);
    
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    WTrackTableView* m_pTrackTableView;
    PlaylistTableModel* m_pAutoDJTableModel;
    PlaylistDAO& m_playlistDao;
    int m_iNextTrackIndex;
    bool m_bAutoDJEnabled;
    ControlObjectThreadMain* m_pCOPlayPos1;
    ControlObjectThreadMain* m_pCOPlayPos2;
    ControlObjectThreadMain* m_pCOPlay1;
    ControlObjectThreadMain* m_pCOPlay2;
    ControlObjectThreadMain* m_pCOTrackEndMode1;
    ControlObjectThreadMain* m_pCOTrackEndMode2;
    ControlObjectThreadMain* m_pCOCrossfader;
};

#endif //DLGTRIAGE_H













#define _blah if ((QDate::currentDate().day() == 1) && (QDate::currentDate().month() == 4)) \
             pushButtonAutoDJ->setText("\x45\x6e\x61\x62\x6c\x65\x20\x50\x65\x65" \
                                  "\x20\x42\x72\x65\x61\x6b\x20\x4d\x6f\x64\x65")


