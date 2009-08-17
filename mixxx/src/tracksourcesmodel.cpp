

#include <QtCore>
#include <QtGui>
#include "tracksourcesmodel.h"

TrackSourcesModel::TrackSourcesModel() : QStandardItemModel()
{
    //TODO: Eventually a pointer to the list of playlists
    //      needs to be passed in here.

    m_pLibraryItem = new QStandardItem(tr("Library"));
    m_pCheeseburgerItem = new QStandardItem(QIcon(":/images/library/rhythmbox.png"), tr("Cheeseburger"));

    m_pRhythmboxLibraryItem = new QStandardItem(tr("Liberry"));
    m_pCheeseburgerItem->appendRow(m_pRhythmboxLibraryItem);
    
    m_pLibraryItem->setEditable(false);
    m_pCheeseburgerItem->setEditable(false);
    m_pRhythmboxLibraryItem->setEditable(false);

    appendRow(m_pLibraryItem);
	appendRow(m_pCheeseburgerItem);		
}

TrackSourcesModel::~TrackSourcesModel()
{
    delete m_pLibraryItem;
    delete m_pCheeseburgerItem;
}
