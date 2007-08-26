#include "wtracktableview.h"
#include "wtracktablemodel.h"
#include "wwidget.h"
#include "wskincolor.h"
#include "trackinfoobject.h"
#include "trackcollection.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "track.h"

#include <qfont.h>
#include <qcolor.h>
#include <q3dragobject.h>
#include <qstring.h>
#include <qmime.h>

//Added by qt3to4:
#include <Q3Frame>
#include <qwidget.h>
#include <QTableView>
#include <QMenu>


#ifdef __EXPERIMENTAL_BPM__
  #include "dlgbpmtap.h"
#endif

/*Constructor, sets up attributes for WTrackTableView*/
WTrackTableView::WTrackTableView(QWidget *parent) : QTableView(parent)
{
	m_pTable = new WTrackTableModel(this);

	//setup properties for table
	setSelectionBehavior(SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
	viewport()->setAcceptDrops(true);
	setDragEnabled(true);
	setWordWrap(false);
	setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
    setShowGrid(false);
	verticalHeader()->hide();
	verticalHeader()->setDefaultSectionSize(20);
	setFrameStyle(Q3Frame::NoFrame);
	setCornerButtonEnabled ( false );
	setSortingEnabled ( true );

	createActions();
	m_pSearchFilter = new SortFilterProxyModel(parent);
    /*
	//setFocusStyle(Q3Table::FollowStyle);

    // Allow table reordering
    //setRowMovingEnabled(true);
	*/
    #ifdef __EXPERIMENTAL_BPM__
      connect(this, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotMouseDoubleClicked(const QModelIndex &)));
      bpmTapDlg = 0;
    #endif
}
WTrackTableView::~WTrackTableView()
{
    #ifdef __EXPERIMENTAL_BPM__
    if(bpmTapDlg)
        delete bpmTapDlg;
    #endif
}

/*Graphically sets up playlist and library directory*/
void WTrackTableView::setup(QDomNode node)
{
	// Position
	if (!WWidget::selectNode(node, "Pos").isNull())
    {
		QString pos = WWidget::selectNodeQString(node, "Pos");
		int x = pos.left(pos.find(",")).toInt();
		int y = pos.mid(pos.find(",")+1).toInt();
		move(x,y);
	}

    // Size
	if (!WWidget::selectNode(node, "Size").isNull())
    {
		QString size = WWidget::selectNodeQString(node, "Size");
		int x = size.left(size.find(",")).toInt();
		int y = size.mid(size.find(",")+1).toInt();
		setFixedSize(x,y);
    }
    // Foreground color
	QColor fgc(0,0,0);
    if (!WWidget::selectNode(node, "FgColor").isNull())
    {
        fgc.setNamedColor(WWidget::selectNodeQString(node, "FgColor"));
		m_pTable->setForegroundColor(fgc);
    }
	
	
    // Row colors
    if (!WWidget::selectNode(node, "BgColorRowEven").isNull())
    {
        QColor r1;
        r1.setNamedColor(WWidget::selectNodeQString(node, "BgColorRowEven"));
        QColor r2;
        r2.setNamedColor(WWidget::selectNodeQString(node, "BgColorRowUneven"));
		setAlternatingRowColors ( true );
		QPalette Rowpalette = palette();
		Rowpalette.setColor(QPalette::Base, r1);
		Rowpalette.setColor(QPalette::AlternateBase, r2);
		setPalette(Rowpalette);
    }
	 
	
    // BPM confidence colors
	/*
    if (!WWidget::selectNode(node, "BgColorBpmNoConfirm").isNull())
    {
        QColor c1;
        c1.setNamedColor(WWidget::selectNodeQString(node, "BgColorBpmNoConfirm"));
        QColor c2;
        c2.setNamedColor(WWidget::selectNodeQString(node, "BgColorBpmConfirm"));
		m_pTable->setBpmColor(c1,c2);
	}*/
	/*
	typedef QMap<int,QString> ColMap;
	ColMap ColumnMap;
	ColumnMap[COL_SCORE] = "ColWidthScore";	
	ColumnMap[COL_TITLE] = "ColWidthTitle";
	ColumnMap[COL_ARTIST] = "ColWidthArtist";
	ColumnMap[COL_COMMENT] = "ColWidthComment";
	ColumnMap[COL_TYPE] = "ColWidthType";
	ColumnMap[COL_DURATION] = "ColWidthDuration";
	ColumnMap[COL_BITRATE] = "ColWidthBitrate";
	ColumnMap[COL_BPM] = "ColWidthBpm";

	QMapIterator<int,QString> i(ColumnMap);
	while (i.hasNext()) 
	{
		i.next();
		if (!WWidget::selectNode(node, i.value()).isNull() && WWidget::selectNodeQString(node, i.value()).toInt() != columnWidth(i.key())) 
		{
			qDebug("Correcting Column Width from %i to %i",columnWidth(i.key()),WWidget::selectNodeQString(node, i.value()).toInt());
			setColumnWidth(i.key(),WWidget::selectNodeQString(node, i.value()).toInt());
		}
	}*/
}

