#include <QItemDelegate>
#include <QtCore>
#include <QtGui>
#include <QtXml>
#include <QModelIndex>

#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "widget/wtracktableviewheader.h"
#include "library/librarytablemodel.h"
#include "library/trackcollection.h"
#include "trackinfoobject.h"
#include "controlobject.h"
#include "wtracktableview.h"
#include "dlgtrackinfo.h"

WTrackTableView::WTrackTableView(QWidget * parent,
                                 ConfigObject<ConfigValue> * pConfig,
                                 TrackCollection* pTrackCollection)
        : WLibraryTableView(parent, pConfig,
                            ConfigKey(LIBRARY_CONFIGVALUE,
                                      WTRACKTABLEVIEW_VSCROLLBARPOS_KEY)),
          m_pConfig(pConfig),
          m_pTrackCollection(pTrackCollection),
          m_searchThread(this) {

    pTrackInfo = new DlgTrackInfo(this);
    connect(pTrackInfo, SIGNAL(next()),
            this, SLOT(slotNextTrackInfo()));
    connect(pTrackInfo, SIGNAL(previous()),
            this, SLOT(slotPrevTrackInfo()));

    m_pMenu = new QMenu(this);
    m_pPlaylistMenu = new QMenu(this);
    m_pPlaylistMenu->setTitle(tr("Add to Playlist"));
    m_pCrateMenu = new QMenu(this);
    m_pCrateMenu->setTitle(tr("Add to Crate"));
    //Disable editing
    //setEditTriggers(QAbstractItemView::NoEditTriggers);

    //Create all the context m_pMenu->actions (stuff that shows up when you
    //right-click)
    createActions();

    //Connect slots and signals to make the world go 'round.
    connect(this, SIGNAL(doubleClicked(const QModelIndex &)),
            this, SLOT(slotMouseDoubleClicked(const QModelIndex &)));

    connect(&m_playlistMapper, SIGNAL(mapped(int)),
            this, SLOT(addSelectionToPlaylist(int)));
    connect(&m_crateMapper, SIGNAL(mapped(int)),
            this, SLOT(addSelectionToCrate(int)));
}

WTrackTableView::~WTrackTableView()
{
    WTrackTableViewHeader* pHeader =
            dynamic_cast<WTrackTableViewHeader*>(horizontalHeader());
    if (pHeader) {
        pHeader->saveHeaderState();
    }

    delete m_pAutoDJAct;
    delete m_pPlayer1Act;
    delete m_pPlayer2Act;
    delete m_pRemoveAct;
    delete m_pPropertiesAct;
    delete m_pMenu;
    delete m_pPlaylistMenu;
    delete m_pCrateMenu;
    //delete m_pRenamePlaylistAct;
}

