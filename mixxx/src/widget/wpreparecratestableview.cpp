#include <QHeaderView>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include "library/trackcollection.h"
#include "library/dao/trackdao.h"
#include "library/dao/cratedao.h"
#include "library/preparecratedelegate.h"
#include "wpreparecratestableview.h"


WPrepareCratesTableView::WPrepareCratesTableView(QWidget* parent, TrackCollection* pTrackCollection) : QTableView(parent)
{
    m_pTrackCollection = pTrackCollection;
    setAcceptDrops(true);
    setShowGrid(false);
    setSelectionMode(QAbstractItemView::NoSelection);

    //Hack the max height into here so it doesn't go giant
    setMaximumHeight(51);

    horizontalHeader()->hide();
    verticalHeader()->hide();

    setItemDelegate(new PrepareCrateDelegate(this));
}

WPrepareCratesTableView::~WPrepareCratesTableView()
{
}


/** Drag enter event, happens when a dragged item hovers over the track table view*/
void WPrepareCratesTableView::dragEnterEvent(QDragEnterEvent * event)
{
    //qDebug() << "dragEnterEvent" << event->mimeData()->formats();
    if (event->mimeData()->hasUrls())
    {
        if (event->source() == this) {
            event->ignore();
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
void WPrepareCratesTableView::dragMoveEvent(QDragMoveEvent * event)
{
    //qDebug() << "dragMoveEvent" << event->mimeData()->formats();
    if (event->mimeData()->hasUrls())
    {
        if (event->source() == this) {
            event->ignore();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

/** Drag-and-drop "drop" event. Occurs when something is dropped onto the track table view */
void WPrepareCratesTableView::dropEvent(QDropEvent * event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls(event->mimeData()->urls());
        QUrl url;
        QModelIndex destIndex; 

        //Drag and drop within this widget (crate reordering)
        if (event->source() == this)
        {
            event->ignore();
        }
        else
        {
            //Drag-and-drop from an external widget
            
            destIndex = this->indexAt(event->pos());
            foreach (url, urls)
            {
                QModelIndex destIndex = this->indexAt(event->pos());
                QString crateName = destIndex.data().toString(); 
                int crateId = m_pTrackCollection->getCrateDAO().getCrateIdByName(crateName);
                int trackId = m_pTrackCollection->getTrackDAO().getTrackId(url.toLocalFile());
                if (trackId >= 0)
                    m_pTrackCollection->getCrateDAO().addTrackToCrate(trackId, crateId);
            }
        }

        event->acceptProposedAction();

    } else {
        event->ignore();
    }
}
