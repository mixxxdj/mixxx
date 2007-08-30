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

#include "proxymodel.h"

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
class WTrackTableView : public QTableView
{
    Q_OBJECT

public:
    WTrackTableView(QWidget *parent=0);
    ~WTrackTableView();
	/**Graphically set up WTrackTableView**/
    void setup(QDomNode node);
    void sortColumn(int col, bool ascending, bool);
	/**Sets current model to WTrackTableView**/
	void setSearchSource(WTrackTableModel *pSearchSourceModel);
	/**Right click menu**/
	void contextMenuEvent(QContextMenuEvent * event);
	/**Sets Track pointer**/
	void setTrack(Track *pTrack);
	/**Used to filter items in table with a given search string**/
	SortFilterProxyModel *m_pSearchFilter;
	/**Current WTrackTableModel**/
	WTrackTableModel *m_pTable;
private:
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
private slots:
	/**sends track to Playqueue*/
	void slotSendToPlayqueue();
	/** Load the given track in player 1 */
    void slotLoadPlayer1();
    /** Load the given track in player 2 */
    void slotLoadPlayer2();
	/** Remove selected track from active playlist */
    void slotRemoveFromPlaylist();
protected slots:
    void slotMouseDoubleClicked(const QModelIndex &);
    //Q3DragObject *dragObject();
protected:
   DlgBpmTap *bpmTapDlg;
};
#endif