void WTrackTableView::loadTrackModel(QAbstractItemModel *model) {
    qDebug() << "WTrackTableView::loadTrackModel()" << model;

    TrackModel* track_model = dynamic_cast<TrackModel*>(model);

    Q_ASSERT(model);
    Q_ASSERT(track_model);
    setVisible(false);

    // Save the previous track model's header state
    WTrackTableViewHeader* oldHeader =
            dynamic_cast<WTrackTableViewHeader*>(horizontalHeader());
    if (oldHeader) {
        oldHeader->saveHeaderState();
    }

    // rryan 12/2009 : Due to a bug in Qt, in order to switch to a model with
    // different columns than the old model, we have to create a new horizontal
    // header. Also, for some reason the WTrackTableView has to be hidden or
    // else problems occur. Since we parent the WtrackTableViewHeader's to the
    // WTrackTableView, they are automatically deleted.
    QHeaderView* header = new WTrackTableViewHeader(Qt::Horizontal, this);

    // WTF(rryan) The following saves on unnecessary work on the part of
    // WTrackTableHeaderView. setHorizontalHeader() calls setModel() on the
    // current horizontal header. If this happens on the old
    // WTrackTableViewHeader, then it will save its old state, AND do the work
    // of initializing its menus on the new model. We create a new
    // WTrackTableViewHeader, so this is wasteful. Setting a temporary
    // QHeaderView here saves on setModel() calls. Since we parent the
    // QHeaderView to the WTrackTableView, it is automatically deleted.
    QHeaderView* tempHeader = new QHeaderView(Qt::Horizontal, this);
    setHorizontalHeader(tempHeader);

    setModel(model);
    setHorizontalHeader(header);
    header->setMovable(true);
    header->setClickable(true);
    header->setHighlightSections(true);
    header->setSortIndicatorShown(true);
    //setSortingEnabled(true);
    connect(horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            this, SLOT(sortByColumn(int)), Qt::AutoConnection);

    // Initialize all column-specific things
    for (int i = 0; i < model->columnCount(); ++i) {
        //Setup delegates according to what the model tells us
        QItemDelegate* delegate = track_model->delegateForColumn(i);
        // We need to delete the old delegates, since the docs say the view will
        // not take ownership of them.
        QAbstractItemDelegate* old_delegate = itemDelegateForColumn(i);
        // If delegate is NULL, it will unset the delegate for the column
        setItemDelegateForColumn(i, delegate);
        delete old_delegate;

        // Show or hide the column based on whether it should be shown or not.
        if (track_model->isColumnInternal(i)) {
            //qDebug() << "Hiding column" << i;
            horizontalHeader()->hideSection(i);
        }
    }

    // Set up drag and drop behaviour according to whether or not the track
    // model says it supports it.

    //Defaults
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragOnly);
    setDragEnabled(true); //Always enable drag for now (until we have a model that doesn't support this.)

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_RECEIVEDROPS)) {
        setDragDropMode(QAbstractItemView::DragDrop);
        setDropIndicatorShown(true);
        setAcceptDrops(true);
        //viewport()->setAcceptDrops(true);
    }

    //Possible giant fuckup alert - It looks like Qt has something like these
    //caps built-in, see http://doc.trolltech.com/4.5/qt.html#ItemFlag-enum and
    //the flags(...) function that we're already using in LibraryTableModel. I
    //haven't been able to get it to stop us from using a model as a drag target
    //though, so my hax above may not be completely unjustified.

    setVisible(true);
}

void WTrackTableView::createActions()
{
    Q_ASSERT(m_pMenu);

    m_pPlayer1Act = new QAction(tr("Load in Player 1"),this);
    connect(m_pPlayer1Act, SIGNAL(triggered()), this, SLOT(slotLoadPlayer1()));

    m_pPlayer2Act = new QAction(tr("Load in Player 2"),this);
    connect(m_pPlayer2Act, SIGNAL(triggered()), this, SLOT(slotLoadPlayer2()));

    m_pRemoveAct = new QAction(tr("Remove"),this);
    connect(m_pRemoveAct, SIGNAL(triggered()), this, SLOT(slotRemove()));

    m_pPropertiesAct = new QAction(tr("Properties..."), this);
    connect(m_pPropertiesAct, SIGNAL(triggered()), this, SLOT(slotShowTrackInfo()));

    m_pAutoDJAct = new QAction(tr("Add to Auto DJ Queue"),this);
    connect(m_pAutoDJAct, SIGNAL(triggered()), this, SLOT(slotSendToAutoDJ()));

 	//m_pRenamePlaylistAct = new QAction(tr("Rename..."), this);
 	//connect(RenamePlaylistAct, SIGNAL(triggered()), this, SLOT(slotShowPlaylistRename()));

 	//Create all the "send to->playlist" actions.
 	//updatePlaylistActions();
}

void WTrackTableView::slotMouseDoubleClicked(const QModelIndex &index)
{
    TrackModel* trackModel = getTrackModel();
    TrackPointer pTrack;
    if (trackModel && (pTrack = trackModel->getTrack(index))) {
        emit(loadTrack(pTrack));
    }
}

void WTrackTableView::slotLoadPlayer1() {
    if (m_selectedIndices.size() > 0) {
        QModelIndex index = m_selectedIndices.at(0);
        TrackModel* trackModel = getTrackModel();
        TrackPointer pTrack;
        if (trackModel &&
            (pTrack = trackModel->getTrack(index))) {
            emit(loadTrackToPlayer(pTrack, 1));
        }
    }
}

