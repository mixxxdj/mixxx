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
    m_pTrack = NULL;
    bpmTapDlg = 0;

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
    m_pDirModel->setSorting(QDir::DirsFirst | QDir::IgnoreCase);
    m_dirindex = m_pDirModel->index(m_pConfig->getValueString(ConfigKey("[Playlist]","Directory")));

    m_pDirFilter = new WTrackTableFilter(m_dirindex);
    m_pDirFilter->setSourceModel(m_pDirModel);
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

int WTrackTableView::getTableMode()
{
    return m_iTableMode;
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

/*
    // FIXME: There is no point in calling this stuff as it is not reflected in the interface anywhere.
    bool ascending = (horizontalHeader()->sortIndicatorOrder() == Qt::AscendingOrder);

    if (m_pTable)
    {
        switch(col)
        {
            // Score Column
        case COL_SCORE:
            m_pTable->m_pTrackPlaylist->sortByScore(ascending);
            break;

            // Title
        case COL_TITLE:
            m_pTable->m_pTrackPlaylist->sortByTitle(ascending);
            break;

            // Artist
        case COL_ARTIST:
            m_pTable->m_pTrackPlaylist->sortByArtist(ascending);
            break;

            // Type
        case COL_TYPE:
            m_pTable->m_pTrackPlaylist->sortByType(ascending);
            break;

            // Duration
        case COL_LENGTH:
            m_pTable->m_pTrackPlaylist->sortByDuration(ascending);
            break;

            // Bitrate
        case COL_BITRATE:
            m_pTable->m_pTrackPlaylist->sortByBitrate(ascending);
            break;

            // BPM
        case COL_BPM:
            m_pTable->m_pTrackPlaylist->sortByBpm(ascending);
            break;

            // Comment
        case COL_COMMENT:
            m_pTable->m_pTrackPlaylist->sortByComment(ascending);
            break;
        }
	slotFilter();
    }
*/
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

        bpmTapDlg = new DlgBpmTap(NULL, pTrackInfoObject, m_pTable->m_pTrackPlaylist, m_pConfig);
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
    QMenu addToPlaylists(tr("Add to Playlist"));

    //Populate it with various action items depending on what mode the table is in
    menu.addAction(PlayQueueAct);
	
    if (m_iTableMode == TABLE_MODE_LIBRARY ||
        m_iTableMode == TABLE_MODE_PLAYQUEUE ||
        m_iTableMode == TABLE_MODE_BROWSE)
    {
    	//Add the "Player 1" action
        menu.addAction(Player1Act);

        //Add the "Player 2" action
        menu.addAction(Player2Act);

        //Add the "Add to Playlist" menu.
		menu.addMenu(&addToPlaylists);
		
		//Fill the "Add to... playlists" menu with the names of the playlists
		for (int i = 0; i < PlaylistActs.size(); i++)
		{
			addToPlaylists.addAction(PlaylistActs.at(i));
		}

        menu.addAction(PropertiesAct);
    }
	
	//Show the "Rename..." option for the playlists view.
	if (m_iTableMode == TABLE_MODE_PLAYLISTS)
	{
		menu.addAction(RenamePlaylistAct);
	}

    //Gray out some stuff if multiple songs were selected.
    if (m_selectedIndices.count() != 1)
    {
		Player1Act->setEnabled(false);
		Player2Act->setEnabled(false);
		PropertiesAct->setEnabled(false);
    }
    else
    {
		Player1Act->setEnabled(true);
		Player2Act->setEnabled(true);
		PropertiesAct->setEnabled(true);
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
		PropertiesAct->setEnabled(false);

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
    PlayQueueAct = new QAction(tr("Add to Play Queue"),this);
    connect(PlayQueueAct, SIGNAL(triggered()), this, SLOT(slotSendToPlayqueue()));

    Player1Act = new QAction(tr("Load in Player 1"),this);
    connect(Player1Act, SIGNAL(triggered()), this, SLOT(slotLoadPlayer1()));

    Player2Act = new QAction(tr("Load in Player 2"),this);
    connect(Player2Act, SIGNAL(triggered()), this, SLOT(slotLoadPlayer2()));

    RemoveAct = new QAction(tr("Remove"),this);
    connect(RemoveAct, SIGNAL(triggered()), this, SLOT(slotRemove()));

	PropertiesAct = new QAction(tr("Properties..."), this);
	connect(PropertiesAct, SIGNAL(triggered()), this, SLOT(slotShowBPMTapDlg()));

	RenamePlaylistAct = new QAction(tr("Rename..."), this);
	connect(RenamePlaylistAct, SIGNAL(triggered()), this, SLOT(slotShowPlaylistRename()));
	
	
	//Create all the "send to->playlist" actions.
	if (m_pTrack)
		updatePlaylistActions();
	
}

/** Shows the "Rename Playlist" input dialog */
void WTrackTableView::slotShowPlaylistRename()
{
	bool ok;
	
	if (m_iTableMode == TABLE_MODE_PLAYLISTS)
    {
        for (int i = 0; i < m_selectedPlaylists.count(); i++)
        {

     		QString text = QInputDialog::getText(this, tr("Rename Playlist"),
                                          tr("Rename ") + m_selectedPlaylists.at(i)->getName() + " to:",
                                          QLineEdit::Normal,
                                          m_selectedPlaylists.at(i)->getName(), &ok);
     		if (ok && !text.isEmpty())
         		m_selectedPlaylists.at(i)->setListName(text);
         	   	
        	
        	//m_pTrack->slotSendToPlayqueue(m_selectedPlaylists.at(i));
        }

        //Update the right-click menu with the new name(s) of the playlist(s).
        updatePlaylistActions();
    }
}

void WTrackTableView::updatePlaylistActions()
{
	QAction* cur_action = NULL;
	
	//Clear and free any existing playlist QActions
	while (!PlaylistActs.isEmpty())
	{
		cur_action = PlaylistActs.takeLast();
		delete cur_action;
	}
	
	//Get the list of playlists
	TrackPlaylistList* playlists = m_pTrack->getPlaylists();
		
	//Make sure it's not NULL
	Q_ASSERT(playlists != NULL);
	
	//Make sure it's not empty (QLists can be strange sometimes when they're empty)
	if (playlists->isEmpty())
	{
		qDebug() << "No playlists, returning";
		return;
	}
	
	QString qPlaylistName;
	QAction* sendToPlaylistAction;
	
	//Create a send-to action for each playlist.
	for (int i = 0; i < playlists->count(); i++)
	{
		if (playlists->at(i) != NULL)
		{
			qPlaylistName = playlists->at(i)->getName();
			//qDebug() << "Iterated:" << qPlaylistName;
		
			//Create a new action for this playlist
			sendToPlaylistAction = new QAction(qPlaylistName, this);
			sendToPlaylistAction->setData(QVariant((qlonglong)playlists->at(i)));
			PlaylistActs.append(sendToPlaylistAction);
		
			//Connect this action to some sendtoplaylist(name) action.
			connect(sendToPlaylistAction, SIGNAL(triggered()), this, SLOT(slotSendToPlaylist()));
		}
		else
		{
			qDebug() << "NULL playlist detected in" << __FILE__ << "at line:" << __LINE__;
		}
	}
	
	
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

void WTrackTableView::slotSendToPlaylist()
{
	TrackPlaylist* playlist;
	int playlist_ptr;
	
	//This is terrible, but necessary. We look back at the sender that sent the signal, try to cast it to a QAction,
	//then look at the QAction's "data" member to get the pointer to the playlist.
	if (QAction *act = qobject_cast<QAction *>(sender())) {
    	playlist_ptr = act->data().toInt();
    	playlist = (TrackPlaylist*)playlist_ptr;
    }
	else
	{
		qDebug() << "FIXME: slotSendToPlaylist() is only implemented for QActions in" << __FILE__ "on line:" << __LINE__;
		return;
	}
	
    if (m_iTableMode == TABLE_MODE_BROWSE) //Browse mode
    {
        for (int i = 0; i < m_selectedDirTrackNames.count(); i++) {
            m_pTrack->slotSendToPlaylist(playlist, m_selectedDirTrackNames.at(i));
        }
    }
    else if (m_iTableMode == TABLE_MODE_PLAYLISTS)
    {
		qDebug() << "FIXME: Trying to send a playlist to a playlist... (did you code something incorrectly?)";
		qDebug() << __FILE__ ":" << __LINE__;
    }
    else //Library mode, etc.
    {
    	for (int i = 0; i < m_selectedTrackInfoObjects.count(); i++) {
        	m_pTrack->slotSendToPlaylist(playlist, m_selectedTrackInfoObjects.at(i));
        }
    }	
}

void WTrackTableView::slotRemove()
{
    if (m_iTableMode == TABLE_MODE_PLAYLISTS)
    {
	//Get the indices of the selected rows.
    	m_selectedIndices = this->selectionModel()->selectedRows();

    	for (int i = 0; i < m_selectedIndices.count(); i++)
    	{
    		QModelIndex index = m_selectedIndices.at(i);
        	m_pTrack->getPlaylists()->removeAt(index.row());
        	
        	//FIXME: The approach above probably won't work when sorting is fixed in the
        	//		 playlists view.
        	//		 Something like this may or may not work instead:
        	//m_pTrack->getPlaylists()->removeAll(m_pTrack->getPlaylistByIndex(index.row()));
        }
		
		updatePlaylistActions(); //Update the right-click menu's list of playlists.
		repaintEverything(); //For good luck
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

/** Stub for drag and drop... FIXME into real drag-and-drop-to-library support - Albert Feb 6/08*/
void WTrackTableView::dragEnterEvent(QDragEnterEvent * event)
{
  if (event->mimeData()->hasUrls())
      event->acceptProposedAction();
}

/** Stub for drag and drop... FIXME into real drag-and-drop-to-library support - Albert Feb 6/08*/
void WTrackTableView::dropEvent(QDropEvent * event)
{
  if (event->mimeData()->hasUrls()) {
    QList<QUrl> urls(event->mimeData()->urls());
    QUrl url = urls.first();

    event->accept();
    //emit(trackDropped(name));

    //Add the track(s) to the active playlist
    m_pTrack->getActivePlaylist()->addTrack(url.path());
    repaintEverything();
  } else
    event->ignore();
}

void WTrackTableView::keyPressEvent(QKeyEvent *event)
{

}

QString WTrackTableView::getFilterString() {
    return m_filterString;
}
