
#include <QtCore>
#include <QtGui>
#include "wtracksourcesview.h"

WTrackSourcesView::WTrackSourcesView() : QTreeView()
{
    //Set some properties
    setHeaderHidden(true);
    
    //Connect some signals
    connect(this, SIGNAL(activated(const QModelIndex&)), 
            this, SLOT(activatedSignalProxy(const QModelIndex&)));
    connect(this, SIGNAL(clicked(const QModelIndex&)), 
            this, SLOT(activatedSignalProxy(const QModelIndex&)));
}

WTrackSourcesView::~WTrackSourcesView()
{

}

void WTrackSourcesView::activatedSignalProxy(const QModelIndex& index)
{
    if (model()->data(index).toString() == tr("Library"))
    {
        emit(libraryItemActivated());
    }
    else if (model()->data(index).toString() == tr("Cheeseburger"))
    {
        emit(cheeseburgerItemActivated());
    }
}