void WTrackTableView::slotLoadPlayer2() {
    if (m_selectedIndices.size() > 0) {
        QModelIndex index = m_selectedIndices.at(0);
        TrackModel* trackModel = getTrackModel();
        TrackPointer pTrack;
        if (trackModel &&
            (pTrack = trackModel->getTrack(index))) {
            emit(loadTrackToPlayer(pTrack, 2));
        }
    }
}

void WTrackTableView::slotRemove()
{
    if (m_selectedIndices.size() > 0)
    {
        TrackModel* trackModel = getTrackModel();
        if (trackModel) {
            trackModel->removeTracks(m_selectedIndices);
        }
    }
}

void WTrackTableView::slotShowTrackInfo() {
    if (m_selectedIndices.size() == 0)
        return;

    showTrackInfo(m_selectedIndices[0]);
}

void WTrackTableView::slotNextTrackInfo() {
    QModelIndex nextRow = currentTrackInfoIndex.sibling(
        currentTrackInfoIndex.row()+1, currentTrackInfoIndex.column());
    if (nextRow.isValid())
        showTrackInfo(nextRow);
}

void WTrackTableView::slotPrevTrackInfo() {
    QModelIndex prevRow = currentTrackInfoIndex.sibling(
        currentTrackInfoIndex.row()-1, currentTrackInfoIndex.column());
    if (prevRow.isValid())
        showTrackInfo(prevRow);
}

void WTrackTableView::showTrackInfo(QModelIndex index) {
    TrackModel* trackModel = getTrackModel();

    if (!trackModel)
        return;

    TrackPointer pTrack = trackModel->getTrack(index);
    // NULL is fine.
    pTrackInfo->loadTrack(pTrack);
    currentTrackInfoIndex = index;
    pTrackInfo->show();
}

void WTrackTableView::contextMenuEvent(QContextMenuEvent * event)
{
    //Get the indices of the selected rows.
    m_selectedIndices = this->selectionModel()->selectedRows();

    //Gray out some stuff if multiple songs were selected.
    if (m_selectedIndices.count() != 1) {
        m_pPlayer1Act->setEnabled(false);
        m_pPlayer2Act->setEnabled(false);
        m_pPropertiesAct->setEnabled(false);
    } else {
        m_pPlayer1Act->setEnabled(true);
        m_pPlayer2Act->setEnabled(true);
        m_pPropertiesAct->setEnabled(true);
    }

    //Gray out player 1 and/or player 2 if those players are playing.
    if (ControlObject::getControl(ConfigKey("[Channel1]","play"))->get()==1.)
        m_pPlayer1Act->setEnabled(false);
    if (ControlObject::getControl(ConfigKey("[Channel2]","play"))->get()==1.)
        m_pPlayer2Act->setEnabled(false);

    m_pMenu->clear();

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_ADDTOAUTODJ)) {
        m_pMenu->addAction(m_pAutoDJAct);
        m_pMenu->addSeparator();
    }

    m_pMenu->addAction(m_pPlayer1Act);
    m_pMenu->addAction(m_pPlayer2Act);

    m_pMenu->addSeparator();

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_ADDTOPLAYLIST)) {
        m_pPlaylistMenu->clear();

        PlaylistDAO& playlistDao = m_pTrackCollection->getPlaylistDAO();
        int numPlaylists = playlistDao.playlistCount();
        for (int i = 0; i < numPlaylists; ++i) {
            int iPlaylistId = playlistDao.getPlaylistId(i);
            if (playlistDao.isHidden(iPlaylistId))
                continue;
            QString playlistName = playlistDao.getPlaylistName(iPlaylistId);
            // No leak because making the menu the parent means they will be
            // auto-deleted
            QAction* pAction = new QAction(playlistName, m_pPlaylistMenu);
            m_pPlaylistMenu->addAction(pAction);
            m_playlistMapper.setMapping(pAction, iPlaylistId);
            connect(pAction, SIGNAL(triggered()), &m_playlistMapper, SLOT(map()));
        }

        m_pMenu->addMenu(m_pPlaylistMenu);
    }

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_ADDTOCRATE)) {
        m_pCrateMenu->clear();

        CrateDAO& crateDao = m_pTrackCollection->getCrateDAO();
        int numCrates = crateDao.crateCount();
        for (int i = 0; i < numCrates; ++i) {
            int iCrateId = crateDao.getCrateId(i);
            // No leak because making the menu the parent means they will be
            // auto-deleted
            QAction* pAction = new QAction(crateDao.crateName(iCrateId), m_pCrateMenu);
            m_pCrateMenu->addAction(pAction);
            m_crateMapper.setMapping(pAction, iCrateId);
            connect(pAction, SIGNAL(triggered()), &m_crateMapper, SLOT(map()));
        }

        m_pMenu->addMenu(m_pCrateMenu);
    }

    m_pMenu->addSeparator();
    m_pMenu->addAction(m_pRemoveAct);
    m_pMenu->addAction(m_pPropertiesAct);

    //Create the right-click menu
    m_pMenu->popup(event->globalPos());
}

