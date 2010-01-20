#include <QtCore>
#include <QtGui>
#include "library/sidebarmodel.h"
#include "widget/wlibrarysidebar.h"

WLibrarySidebar::WLibrarySidebar(QWidget* parent) : QTreeView(parent) {
    //Set some properties
    setHeaderHidden(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    //Drag and drop setup
    setDragEnabled(false);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDropIndicatorShown(true);
    setAcceptDrops(true);
}

WLibrarySidebar::~WLibrarySidebar() {
}


void WLibrarySidebar::contextMenuEvent(QContextMenuEvent *event)
{
    QModelIndex clickedItem = indexAt(event->pos());
    emit(rightClicked(event->globalPos(), clickedItem));
}

/** Drag enter event, happens when a dragged item enters the track sources view*/
void WLibrarySidebar::dragEnterEvent(QDragEnterEvent * event)
{
    qDebug() << "WLibrarySidebar::dragEnterEvent" << event->mimeData()->formats();
    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

/** Drag move event, happens when a dragged item hovers over the track sources view...
 */
void WLibrarySidebar::dragMoveEvent(QDragMoveEvent * event)
{
    //qDebug() << "dragMoveEvent" << event->mimeData()->formats();

    if (event->mimeData()->hasUrls())
    {
        QList<QUrl> urls(event->mimeData()->urls());

        QUrl url;

        //Drag and drop within this widget
        if (event->source() == this && event->possibleActions() & Qt::MoveAction)
        {
            //Do nothing.
             event->ignore();
        }
        else
        {
            SidebarModel* sidebarModel = dynamic_cast<SidebarModel*>(model());
            bool accepted = true;
            if (sidebarModel) {
                foreach (url, urls)
                {
                    QModelIndex destIndex = this->indexAt(event->pos());
                    if (!sidebarModel->dragMoveAccept(destIndex, url))
                    {
                        //We only need one URL to be invalid for us
                        //to reject the whole drag...
                        //(eg. you may have tried to drag two MP3's and an EXE)
                        accepted = false;
                        break;
                    }
                }
            }
            if (accepted)
                event->acceptProposedAction();
            else
                event->ignore();
        }
    }
    else
        event->ignore();
}

/** Drag-and-drop "drop" event. Occurs when something is dropped onto the track sources view */
void WLibrarySidebar::dropEvent(QDropEvent * event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls(event->mimeData()->urls());
        QUrl url;

        //Drag and drop within this widget
        if (event->source() == this && event->possibleActions() & Qt::MoveAction)
        {
            //Do nothing.
             event->ignore();
        }
        else
        {
            //Reset the selected items (if you had anything highlighted, it clears it)
            //this->selectionModel()->clear();

            //Drag-and-drop from an external application or the track table widget
            //eg. dragging a track from Windows Explorer onto the sidebar

            SidebarModel* sidebarModel = dynamic_cast<SidebarModel*>(model());
            bool accepted = false;
            if (sidebarModel) {
                foreach (url, urls)
                {
                    //qDebug() << "dropEvent" << url;
                    QModelIndex destIndex = indexAt(event->pos());
                    if (sidebarModel->dropAccept(destIndex, url))
                    {
                        accepted = true;

                    }
                }
            }

            if (accepted)
                event->acceptProposedAction();
            else
                event->ignore();
        }


        //emit(trackDropped(name));

        //repaintEverything();
    } else {
        event->ignore();
    }
}

void WLibrarySidebar::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return)
    {
        QModelIndexList selectedIndices = this->selectionModel()->selectedRows();
        if (selectedIndices.size() > 0) {
            QModelIndex index = selectedIndices.at(0);
            emit(activated(index));
            //Expand or collapse the item as necessary.
            setExpanded(index, !isExpanded(index));
        }
    }
    else if (event->key() == Qt::Key_Down || event->key() == Qt::Key_Up)
    {
        //Let the tree view move up and down for us...
        QTreeView::keyPressEvent(event);
        //But force the index to be activated/clicked after the selection
        //changes. (Saves you from having to push "enter" after changing the selection.)
        QModelIndexList selectedIndices = this->selectionModel()->selectedRows();
        //Note: have to get the selected indices _after_ QTreeView::keyPressEvent()
        if (selectedIndices.size() > 0) {
            QModelIndex index = selectedIndices.at(0);
            emit(activated(index));
        }
    }
    else
        QTreeView::keyPressEvent(event);
}