/*sorts a given column*/
void WTrackTableView::sortColumn(int col, bool ascending, bool)
{
    //Q3Table::sortColumn(col,ascending,true);
}

/*checks for Mouse action*/

void WTrackTableView :: slotMouseDoubleClicked(const QModelIndex &)
{
	/*
    #ifdef __EXPERIMENTAL_BPM__
    if(col == COL_BPM && button==Qt::LeftButton)
    {
        WTrackTableItem *p = (WTrackTableItem *)item(row,col);
    
        if(p)
        {
            TrackInfoObject *pTrackInfoObject = p->getTrackInfoObject();
            if(pTrackInfoObject)
            {
                if(bpmTapDlg)
                    delete bpmTapDlg;

                bpmTapDlg = new DlgBPMTap(NULL, pTrackInfoObject);
                bpmTapDlg->show();
            }
        }
    }
    #endif*/
}

/*enables contents to be dragable
Q3DragObject *WTrackTableView::dragObject()
{
	
    WTrackTableItem *p = (WTrackTableItem *)item(currentRow(),currentColumn());
    TrackInfoObject *pTrackInfoObject = p->getTrackInfoObject();

    Q3UriDrag *ud = new Q3UriDrag(this);
    ud->setFileNames(QStringList(pTrackInfoObject->getLocation()));

    return ud;
}*/
void WTrackTableView :: setSearchSource(WTrackTableModel *pSearchSourceModel)
{
	m_pTable = pSearchSourceModel;
	m_pSearchFilter->setSourceModel(m_pTable);
	setModel(m_pSearchFilter);
}
void WTrackTableView :: contextMenuEvent(QContextMenuEvent * event)
{
	index = indexAt(event->pos());
	m_pTrackInfoObject = m_pTable->m_pTrackPlaylist->getTrackAt(index.row());
	if(index.isValid())
	{
		QMenu menu(this);
		menu.addAction(PlayQueueAct);
		if (ControlObject::getControl(ConfigKey("[Channel1]","play"))->get()!=1.)
			menu.addAction(Player1Act);
		if (ControlObject::getControl(ConfigKey("[Channel2]","play"))->get()!=1.)
			menu.addAction(Player2Act);
		menu.addAction(RemoveAct);
		menu.exec(event->globalPos());
	}
}
void WTrackTableView :: createActions()
{
	PlayQueueAct = new QAction(tr("Play Queue"),this);
	connect(PlayQueueAct, SIGNAL(triggered()), this, SLOT(slotSendToPlayqueue()));

	Player1Act = new QAction(tr("Player 1"),this);
	connect(Player1Act, SIGNAL(triggered()), this, SLOT(slotLoadPlayer1()));

	Player2Act = new QAction(tr("Player 2"),this);
	connect(Player2Act, SIGNAL(triggered()), this, SLOT(slotLoadPlayer2()));

	RemoveAct = new QAction(tr("Remove"),this);
	connect(RemoveAct, SIGNAL(triggered()), this, SLOT(slotRemoveFromPlaylist()));
}
void WTrackTableView :: setTrack(Track *pTrack)
{
	m_pTrack = pTrack;
}
void WTrackTableView :: slotLoadPlayer1()
{
	m_pTrack->slotLoadPlayer1(m_pTrackInfoObject);
}
void WTrackTableView :: slotLoadPlayer2()
{
	m_pTrack->slotLoadPlayer2(m_pTrackInfoObject);
}
void WTrackTableView :: slotSendToPlayqueue()
{
	m_pTrack->slotSendToPlayqueue(m_pTrackInfoObject);
}
void WTrackTableView :: slotRemoveFromPlaylist()
{
	m_pTable->removeRow(index.row(),index);
}
