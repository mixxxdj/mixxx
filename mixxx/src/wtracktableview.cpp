/* -*- mode:C++; indent-tabs-mode:s; tab-width:4; c-basic-offset:4; -*- */
#include "wtracktableview.h"
#include "wplaylistlistmodel.h"
#include "wtracktablefilter.h"
#include "wtracktablemodel.h"
#include "widget/wwidget.h"
#include "widget/wskincolor.h"
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
#include <QtGui>
#include <QtCore>

//Added by qt3to4:
#include <Q3Frame>
#include <qwidget.h>
#include <QTableView>
#include <QMenu>
#include <QDirModel>


#include "dlgbpmtap.h"

/* CTAF's TODO list
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

    //Drag and drop setup
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDropIndicatorShown(true);
    setAcceptDrops(true);

    //Blacklist whatever table modes we don't want to allow drag-and-drop to:
    m_dndTableModeBlacklist.append(TABLE_MODE_PROMO);
    m_dndTableModeBlacklist.append(TABLE_MODE_IPOD);
    m_dndTableModeBlacklist.append(TABLE_MODE_PLAYLISTS);
    m_dndTableModeBlacklist.append(TABLE_MODE_BROWSE);

    setWordWrap(false);
    setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
    setShowGrid(false);
    horizontalHeader()->setClickable(true);
    verticalHeader()->hide();
    verticalHeader()->setDefaultSectionSize(20);
    setFrameStyle(Q3Frame::NoFrame);
    setCornerButtonEnabled ( false );
    createActions();
    m_pSearchFilter = new SortFilterProxyModel(parent);
    setTableMode(TABLE_MODE_LIBRARY);

    // Sort Library by Artist on startup...
    setSortingEnabled ( true );
    horizontalHeader()->setSortIndicatorShown(true);
    horizontalHeader()->setSortIndicator(WTrackTableModel::ARTIST, Qt::AscendingOrder);
    m_pSearchFilter->sort(WTrackTableModel::ARTIST);


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
    //NOTE: Position and size are now managed by a QLayout
    //      inside mixxxview.cpp's tab widget.
/*
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
  */

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
       ColumnMap[WTrackTableModel::SCORE] = "ColWidthScore";
       ColumnMap[WTrackTableModel::TITLE] = "ColWidthTitle";
       ColumnMap[WTrackTableModel::ARTIST] = "ColWidthArtist";
       ColumnMap[WTrackTableModel::COMMENT] = "ColWidthComment";
       ColumnMap[WTrackTableModel::TYPE] = "ColWidthType";
       ColumnMap[WTrackTableModel::DURATION] = "ColWidthDuration";
       ColumnMap[WTrackTableModel::BITRATE] = "ColWidthBitrate";
       ColumnMap[WTrackTableModel::BPM] = "ColWidthBpm";

       QMapIterator<int,QString> i(ColumnMap);
       while (i.hasNext())
       {
            i.next();
            if (!WWidget::selectNode(node, i.value()).isNull() && WWidget::selectNodeQString(node, i.value()).toInt() != columnWidth(i.key()))
            {
                    qDebug() << "Correcting Column Width from " << columnWidth(i.key()) << " to " << WWidget::selectNodeQString(node;
                    setColumnWidth(i.key(),WWidget::selectNodeQString(node, i.value()).toInt());
            }
       }*/
}

/* Sets the table mode (library, play queue, browse, etc.) */
void WTrackTableView::setTableMode(table_mode_t table_mode)
{
    m_iTableMode = table_mode;
}

