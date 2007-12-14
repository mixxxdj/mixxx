#ifndef WTRACKTABLEVIEW_H
#define WTRACKTABLEVIEW_H

#include <qdom.h>
#include <qevent.h>
//Q3Table: Added by qt3to4:

#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
//needed for QTableView
#include <QModelIndex>
#include <QTableView>
#include "wtracktablefilter.h"
#include "wplaylistlistmodel.h"
#include "proxymodel.h"
#include "configobject.h"

class QWidget;

/**
  *@author Tue & Ken Haste Andersen
  */
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class QDomNode;
class QDomElement;
class QWidget;
class TrackInfoObject;
class DlgBpmTap;
//needed for QTableView
class QTableView;
class WTrackTableModel;
class Track;
class QAction;
class QMenu;
class QDirModel;
class QModelIndex;
class QDrag;
class QMouseEvent;

class WTrackTableView : public QTableView
{
    Q_OBJECT

public:
    WTrackTableView(QWidget *parent=0, ConfigObject<ConfigValue> *pConfig=0);
    ~WTrackTableView();
	/**Graphically set up WTrackTableView**/
    void setup(QDomNode node);
    void sortColumn(int col, bool ascending, bool);

    /**Sets current model to WTrackTableView**/
    void setSearchSource(WTrackTableModel *pSearchSourceModel);
    void setDirModel();
    void setPlaylistListModel(WPlaylistListModel *model);
    
    /** Returns the QDirModel **/
    QDirModel* getDirModel();
    
    /** Gets the next track from the table while in "Browse mode" */
    QString getNextTrackBrowseMode(TrackInfoObject* current);
    QString getPrevTrackBrowseMode(TrackInfoObject* current);
    
    /**Right click menu**/
    void contextMenuEvent(QContextMenuEvent * event);
    /**Sets Track pointer**/
    void setTrack(Track *pTrack);
    /**Used to filter items in table with a given search string**/
    SortFilterProxyModel *m_pSearchFilter;
    /* filter files in browse mode */
    WTrackTableFilter *m_pDirFilter;

    /**Current WTrackTableModel**/
    WTrackTableModel *m_pTable;

    /* Helper functions to move the row selection */
    void selectNext();
    void selectPrevious();

private:
    /** Config object*/
    ConfigObject<ConfigValue> *m_pConfig;
    /**Pointer to Track object**/
    Track *m_pTrack;
    /**Pointer to selected TrackInfoObject in model**/
    TrackInfoObject *m_pTrackInfoObject;
    /**Current index of selected TrackInfoObject**/
    QModelIndex index;

    //Used for right-click operations
    /**Send to Play Queue Action**/
    QAction *PlayQueueAct;
    /**Send to Player 1 Action**/
    QAction *Player1Act;
    /**Send to Player 2 Action**/
    QAction *Player2Act;
    /**Remove from Table Action**/
    QAction *RemoveAct;
    /**creates all actions and connects them to repective slots**/
    void createActions();

    QDrag *getDragObject(QMouseEvent *event);

private slots:
    /**sends track to Playqueue*/
    void slotSendToPlayqueue();
    /** Load the given track in player 1 */
    void slotLoadPlayer1();
    /** Load the given track in player 2 */
    void slotLoadPlayer2();
    /** Remove selected track from active playlist */
    void slotRemoveFromPlaylist();
public slots:
    /** Set the search filter **/
    void slotFilter(const QString &);
    void slotFilter();
	void repaintEverything();

protected slots:
    void sortByColumn(int col);
    void slotMouseDoubleClicked(const QModelIndex &);

protected:
    DlgBpmTap *bpmTapDlg;
    QModelIndex m_dirindex;
    QString m_dirTrackName; //Name of the track right-clicked when in browse mode.
    /* directory model*/
    QDirModel *m_pDirModel;
    WPlaylistListModel *m_pPlaylistListModel;
    QString m_filterString;
};
#endif
