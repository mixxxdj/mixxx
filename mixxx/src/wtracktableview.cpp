#include <QItemDelegate>
#include <QtCore>
#include <QtGui>
#include <QtXml>

#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "library/trackmodel.h"
#include "library/librarytablemodel.h"
#include "trackinfoobject.h"
#include "controlobject.h"
#include "wtracktableview.h"

WTrackTableView::WTrackTableView(QWidget * parent,
                                 ConfigObject<ConfigValue> * pConfig)
        : WLibraryTableView(parent, pConfig),
          m_pConfig(pConfig) {

    //Disable editing
    //setEditTriggers(QAbstractItemView::NoEditTriggers);

    //Drag and drop setup
    //TODO: Enable this stuff, make it work with new sqlite library.
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDropIndicatorShown(true);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);

    //Create all the context menu actions (stuff that shows up when you right-click)
    createActions();

    //Connect slots and signals to make the world go 'round.
    connect(this, SIGNAL(doubleClicked(const QModelIndex &)),
            this, SLOT(slotMouseDoubleClicked(const QModelIndex &)));

}

WTrackTableView::~WTrackTableView()
{
 	delete m_pPlayQueueAct;
 	delete m_pPlayer1Act;
 	delete m_pPlayer2Act;
 	delete m_pRemoveAct;
 	delete m_pPropertiesAct;
 	//delete m_pRenamePlaylistAct;
}

ConfigKey WTrackTableView::getHeaderStateKey() {
    return ConfigKey(LIBRARY_CONFIGVALUE,
                     WTRACKTABLEVIEW_HEADERSTATE_KEY);
}

ConfigKey WTrackTableView::getVScrollBarPosKey() {
    return ConfigKey(LIBRARY_CONFIGVALUE,
                     WTRACKTABLEVIEW_VSCROLLBARPOS_KEY);
}

void WTrackTableView::loadTrackModel(QAbstractItemModel *model) {
    qDebug() << "WTrackTableView::loadTrackModel()" << model;

    TrackModel* track_model = dynamic_cast<TrackModel*>(model);

    Q_ASSERT(model);
    Q_ASSERT(track_model);

    setModel(model);

    for (int i = 0; i < model->columnCount(); ++i) {
        QItemDelegate* delegate = track_model->delegateForColumn(i);
        // We need to delete the old delegates, since the docs say the view will
        // not take ownership of them.
        QAbstractItemDelegate* old_delegate = itemDelegateForColumn(i);
        // If delegate is NULL, it will unset the delegate for the column
        setItemDelegateForColumn(i, delegate);
        delete old_delegate;
    }
}

void WTrackTableView::createActions()
{
    m_pPlayer1Act = new QAction(tr("Load in Player 1"),this);
    connect(m_pPlayer1Act, SIGNAL(triggered()), this, SLOT(slotLoadPlayer1()));

    m_pPlayer2Act = new QAction(tr("Load in Player 2"),this);
    connect(m_pPlayer2Act, SIGNAL(triggered()), this, SLOT(slotLoadPlayer2()));

    m_pRemoveAct = new QAction(tr("Remove"),this);
    connect(m_pRemoveAct, SIGNAL(triggered()), this, SLOT(slotRemove()));

 	m_pPropertiesAct = new QAction(tr("Properties..."), this);
 	//connect(m_pPropertiesAct, SIGNAL(triggered()), this, SLOT(slotShowBPMTapDlg()));

    m_pPlayQueueAct = new QAction(tr("Add to Play Queue"),this);
    //connect(m_pPlayQueueAct, SIGNAL(triggered()), this, SLOT(slotSendToPlayqueue()));

 	//m_pRenamePlaylistAct = new QAction(tr("Rename..."), this);
 	//connect(RenamePlaylistAct, SIGNAL(triggered()), this, SLOT(slotShowPlaylistRename()));

 	//Create all the "send to->playlist" actions.
 	//updatePlaylistActions();
}

void WTrackTableView::slotMouseDoubleClicked(const QModelIndex &index)
{
    TrackModel* trackModel = dynamic_cast<TrackModel*>(model());
    TrackInfoObject* pTrack = NULL;
    if (trackModel && (pTrack = trackModel->getTrack(index))) {
        emit(loadTrack(pTrack));
    }
}

void WTrackTableView::slotLoadPlayer1() {
    if (m_selectedIndices.size() > 0) {
        TrackModel* trackModel = dynamic_cast<TrackModel*>(model());
        TrackInfoObject* pTrack = NULL;
        if (trackModel &&
            (pTrack = trackModel->getTrack(m_selectedIndices.at(0)))) {
            emit(loadTrackToPlayer(pTrack, 1));
        }
    }
}

void WTrackTableView::slotLoadPlayer2() {
 	if (m_selectedIndices.size() > 0) {
        TrackModel* trackModel = dynamic_cast<TrackModel*>(model());
        TrackInfoObject* pTrack = NULL;
        if (trackModel &&
            (pTrack = trackModel->getTrack(m_selectedIndices.at(0)))) {
            emit(loadTrackToPlayer(pTrack, 2));
        }
    }
}

void WTrackTableView::slotRemove()
{
 	if (m_selectedIndices.size() > 0)
        {
            TrackModel* trackModel = dynamic_cast<TrackModel*>(model());
            if (trackModel) {
                QModelIndex curIndex;
                //The model indices are sorted so that we remove the tracks from the table
                //in ascending order. This is necessary because if track A is above track B in
                //the table, and you remove track A, the model index for track B will change.
                //Sorting the indices first means we don't have to worry about this.
                qSort(m_selectedIndices);

                //Going through the model indices in descending order (see above comment for explanation).
                QListIterator<QModelIndex> it(m_selectedIndices);
                it.toBack();
                while (it.hasPrevious())
                    {
                        curIndex = it.previous();
                        trackModel->removeTrack(curIndex);
                    }
            }
        }
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

    //Create the right-click menu
    QMenu menu(this);
    menu.addAction(m_pPlayer1Act);
    menu.addAction(m_pPlayer2Act);
    menu.addSeparator();
    menu.addAction(m_pRemoveAct);
    menu.exec(event->globalPos());
}

/** Drag enter event, happens when a dragged item hovers over the track table view*/
void WTrackTableView::dragEnterEvent(QDragEnterEvent * event)
{
    //qDebug() << "dragEnterEvent" << event->mimeData()->formats();
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

        //Drag and drop within this widget (track reordering)
        if (event->source() == this && event->possibleActions() & Qt::MoveAction)
        {
/*
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
      	*/

        }
        else
        {
            //Reset the selected tracks (if you had any tracks highlighted, it clears them)
            this->selectionModel()->clear();

            //Drag-and-drop from an external application
            //eg. dragging a track from Windows Explorer onto the track table.

            TrackModel* trackModel = dynamic_cast<TrackModel*>(model());
			if (trackModel) {
	            foreach (url, urls)
	            {
	                QModelIndex destIndex = this->indexAt(event->pos());
	                //TrackInfoObject* draggedTrack = m_pTrack->getTrackCollection()->getTrack(url.toLocalFile());
	                //if (draggedTrack) //Make sure the track was valid
	                //{
	                //if (model()->insertRow(destIndex.row(), url.toLocalFile()))
	                trackModel->addTrack(destIndex, url.toLocalFile());
	                {
	                    //this->selectionModel()->select(destIndex, QItemSelectionModel::Select |
	                    //                                          QItemSelectionModel::Rows);
	                }
	                //}
	            }
			}
        }

        event->acceptProposedAction();
        //emit(trackDropped(name));

        //repaintEverything();
    } else
        event->ignore();
}