void WTrackTableView::onSearch(const QString& text) {
    TrackModel* trackModel = getTrackModel();
    if (trackModel) {
        m_searchThread.enqueueSearch(trackModel, text);
    }
}

void WTrackTableView::onSearchStarting() {
    saveVScrollBarPos();
}

void WTrackTableView::onSearchCleared() {
    restoreVScrollBarPos();
    TrackModel* trackModel = getTrackModel();
    if (trackModel) {
        m_searchThread.enqueueSearch(trackModel, "");
    }
}

void WTrackTableView::onShow()
{

}

QWidget* WTrackTableView::getWidgetForMIDIControl()
{
    return this;
}

/** Drag enter event, happens when a dragged item hovers over the track table view*/
void WTrackTableView::dragEnterEvent(QDragEnterEvent * event)
{
    //qDebug() << "dragEnterEvent" << event->mimeData()->formats();
    if (event->mimeData()->hasUrls())
    {
        if (event->source() == this) {
            if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_REORDER)) {
                event->acceptProposedAction();
            } else {
                event->ignore();
            }
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
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
        if (event->source() == this) {
            if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_REORDER)) {
                event->acceptProposedAction();
            } else {
                event->ignore();
            }
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

/** Drag-and-drop "drop" event. Occurs when something is dropped onto the track table view */
void WTrackTableView::dropEvent(QDropEvent * event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls(event->mimeData()->urls());
        QUrl url;
        QModelIndex selectedIndex; //Index of a selected track (iterator)

        //TODO: Filter out invalid URLs (eg. files that aren't supported audio filetypes, etc.)

        //Save the vertical scrollbar position. Adding new tracks and moving tracks in
        //the SQL data models causes a select() (ie. generation of a new result set),
        //which causes view to reset itself. A view reset causes the widget to scroll back
        //up to the top, which is confusing when you're dragging and dropping. :)
        saveVScrollBarPos();

        //The model index where the track or tracks are destined to go. :)
        //(the "drop" position in a drag-and-drop)
        QModelIndex destIndex = indexAt(event->pos());


        //qDebug() << "destIndex.row() is" << destIndex.row();

        //Drag and drop within this widget (track reordering)
        if (event->source() == this)
        {
            //For an invalid destination (eg. dropping a track beyond
            //the end of the playlist), place the track(s) at the end
            //of the playlist.
            if (destIndex.row() == -1) {
                int destRow = model()->rowCount() - 1;
                destIndex = model()->index(destRow, 0);
            }
            //Note the above code hides an ambiguous case when a
            //playlist is empty. For that reason, we can't factor that
            //code out to be common for both internal reordering
            //and external drag-and-drop. With internal reordering,
            //you can't have an empty playlist. :)

            //qDebug() << "track reordering" << __FILE__ << __LINE__;

            //Save a list of row (just plain ints) so we don't get screwed over
            //when the QModelIndexes all become invalid (eg. after moveTrack()
            //or addTrack())
            m_selectedIndices = this->selectionModel()->selectedRows();
            QList<int> selectedRows;
            QModelIndex idx;
            foreach (idx, m_selectedIndices)
            {
                selectedRows.append(idx.row());
            }


            /** Note: The biggest subtlety in the way I've done this track reordering code
                is that as soon as we've moved ANY track, all of our QModelIndexes probably
                get screwed up. The starting point for the logic below is to say screw it to
                the QModelIndexes, and just keep a list of row numbers to work from. That
                ends up making the logic simpler and the behaviour totally predictable,
                which lets us do nice things like "restore" the selection model.
            */

            if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_REORDER)) {
                TrackModel* trackModel = getTrackModel();

                //The model indices are sorted so that we remove the tracks from the table
                //in ascending order. This is necessary because if track A is above track B in
                //the table, and you remove track A, the model index for track B will change.
                //Sorting the indices first means we don't have to worry about this.
                //qSort(m_selectedIndices);
                //qSort(m_selectedIndices.begin(), m_selectedIndices.end(), qGreater<QModelIndex>());
                qSort(selectedRows);
                int maxRow = selectedRows.last();
                int minRow = selectedRows.first();
                int selectedRowCount = selectedRows.count();
                int firstRowToSelect = destIndex.row();

                //If you drag a contiguous selection of multiple tracks and drop
                //them somewhere inside that same selection, do nothing.
                if (destIndex.row() >= minRow && destIndex.row() <= maxRow)
                    return;

                //If we're moving the tracks _up_, then reverse the order of the row selection
                //to make the algorithm below work without added complexity.
                if (destIndex.row() < minRow) {
                    qSort(selectedRows.begin(), selectedRows.end(), qGreater<int>());
                }

                if (destIndex.row() > maxRow)
                {
                    //Shuffle the row we're going to start making a new selection at:
                    firstRowToSelect = firstRowToSelect - selectedRowCount + 1;
                }

                //For each row that needs to be moved...
                while (!selectedRows.isEmpty())
                {
                    int movedRow = selectedRows.takeFirst(); //Remember it's row index
                    //Move it
                    trackModel->moveTrack(model()->index(movedRow, 0), destIndex);

                    //Shuffle the row indices for rows that got bumped up
                    //into the void we left, or down because of the new spot
                    //we're taking.
                    for (int i = 0; i < selectedRows.count(); i++)
                    {
                        if ((selectedRows[i] > movedRow) &&
                            (destIndex.row() > selectedRows[i])) {
                            selectedRows[i] = selectedRows[i] - 1;
                        }
                        else if ((selectedRows[i] < movedRow) &&
                                 (destIndex.row() < selectedRows[i])) {
                            selectedRows[i] = selectedRows[i] + 1;
                        }
                    }
                }

                //Highlight the moved rows again (restoring the selection)
                //QModelIndex newSelectedIndex = destIndex;
                for (int i = 0; i < selectedRowCount; i++)
                {
                    this->selectionModel()->select(model()->index(firstRowToSelect + i, 0),
                                                   QItemSelectionModel::Select | QItemSelectionModel::Rows);
                }

            }
        }
        else
        {
            //Reset the selected tracks (if you had any tracks highlighted, it
            //clears them)
            this->selectionModel()->clear();

            //Drag-and-drop from an external application
            //eg. dragging a track from Windows Explorer onto the track table.
            TrackModel* trackModel = getTrackModel();
            if (trackModel) {
                int numNewRows = urls.count(); //XXX: Crappy, assumes all URLs are valid songs.
                                               //     Should filter out invalid URLs at the start of this function.

                int selectionStartRow = destIndex.row();  //Have to do this here because the index is invalid after addTrack

                //Make a new selection starting from where the first track was dropped, and select
                //all the dropped tracks

                //If the track was dropped into an empty playlist, start at row 0 not -1 :)
                if ((destIndex.row() == -1) && (model()->rowCount() == 0))
                {
                    selectionStartRow = 0;
                }
                //If the track was dropped beyond the end of a playlist, then we need
                //to fudge the destination a bit...
                else if ((destIndex.row() == -1) && (model()->rowCount() > 0))
                {
                    //qDebug() << "Beyond end of playlist";
                    //qDebug() << "rowcount is:" << model()->rowCount();
                    selectionStartRow = model()->rowCount();
                }

                //Add all the dropped URLs/tracks to the track model (playlist/crate)
                foreach (url, urls)
                {
                    QFileInfo file(url.toLocalFile());
                    if (!trackModel->addTrack(destIndex, file.absoluteFilePath()))
                        numNewRows--; //# of rows to select must be decremented if we skipped some tracks
                }

                //Create the selection, but only if the track model supports reordering.
                //(eg. crates don't support reordering/indexes)
                if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_REORDER)) {
                    for (int i = selectionStartRow; i < selectionStartRow + numNewRows; i++)
                    {
                        this->selectionModel()->select(model()->index(i, 0), QItemSelectionModel::Select |
                                                    QItemSelectionModel::Rows);
                    }
                }
            }
        }

        event->acceptProposedAction();

        restoreVScrollBarPos();

    } else {
        event->ignore();
    }
}

