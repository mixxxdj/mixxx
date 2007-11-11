/* -*- mode:C++; indent-tabs-mode:t; tab-width:8; c-basic-offset:4; -*- */
#include "wtracktableview.h"
#include "wplaylistlistmodel.h"
#include "wtracktablefilter.h"
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
#include <QDirModel>


#include "dlgbpmtap.h"

/* TODO
 * remove color attribute from tracktablemodel (not used, and not at their place)
 * move playlist model instance from track to here?
 * refresh the QDirModel
 * search: create a custom proxy, filtering only the current branch in the hierarchie
 *
 */


/*Constructor, sets up attributes for WTrackTableView*/
WTrackTableView::WTrackTableView(QWidget * parent, ConfigObject<ConfigValue> * pConfig) : QTableView(parent)
{
    m_pTable = new WTrackTableModel(this);
    m_pConfig = pConfig;
    //setup properties for table
    setSelectionBehavior(SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    viewport()->setAcceptDrops(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setWordWrap(false);
    setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
    setShowGrid(false);
    horizontalHeader()->setClickable(true);
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

    connect(this, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotMouseDoubleClicked(const QModelIndex &)));

    m_pDirModel = new QDirModel;
    m_pDirModel->setFilter(QDir::AllEntries);
    m_dirindex = m_pDirModel->index(m_pConfig->getValueString(ConfigKey("[Playlist]","Directory")));

    m_pDirFilter = new WTrackTableFilter(m_dirindex);
    m_pDirFilter->setSourceModel(m_pDirModel);

    bpmTapDlg = 0;

}
WTrackTableView::~WTrackTableView()
{
    if(bpmTapDlg)
        delete bpmTapDlg;
}

/*Graphically sets up playlist and library directory*/
void WTrackTableView::setup(QDomNode node)
{
    // Position
    if (!WWidget::selectNode(node, "Pos").isNull())
    {
        QString pos = WWidget::selectNodeQString(node, "Pos");
        int x = pos.left(pos.indexOf(",")).toInt();
        int y = pos.mid(pos.indexOf(",")+1).toInt();
        move(x,y);
    }

    // Size
    if (!WWidget::selectNode(node, "Size").isNull())
    {
        QString size = WWidget::selectNodeQString(node, "Size");
        int x = size.left(size.indexOf(",")).toInt();
        int y = size.mid(size.indexOf(",")+1).toInt();
        setFixedSize(x,y);
    }
    // Foreground color
    QColor fgc(0,255,0);
    if (!WWidget::selectNode(node, "FgColor").isNull())
    {
        fgc.setNamedColor(WWidget::selectNodeQString(node, "FgColor"));
	if (m_pTable)
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
}

void WTrackTableView::sortByColumn(int col)
{
    if(horizontalHeader()->sortIndicatorSection() != col)
        return QTableView::sortByColumn(col);

    bool ascending = (horizontalHeader()->sortIndicatorOrder() == Qt::AscendingOrder);

    if (m_pTable)
    {
        switch(col)
        {
            // Score Column
        case 0:
            m_pTable->m_pTrackPlaylist->sortByScore(ascending);
            break;

            // Title
        case 1:
            m_pTable->m_pTrackPlaylist->sortByTitle(ascending);
            break;

            // Artist
        case 2:
            m_pTable->m_pTrackPlaylist->sortByArtist(ascending);
            break;

            // Type
        case 3:
            m_pTable->m_pTrackPlaylist->sortByType(ascending);
            break;

            // Duration
        case 4:
            m_pTable->m_pTrackPlaylist->sortByDuration(ascending);
            break;

            // Bitrate
        case 5:
            m_pTable->m_pTrackPlaylist->sortByBitrate(ascending);
            break;

            // BPM
        case 6:
            m_pTable->m_pTrackPlaylist->sortByBpm(ascending);
            break;

            // Comment
        case 7:
            m_pTable->m_pTrackPlaylist->sortByComment(ascending);
            break;
        }
	slotFilter();
    }

    return QTableView::sortByColumn(col);
}

/*checks for Mouse action*/

void WTrackTableView::slotMouseDoubleClicked(const QModelIndex & index)
{
    if (!m_pTable) {
        QModelIndex temp_dirindex = m_pDirFilter->mapToSource(index);
        if (m_pDirModel->isDir(temp_dirindex)) {
            m_dirindex = temp_dirindex;
            setRootIndex(index);
            m_pDirFilter->setIndex(m_dirindex);
        }
        return;
    }

    TrackInfoObject * pTrackInfoObject = m_pTable->m_pTrackPlaylist->getTrackAt(index.row());
    if(pTrackInfoObject)
    {
        if(bpmTapDlg)
            delete bpmTapDlg;

        bpmTapDlg = new DlgBpmTap(NULL, pTrackInfoObject, m_pTable->m_pTrackPlaylist);
        bpmTapDlg->show();
    }
}

void WTrackTableView::setSearchSource(WTrackTableModel * pSearchSourceModel)
{
    m_pTable = pSearchSourceModel;
    m_pSearchFilter->setSourceModel(m_pTable);
    setModel(m_pSearchFilter);
}

void WTrackTableView::setDirModel()
{
    m_pTable = NULL;
    setModel(m_pDirFilter);
    setRootIndex(m_pDirFilter->mapFromSource(m_dirindex));
}

void WTrackTableView::setPlaylistListModel(WPlaylistListModel *model)
{
    m_pTable = NULL;
    m_pPlaylistListModel = model;
    setModel(m_pPlaylistListModel);
    //setRootIndex(m_pDirFilter->mapFromSource(m_dirindex));
}

QDirModel* WTrackTableView::getDirModel()
{
    return m_pDirModel;
}

/** Gets the next track from the table while in "Browse mode" */
QString WTrackTableView::getNextTrackBrowseMode(TrackInfoObject* current)
{
    QString qNextTrackName;
    QString fullpath = current->getFilepath() + "/" + current->getFilename();
    
    QModelIndex temp_index = m_pDirModel->index(fullpath);
    //qDebug() << current->getFilepath();
    //qDebug() << "index of current track" << temp_index.row();
    
    //Don't run this through the filter, it crashes:
    //temp_index = m_pDirFilter->mapToSource(temp_index);
    //qDebug() << "index of filtered current track" << temp_index.row();
    
    //Increment the index to get the NEXT track...
    do {
    	temp_index = temp_index.sibling(temp_index.row()+1, 0);
    } while (temp_index.isValid() && m_pDirModel->isDir(temp_index));
    
    //If we broke out of the loop due to an invalid index, decrement to get a valid one.
    //This happens at the end if you seek forwards in NEXT mode with the last song in a directory.
    if (!temp_index.isValid())
    {
    	temp_index = temp_index.sibling(temp_index.row()-1, 0);
    }    
    
    qNextTrackName = m_pDirModel->filePath(temp_index);

    return qNextTrackName;
}

/** Gets the previous track from the table while in "Browse mode" */
QString WTrackTableView::getPrevTrackBrowseMode(TrackInfoObject* current)
{
    QString qNextTrackName;
    QString fullpath = current->getFilepath() + "/" + current->getFilename();
    
    QModelIndex temp_index = m_pDirModel->index(fullpath);

    //Decrement the index to get the NEXT track...
    do {
    	temp_index = temp_index.sibling(temp_index.row()-1, 0);
    } while (temp_index.isValid() && m_pDirModel->isDir(temp_index));
    
    //If we broke out of the loop due to an invalid index, increment to get a valid one.
    //This happens at the end if you seek backwards in NEXT mode with the first song in a directory.
    if (!temp_index.isValid())
    {
    	temp_index = temp_index.sibling(temp_index.row()+1, 0);
    }
    
    qNextTrackName = m_pDirModel->filePath(temp_index);

    return qNextTrackName;
}

void WTrackTableView::contextMenuEvent(QContextMenuEvent * event)
{
    index = indexAt(event->pos());

    if (index.isValid())
    {
        //Browse mode menu (using the QDirModel)
        if (!m_pTable)
        {
            QModelIndex temp_dirindex = m_pDirFilter->mapToSource(index);
            if (!m_pDirModel->isDir(temp_dirindex)) {
                m_dirTrackName = m_pDirModel->filePath(temp_dirindex);
            }
        }
        else //Regular library mode menu
        {
            m_pTrackInfoObject = m_pTable->m_pTrackPlaylist->getTrackAt(index.row());
        }

        QMenu menu(this);
        menu.addAction(PlayQueueAct);
        if (ControlObject::getControl(ConfigKey("[Channel1]","play"))->get()!=1.)
            menu.addAction(Player1Act);
        if (ControlObject::getControl(ConfigKey("[Channel2]","play"))->get()!=1.)
            menu.addAction(Player2Act);
        if (m_pTable) //Only show "Remove" in Library mode
            menu.addAction(RemoveAct);
        menu.exec(event->globalPos());
    }
}

void WTrackTableView::createActions()
{
    PlayQueueAct = new QAction(tr("Play Queue"),this);
    connect(PlayQueueAct, SIGNAL(triggered()), this, SLOT(slotSendToPlayqueue()));

    Player1Act = new QAction(tr("Player 1"),this);
    connect(Player1Act, SIGNAL(triggered()), this, SLOT(slotLoadPlayer1()));

    Player2Act = new QAction(tr("Player 2"),this);
    connect(Player2Act, SIGNAL(triggered()), this, SLOT(slotLoadPlayer2()));

    RemoveAct = new QAction(tr("Remove"),this);
    connect(RemoveAct, SIGNAL(triggered()), this, SLOT(slotRemoveFromPlaylist()));

/*
    PlayQueueActBrowse = new QAction(tr("Play Queue"),this);
    connect(PlayQueueActBrowse, SIGNAL(triggered()), this, SLOT(slotSendToPlayqueue()));

    Player1ActBrowse = new QAction(tr("Player 1"),this);
    connect(Player1ActBrowse, SIGNAL(triggered()), this, SLOT(slotLoadPlayer1()));

    Player2ActBrowse = new QAction(tr("Player 2"),this);
    connect(Player2ActBrowse, SIGNAL(triggered()), this, SLOT(slotLoadPlayer2()));

    RemoveActBrowse = new QAction(tr("Remove"),this);
    connect(RemoveActBrowse, SIGNAL(triggered()), this, SLOT(slotRemoveFromPlaylist()));
*/
}

void WTrackTableView::setTrack(Track * pTrack)
{
    m_pTrack = pTrack;
}

void WTrackTableView::slotLoadPlayer1()
{
    if (!m_pTable) //Browse mode
        m_pTrack->slotLoadPlayer1(m_dirTrackName);
    else //Library mode
        m_pTrack->slotLoadPlayer1(m_pTrackInfoObject);
}
void WTrackTableView::slotLoadPlayer2()
{
   if (!m_pTable) //Browse mode
        m_pTrack->slotLoadPlayer2(m_dirTrackName);
    else //Library mode
        m_pTrack->slotLoadPlayer2(m_pTrackInfoObject);
}

void WTrackTableView::slotSendToPlayqueue()
{
    if (!m_pTable) //Browse mode
        m_pTrack->slotSendToPlayqueue(m_dirTrackName);
    else //Library mode
        m_pTrack->slotSendToPlayqueue(m_pTrackInfoObject);
}
void WTrackTableView::slotRemoveFromPlaylist()
{
    m_pTable->removeRow(index.row(),index);
}

void WTrackTableView::slotFilter(const QString &pstr)
{
    m_filterString = pstr;
    slotFilter();
}

void WTrackTableView::slotFilter()
{
    m_pSearchFilter->setFilterFixedString(m_filterString);
    m_pDirFilter->setFilterFixedString(m_filterString);
}
