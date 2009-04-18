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

// Defines what mode the track table is in.
enum table_mode_t
{
    TABLE_MODE_LIBRARY =      0,
    TABLE_MODE_PLAYQUEUE,
    TABLE_MODE_BROWSE,
    TABLE_MODE_PLAYLISTS,
    TABLE_MODE_PROMO,
    TABLE_MODE_IPOD
};


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

    /** Sets the current mode for the WTrackTableView */
    void setTableMode(table_mode_t table_mode);

    /** Gets the current mode for the WTrackTableView */
    table_mode_t getTableMode();

    /** Returns the QDirModel **/
    QDirModel* getDirModel();

    /** Gets the next track from the table while in "Browse mode" */
    QString getNextTrackBrowseMode(TrackInfoObject* current);
    QString getPrevTrackBrowseMode(TrackInfoObject* current);

    /**Right click menu**/
    void contextMenuEvent(QContextMenuEvent * event);
    /**Sets Track pointer**/
    void setTrack(Track *pTrack);
    /**updates the list of playlists in the right-click->sendto playlist menu**/
    void updatePlaylistActions();

    /**Used to filter items in table with a given search string**/
    SortFilterProxyModel *m_pSearchFilter;
    /* filter files in browse mode */
    WTrackTableFilter *m_pDirFilter;

    /**Current WTrackTableModel**/
    WTrackTableModel *m_pTable;

    /* Helper functions to move the row selection */
    void selectNext();
    void selectPrevious();

    /** return the current table filter string */
    QString getFilterString();

private:
    /** Config object*/
    ConfigObject<ConfigValue> *m_pConfig;
    /**Pointer to Track object**/
    Track *m_pTrack;
    /**Pointer to selected TrackInfoObjects in model**/
    QList<TrackInfoObject*> m_selectedTrackInfoObjects;

    //Used for right-click operations
    /**Send to Play Queue Action**/
    QAction *PlayQueueAct;
    /**Send to Player 1 Action**/
    QAction *Player1Act;
    /**Send to Player 2 Action**/
    QAction *Player2Act;
    /**Remove from Table Action**/
    QAction *RemoveAct;
    /**Shows track editor/BPM tap**/
    QAction *PropertiesAct;
    /**Copies a promo track to the library**/
    QAction *CopyToLibraryAct;
    /**Visits the website associated with a track*/
    QAction *VisitWebsiteAct;
    /**Send to each playlist Action**/
    QList<QAction*> PlaylistActs;
    /**Rename playlist Action**/
    QAction *RenamePlaylistAct;
    /**creates all actions and connects them to repective slots**/
    void createActions();

    QDrag *getDragObject(QMouseEvent *event);

private slots:
    /** Sends track(s) to Playqueue*/
    void slotSendToPlayqueue();
    /** Send track(s) to a given playlist*/
    void slotSendToPlaylist();
    /** Load the given track in player 1 */
    void slotLoadPlayer1();
    /** Load the given track in player 2 */
    void slotLoadPlayer2();
    /** Remove selected track from the active playlist or whatever's in the tableview */
    void slotRemove();
    /**Show the track editor/bpm tap dialog */
    void slotShowBPMTapDlg(TrackInfoObject* pTrackInfoObject);
    void slotShowBPMTapDlg();
    /**Show playlist rename dialog */
    void slotShowPlaylistRename();
    /**Copies a (promo) track to the user's library directory*/
    void slotCopyToLibrary();
    /**Opens a web browser to the website associated with a track*/
    void slotVisitWebsite();

public slots:
    /** Set the search filter **/
    void slotFilter(const QString &);
    void slotFilter();
	void repaintEverything();

protected slots:
    void sortByColumn(int col);
    void slotMouseDoubleClicked(const QModelIndex &);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void dragMoveEvent(QDragMoveEvent * event);
    void resizeEvent(QResizeEvent *event);
    DlgBpmTap *bpmTapDlg;
    QModelIndex m_dirindex;
    QList<QString> m_selectedDirTrackNames;        //Names of the selected tracks when in browse mode.
    QList<TrackPlaylist*> m_selectedPlaylists;     //The playlists that were selected when right-clicking in playlists mode.
    /* directory model*/
    QDirModel *m_pDirModel;
    WPlaylistListModel *m_pPlaylistListModel;
    QModelIndexList m_selectedIndices;
    QString m_filterString;
    QList<table_mode_t> m_dndTableModeBlacklist;
    table_mode_t m_iTableMode;                      /** Determines what "mode" the table view is in */

};
#endif