table_mode_t WTrackTableView::getTableMode()
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
        qDebug() << "FIXME: repaintEverything switches table model and shouldn't do that when viewing the playlist model in " << __FILE__ ": " << __LINE__;
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
        case WTrackTableModel::SCORE:
            m_pTable->m_pTrackPlaylist->sortByScore(ascending);
            break;

            // Title
        case WTrackTableModel::TITLE:
            m_pTable->m_pTrackPlaylist->sortByTitle(ascending);
            break;

            // Artist
        case WTrackTableModel::ARTIST:
            m_pTable->m_pTrackPlaylist->sortByArtist(ascending);
            break;

            // Type
        case WTrackTableModel::TYPE:
            m_pTable->m_pTrackPlaylist->sortByType(ascending);
            break;

            // Duration
        case WTrackTableModel::LENGTH:
            m_pTable->m_pTrackPlaylist->sortByDuration(ascending);
            break;

            // Bitrate
        case WTrackTableModel::BITRATE:
            m_pTable->m_pTrackPlaylist->sortByBitrate(ascending);
            break;

            // BPM
        case WTrackTableModel::BPM:
            m_pTable->m_pTrackPlaylist->sortByBpm(ascending);
            break;

            // Comment
        case WTrackTableModel::COMMENT:
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
//         QModelIndex temp_dirindex = m_pDirFilter->mapToSource(index);
        QModelIndex temp_dirindex = m_pDirFilter->mapToSource(index.sibling(index.row(),0));
        if (m_pDirModel->isDir(temp_dirindex)) //Double-clicking on a directory in browse mode
        {
            m_dirindex = temp_dirindex;
            setRootIndex(index.sibling(index.row(),0));     //Browse to that directory
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
    else if (m_iTableMode == TABLE_MODE_LIBRARY ||
             m_iTableMode == TABLE_MODE_PLAYQUEUE ||
             m_iTableMode == TABLE_MODE_PROMO ||
             m_iTableMode == TABLE_MODE_IPOD)
    {
        // Know we aren't in browse mode now, so use this mapping.
        QModelIndex temp_sindex = m_pSearchFilter->mapToSource(index);
        TrackInfoObject* pTrackInfoObject = m_pTable->m_pTrackPlaylist->at(temp_sindex.row());

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
        else if (m_iTableMode == TABLE_MODE_LIBRARY ||
                 m_iTableMode == TABLE_MODE_PLAYQUEUE ||
                 m_iTableMode == TABLE_MODE_PROMO ||
                 m_iTableMode == TABLE_MODE_IPOD) //Regular library mode menu
        {
            QModelIndex temp_sindex = m_pSearchFilter->mapToSource(index);
            m_selectedTrackInfoObjects.append(m_pTable->m_pTrackPlaylist->at(temp_sindex.row()));
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
        m_iTableMode == TABLE_MODE_BROWSE ||
        m_iTableMode == TABLE_MODE_PROMO ||
        m_iTableMode == TABLE_MODE_IPOD)
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

    //For every mode but promo mode, show the "Remove" menu item
    if (m_iTableMode != TABLE_MODE_PROMO && m_iTableMode != TABLE_MODE_IPOD)
    {
        menu.addSeparator();
        menu.addAction(RemoveAct);
    }
    else //For promo mode, add the "copy to library" and "visit website" actions instead
    {
        menu.addAction(CopyToLibraryAct);
        if (m_iTableMode == TABLE_MODE_PROMO) {
          menu.addAction(VisitWebsiteAct);
        }
    }

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

    CopyToLibraryAct = new QAction(tr("Copy to Library"), this);
    connect(CopyToLibraryAct, SIGNAL(triggered()), this, SLOT(slotCopyToLibrary()));

    VisitWebsiteAct = new QAction(tr("Visit Website..."), this);
    connect(VisitWebsiteAct, SIGNAL(triggered()), this, SLOT(slotVisitWebsite()));

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

void WTrackTableView::resizeEvent(QResizeEvent *event)
{
    m_pTrack->resizeColumnsForLibraryMode(); //fucking massive hack
    //I hate life
	QWidget::resizeEvent(event);
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
    //Get the indices of the selected rows.
    m_selectedIndices = this->selectionModel()->selectedRows();

    if (m_iTableMode == TABLE_MODE_PLAYLISTS)
    {
        for (int i = 0; i < m_selectedIndices.count(); i++)
        {
            QModelIndex index = m_selectedIndices.at(i);
            m_pTrack->getPlaylists()->removeAt(index.row());

            //FIXME: The approach above probably won't work when sorting is fixed in the
            //         playlists view.
            //         Something like this may or may not work instead:
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
        //Iterate backwards to make sure we remove the bottom rows first. This
        //prevents the model indices from changing as we remove rows.
        for (int i = m_selectedIndices.count() - 1; i >= 0 ; i--)
        {
            QModelIndex index = m_selectedIndices.at(i);
            QModelIndex filteredIndex = m_pSearchFilter->mapToSource(index);
            m_pTable->removeRow(filteredIndex.row());
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

/** Drag enter event, happens when a dragged item hovers over the track table view*/
void WTrackTableView::dragEnterEvent(QDragEnterEvent * event)
{
    //qDebug() << "DragEnterEvent" << event->mimeData()->formats();
    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

/** Drag move event, happens when a dragged item hovers over the track table view...
 *  Why we need this is a little vague, but without it, drag-and-drop just doesn't work.
 *  -- Albert June 8/08
 */
void WTrackTableView::dragMoveEvent(QDragMoveEvent * event)
{
    //qDebug() << "dragMoveEvent" << event->mimeData()->formats();
    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

/** Drag-and-drop "drop" event. Occurs when something is dropped onto the track table view */
void WTrackTableView::dropEvent(QDropEvent * event)
{    
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls(event->mimeData()->urls());
        QUrl url;

        //Handle drag-and-drop unless we're on a weird
        //playlist like the promo tracks or ipod one.
        if (m_dndTableModeBlacklist.contains(getTableMode()))
        {
            event->ignore();
            return;
        }

        //Drag and drop within this widget (track reordering)
        if (event->source() == this && event->possibleActions() & Qt::MoveAction)
        {
            m_selectedIndices = this->selectionModel()->selectedRows();

            QList<TrackInfoObject*> selectedTracks;

            //The model indices are sorted so that we remove the tracks from the table
            //in ascending order. This is necessary because if track A is above track B in
            //the table, and you remove track A, the model index for track B will change.
            //Sorting the indices first means we don't have to worry about this.
            qSort(m_selectedIndices);

            //Going through the model indices in descending order (see above comment for explanation).
            for (int i = m_selectedIndices.count() - 1; i >= 0 ; i--)
            {
                //All the funky "+/- i" math in the next block of code is because when you
                //remove a row, you move the rows below it up. Similarly, when you insert a row,
                //you move the rows below it down.
                QModelIndex srcIndex = m_selectedIndices.at(i);
                QModelIndex filteredSrcIndex = m_pSearchFilter->mapToSource(srcIndex);
                TrackInfoObject *pTrack = m_pTrack->getActivePlaylist()->at(srcIndex.row());
                if (m_pTable->removeRow(srcIndex.row()))
                {
                    selectedTracks.append(pTrack);
                }
            }

            //Reset the indices which are selected (ie. temporarily say that no tracks are selected)
            m_selectedIndices.clear();
            this->selectionModel()->clear();

            for (int i = 0; i < selectedTracks.count(); i++)
            {
                QModelIndex destIndex = this->indexAt(event->pos());
                QModelIndex filteredDestIndex = m_pSearchFilter->mapToSource(destIndex);
                //Insert the row into the new spot
                if (m_pTable->insertRow(filteredDestIndex.row(), selectedTracks.at(i)))
                {
                    this->selectionModel()->select(filteredDestIndex, QItemSelectionModel::Select |
                                                                      QItemSelectionModel::Rows);
                }
                else
                {
                    //If we failed to insert the row, put it back to where it was before??
                    //m_pTable->insertRow(srcIndex.row(), selectedTracks.at(i));
                    qDebug() << "failed to insert at row" << filteredDestIndex.row();
                }
            }
        }
        else
        {
            //Reset the selected tracks (if you had any tracks highlighted, it clears them)
            this->selectionModel()->clear();

            //Drag-and-drop from an external application
            //eg. dragging a track from Windows Explorer onto the track table.
            foreach (url, urls)
            {
                QModelIndex destIndex = this->indexAt(event->pos());
                //FIXME: Use search filter here? I'm not sure what the correct behaviour
                //       should be when a track is dropped onto the table and there's an
                //       active search filter.
                TrackInfoObject* draggedTrack = m_pTrack->getTrackCollection()->getTrack(url.toLocalFile());
                if (draggedTrack) //Make sure the track was valid
                {
                    if (m_pTable->insertRow(destIndex.row(), draggedTrack))
                    {
                        this->selectionModel()->select(destIndex, QItemSelectionModel::Select |
                                                                  QItemSelectionModel::Rows);
                    }
                }
            }
        }

        event->acceptProposedAction();
        //emit(trackDropped(name));

        //repaintEverything();
    } else
        event->ignore();
}

void WTrackTableView::keyPressEvent(QKeyEvent *event)
{

}

QString WTrackTableView::getFilterString() {
    return m_filterString;
}

/** Copies the selected tracks to the library */
void WTrackTableView::slotCopyToLibrary()
{
    bool success = true;

    //Copy each track selected to the library
    for (int i = 0; i < m_selectedTrackInfoObjects.count(); i++) {
        QString srcLocation = m_selectedTrackInfoObjects.at(i)->getLocation();
        QString destLocation = m_pConfig->getValueString(ConfigKey("[Playlist]", "Directory"));
        destLocation += "/" + m_selectedTrackInfoObjects.at(i)->getFilename();
        success = QFile::copy(srcLocation, destLocation);

        qDebug() << "Copying" << srcLocation << "to" << destLocation;

        if (!success)
            break;
    }

    if (!success)
    {
        QMessageBox::warning(NULL, tr("Failed to copy track(s)"), tr("Warning: Failed to copy track(s) to library"));
    }

}

void WTrackTableView::slotVisitWebsite()
{
    for (int i = 0; i < m_selectedTrackInfoObjects.count(); i++) {

        QUrl website(m_selectedTrackInfoObjects.at(i)->getURL());
        QDesktopServices::openUrl(website);
    }
}

