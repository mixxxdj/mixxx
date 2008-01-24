/* -*- mode:C++; indent-tabs-mode:s; tab-width:4; c-basic-offset:4; -*- */
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
    setSelectionMode(QAbstractItemView::ExtendedSelection);
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
    setTableMode(TABLE_MODE_LIBRARY);
    
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
	   m_pTable->setForegroundColor(WSkinColor::getCorrectColor(fgc));
    }


    // Row colors
    if (!WWidget::selectNode(node, "BgColorRowEven").isNull())
    {
        QColor r1;
        r1.setNamedColor(WWidget::selectNodeQString(node, "BgColorRowEven"));
		r1 = WSkinColor::getCorrectColor(r1);
        QColor r2;
        r2.setNamedColor(WWidget::selectNodeQString(node, "BgColorRowUneven"));
		r2 = WSkinColor::getCorrectColor(r2);

		// For now make text the inverse of the background so it's readable
		// In the future this should be configurable from the skin with this
		// as the fallback option
		QColor text(255 - r1.red(), 255 - r1.green(), 255 - r1.blue());
		//QColor text(255, 255, 255);

        setAlternatingRowColors ( true );
        QPalette Rowpalette = palette();
        Rowpalette.setColor(QPalette::Base, r1);
        Rowpalette.setColor(QPalette::AlternateBase, r2);
		Rowpalette.setColor(QPalette::Text, text);

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

/* Sets the table mode (library, play queue, browse, etc.) */
void WTrackTableView::setTableMode(int table_mode)
{
    m_iTableMode = table_mode;
}

/*sorts a given column*/
void WTrackTableView::sortColumn(int col, bool ascending, bool)
{
}

void WTrackTableView::repaintEverything() {

	// Hack ahoy:
	// This invalidates everything in a way i thought reset() would
	// Why exactly it works isn't clear but i suspect it's some subtlety with the filtering
	if (m_pTable) {
		setSearchSource(m_pTable);
	}
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
    if (m_iTableMode == TABLE_MODE_BROWSE) {
        QModelIndex temp_dirindex = m_pDirFilter->mapToSource(index);
        if (m_pDirModel->isDir(temp_dirindex)) //Double-clicking on a directory in browse mode
        {
            m_dirindex = temp_dirindex;
            setRootIndex(index);				//Browse to that directory
            m_pDirFilter->setIndex(m_dirindex);
        }
        else //Double-clicking on a file in browse mode
        {
        	//Send the song to the first player that's not playing.
			while (!m_selectedDirTrackNames.isEmpty()) {
				m_selectedDirTrackNames.pop_back();
			}

    		m_selectedDirTrackNames.append(m_pDirModel->filePath(temp_dirindex));
			
			if (ControlObject::getControl(ConfigKey("[Channel1]","play"))->get()!=1.)
		        this->slotLoadPlayer1();
		    else if (ControlObject::getControl(ConfigKey("[Channel2]","play"))->get()!=1.)
		        this->slotLoadPlayer2();
		}
        return;
    }
    else if (m_iTableMode == TABLE_MODE_LIBRARY || m_iTableMode == TABLE_MODE_PLAYQUEUE)
    {
        // Know we aren't in browse mode now, so use this mapping.
        QModelIndex temp_sindex = m_pSearchFilter->mapToSource(index);
        TrackInfoObject* pTrackInfoObject = m_pTable->m_pTrackPlaylist->getTrackAt(temp_sindex.row());
	
		//Show the BPM tap/track editor dialog.
		//showBPMTapDlg(pTrackInfoObject);

		//Send the song to the first player that's not playing.
		while (!m_selectedTrackInfoObjects.isEmpty()) {
		    m_selectedTrackInfoObjects.pop_back();
		}

		m_selectedTrackInfoObjects.append(pTrackInfoObject);
		
		if (ControlObject::getControl(ConfigKey("[Channel1]","play"))->get()!=1.)
            this->slotLoadPlayer1();
        else if (ControlObject::getControl(ConfigKey("[Channel2]","play"))->get()!=1.)
            this->slotLoadPlayer2();

    }
    else if (m_iTableMode == TABLE_MODE_PLAYLISTS)
    {
    	qDebug() << "FIXME: Half-assed slotMouseDoubleClicked implementation in" << __FILE__ << "at line" << __LINE__;
        
        //FIXME: Make m_pSearchFilter  work with a WPlaylistListModel.
		//QModelIndex temp_sindex = m_pSearchFilter->mapToSource(index);
		//TrackPlaylist* selectedPlaylist = m_pTrack->getPlaylistByIndex(temp_sindex.row());
		//qDebug() << index.row() << temp_sindex.row();
		
		qDebug() << index.row();
		TrackPlaylist* selectedPlaylist = m_pTrack->getPlaylistByIndex(index.row());
		
		m_pTrack->slotShowPlaylist(selectedPlaylist);
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
    m_pTable = NULL; //FIXME: Shouldn't set to NULL, should set m_iTableMode = TABLE_MODE_BROWSE instead
    setModel(m_pDirFilter);
    setRootIndex(m_pDirFilter->mapFromSource(m_dirindex));
}

void WTrackTableView::setPlaylistListModel(WPlaylistListModel *model)
{
    m_pPlaylistListModel = model;
    setModel(m_pPlaylistListModel);
    //FIXME: Set search filter model somewhere or something?
}

QDirModel* WTrackTableView::getDirModel()
{
    return m_pDirModel;
}

void WTrackTableView::slotShowBPMTapDlg()
{	
	Q_ASSERT(m_selectedTrackInfoObjects.count() > 0);
	slotShowBPMTapDlg(m_selectedTrackInfoObjects.at(0));
}

void WTrackTableView::slotShowBPMTapDlg(TrackInfoObject* pTrackInfoObject)
{
    if(pTrackInfoObject)
    {
        if(bpmTapDlg)
            delete bpmTapDlg;

        bpmTapDlg = new DlgBpmTap(NULL, pTrackInfoObject, m_pTable->m_pTrackPlaylist);
        bpmTapDlg->show();
    }
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
    //index = indexAt(event->pos());

	bool isFolder = false;

    //Get the indices of the selected rows.
    m_selectedIndices = this->selectionModel()->selectedRows();
    
    /*
    //Print out the selected idices using messageboxes.
    for (int i = 0; i < m_selectedIndices.count(); ++i)
    { 
        QModelIndex index = m_selectedIndices.at(i);
        QMessageBox::information(this,"", QString::number(index.row()));
    }*/

    //Clear the old internal selection
    while (!m_selectedDirTrackNames.isEmpty()) {
        m_selectedDirTrackNames.pop_back();
    }
    while (!m_selectedPlaylists.isEmpty()) {
        m_selectedPlaylists.pop_back();
    }
    while (!m_selectedTrackInfoObjects.isEmpty()) {
        m_selectedTrackInfoObjects.pop_back();
    }
    
    
    for (int i = 0; i < m_selectedIndices.count(); i++)
    {
    	QModelIndex index = m_selectedIndices.at(i);
    
        //Browse mode menu (using the QDirModel)
        if (m_iTableMode == TABLE_MODE_BROWSE)
        {
            QModelIndex temp_dirindex = m_pDirFilter->mapToSource(index);
            if (!m_pDirModel->isDir(temp_dirindex)) {
                m_selectedDirTrackNames.append(m_pDirModel->filePath(temp_dirindex));
            }
			else
			{
				isFolder = true;
			}
        }
        else if (m_iTableMode == TABLE_MODE_LIBRARY || m_iTableMode == TABLE_MODE_PLAYQUEUE) //Regular library mode menu
        {
            QModelIndex temp_sindex = m_pSearchFilter->mapToSource(index);
            m_selectedTrackInfoObjects.append(m_pTable->m_pTrackPlaylist->getTrackAt(temp_sindex.row()));
        }
        else if (m_iTableMode == TABLE_MODE_PLAYLISTS)
        {           
            //FIXME: This is the code to make searching work, but m_pSearchFilter doesn't work with a WPlaylistListModel yet.
            //QModelIndex temp_sindex = m_pSearchFilter->mapToSource(index);
            //m_selectedPlaylist = m_pTrack->getPlaylistByIndex(temp_sindex.row());
            
            m_selectedPlaylists.append(m_pTrack->getPlaylistByIndex(index.row()));
            
            qDebug() << "Right-clicked" << m_selectedPlaylists.at(m_selectedPlaylists.count()-1)->getListName();
        }
    }
    
    //Create the right-click menu
    QMenu menu(this);
   
        
    //Populate it with various action items depending on what mode the table is in
    menu.addAction(PlayQueueAct);

    if (m_iTableMode == TABLE_MODE_LIBRARY || 
        m_iTableMode == TABLE_MODE_PLAYQUEUE || 
        m_iTableMode == TABLE_MODE_BROWSE)
    {
        menu.addAction(Player1Act);
        menu.addAction(Player2Act);  
        menu.addAction(BPMTapAct);
    }

	

    //Gray out some stuff if multiple songs were selected. 
    if (m_selectedIndices.count() != 1)
    {
		Player1Act->setEnabled(false);
		Player2Act->setEnabled(false);
		BPMTapAct->setEnabled(false);
    }
    else
    {
		Player1Act->setEnabled(true);
		Player2Act->setEnabled(true);  
		BPMTapAct->setEnabled(true);  	
    }
	  
    //Gray out player 1 and/or player 2 if those players are playing.
    if (ControlObject::getControl(ConfigKey("[Channel1]","play"))->get()==1.)
    	Player1Act->setEnabled(false);
    if (ControlObject::getControl(ConfigKey("[Channel2]","play"))->get()==1.)  
    	Player2Act->setEnabled(false);  
    	
    menu.addSeparator();
    menu.addAction(RemoveAct);
    
    //Gray out "Remove" in BROWSE mode
    if (m_iTableMode == TABLE_MODE_BROWSE)      
	{	
        RemoveAct->setEnabled(false);
		BPMTapAct->setEnabled(false);

        if (isFolder)
		{
			Player1Act->setEnabled(false);
			Player2Act->setEnabled(false);
		}
    }
    else
    	RemoveAct->setEnabled(true);
    	
    
    menu.exec(event->globalPos());
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
    connect(RemoveAct, SIGNAL(triggered()), this, SLOT(slotRemove()));

	BPMTapAct = new QAction(tr("Properties"), this);
	connect(BPMTapAct, SIGNAL(triggered()), this, SLOT(slotShowBPMTapDlg()));
/*
    PlayQueueActBrowse = new QAction(tr("Play Queue"),this);
    connect(PlayQueueActBrowse, SIGNAL(triggered()), this, SLOT(slotSendToPlayqueue()));

    Player1ActBrowse = new QAction(tr("Player 1"),this);
    connect(Player1ActBrowse, SIGNAL(triggered()), this, SLOT(slotLoadPlayer1()));

    Player2ActBrowse = new QAction(tr("Player 2"),this);
    connect(Player2ActBrowse, SIGNAL(triggered()), this, SLOT(slotLoadPlayer2()));

    RemoveActBrowse = new QAction(tr("Remove"),this);
    connect(RemoveActBrowse, SIGNAL(triggered()), this, SLOT(slotRemove()));
*/
}

void WTrackTableView::setTrack(Track * pTrack)
{
    m_pTrack = pTrack;
}

/* Move the cursor to the next track. */
void WTrackTableView::selectNext()
{
    QModelIndex c = currentIndex();

    // Create a row selection if none exists
    if (c.row() == -1) {
	selectRow(0);
	c = currentIndex();
    }
    // Advance to next position
    setCurrentIndex(c.child(c.row()+1, c.column()));
    // Roll back to previous position if on last row
    if (currentIndex().row() == -1) setCurrentIndex(c);
    selectRow(currentIndex().row());
}

/* Move the cursor to the previous track. */
void WTrackTableView::selectPrevious()
{
    QModelIndex c = currentIndex();

    // Move to the previous row
    setCurrentIndex(c.child(c.row()-1, c.column()));
    // Roll back to previous position if on first row
    if (currentIndex().row() == -1) setCurrentIndex(c);
    selectRow(currentIndex().row());
}

void WTrackTableView::slotLoadPlayer1()
{
    if (m_iTableMode == TABLE_MODE_BROWSE) //Browse mode
        m_pTrack->slotLoadPlayer1(m_selectedDirTrackNames.at(0));
    else //Library mode
        m_pTrack->slotLoadPlayer1(m_selectedTrackInfoObjects.at(0));
}
void WTrackTableView::slotLoadPlayer2()
{
   if (m_iTableMode == TABLE_MODE_BROWSE) //Browse mode
        m_pTrack->slotLoadPlayer2(m_selectedDirTrackNames.at(0));
    else //Library mode
        m_pTrack->slotLoadPlayer2(m_selectedTrackInfoObjects.at(0));
}

void WTrackTableView::slotSendToPlayqueue()
{
    if (m_iTableMode == TABLE_MODE_BROWSE) //Browse mode
    {
        for (int i = 0; i < m_selectedDirTrackNames.count(); i++) {
            m_pTrack->slotSendToPlayqueue(m_selectedDirTrackNames.at(i));
        }
    }
    else if (m_iTableMode == TABLE_MODE_PLAYLISTS)
    {
        for (int i = 0; i < m_selectedPlaylists.count(); i++) {
        	m_pTrack->slotSendToPlayqueue(m_selectedPlaylists.at(i));
        }
    }
    else //Library mode, etc.
    {
    	for (int i = 0; i < m_selectedTrackInfoObjects.count(); i++) {
        	m_pTrack->slotSendToPlayqueue(m_selectedTrackInfoObjects.at(i));
        }
    }
}
void WTrackTableView::slotRemove()
{
    if (m_iTableMode == TABLE_MODE_PLAYLISTS)
    {   
    	qDebug() << "FIXME: Removing a playlist is unimplented in" << __FILE__ << "on line" << __LINE__;
        //Doesn't work right for some reason:
        //m_pPlaylistListModel->removeRow(index.row());
        //This also probably needs to be reworked to work with multiple selections
        for (int i = 0; i < m_selectedIndices.count(); i++)
        {
              	
		}
    }
    else if (m_iTableMode == TABLE_MODE_BROWSE)
    {
    	qDebug() << "FIXME (?): Removing a song in browse mode is unimplented in" << __FILE__ << "on line" << __LINE__;
    }
    else //Library mode, play queue mode, etc.
    {
        for (int i = 0; i < m_selectedIndices.count(); i++)
        {
        	QModelIndex index = m_selectedIndices.at(i);
        	m_pTable->removeRow(index.row() - i );
        	//qDebug() << "Removed row" << index.row() - i;
        }
    }
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