TrackModel* WTrackTableView::getTrackModel() {
    TrackModel* trackModel = dynamic_cast<TrackModel*>(model());
    return trackModel;
}

bool WTrackTableView::modelHasCapabilities(TrackModel::CapabilitiesFlags capabilities) {
    TrackModel* trackModel = getTrackModel();
    return trackModel &&
            (trackModel->getCapabilities() & capabilities) == capabilities;
}

void WTrackTableView::keyPressEvent(QKeyEvent* event)
{
    
    if (event->key() == Qt::Key_Return)
    {
		/*
		 * It is not a good idea if 'key_return'
		 * causes a track to load since we allow in-line editing
		 * of table items in general
		 */
        return;
    }
    else if (event->key() == Qt::Key_BracketLeft)
    {
        slotLoadPlayer1();
    }
    else if (event->key() == Qt::Key_BracketRight)
    {
        slotLoadPlayer2();
    }
    else
        QTableView::keyPressEvent(event);
}

void WTrackTableView::slotSendToAutoDJ() {
    if (!modelHasCapabilities(TrackModel::TRACKMODELCAPS_ADDTOAUTODJ))
        return;

    PlaylistDAO& playlistDao = m_pTrackCollection->getPlaylistDAO();
    int iAutoDJPlaylistId = playlistDao.getPlaylistIdFromName(AUTODJ_TABLE);

    if (iAutoDJPlaylistId == -1)
        return;

    TrackModel* trackModel = getTrackModel();
    foreach (QModelIndex index, m_selectedIndices) {
        TrackPointer pTrack;
        if (trackModel &&
            (pTrack = trackModel->getTrack(index))) {
            int iTrackId = pTrack->getId();
            if (iTrackId != -1) {
                playlistDao.appendTrackToPlaylist(iTrackId, iAutoDJPlaylistId);
            }
        }
    }
}

void WTrackTableView::addSelectionToPlaylist(int iPlaylistId) {
    PlaylistDAO& playlistDao = m_pTrackCollection->getPlaylistDAO();
    TrackModel* trackModel = getTrackModel();

    foreach (QModelIndex index, m_selectedIndices) {
        TrackPointer pTrack;
        if (trackModel &&
            (pTrack = trackModel->getTrack(index))) {
            int iTrackId = pTrack->getId();
            if (iTrackId != -1) {
                playlistDao.appendTrackToPlaylist(iTrackId, iPlaylistId);
            }
        }
    }
}

void WTrackTableView::addSelectionToCrate(int iCrateId) {
    CrateDAO& crateDao = m_pTrackCollection->getCrateDAO();
    TrackModel* trackModel = getTrackModel();

    foreach (QModelIndex index, m_selectedIndices) {
        TrackPointer pTrack;
        if (trackModel &&
            (pTrack = trackModel->getTrack(index))) {
            int iTrackId = pTrack->getId();
            if (iTrackId != -1) {
                crateDao.addTrackToCrate(iTrackId, iCrateId);
            }
        }
    }
}
