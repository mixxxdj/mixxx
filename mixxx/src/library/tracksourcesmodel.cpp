

#include <QtCore>
#include <QtGui>
#include "tracksourcesmodel.h"

TrackSourcesModel::TrackSourcesModel(RhythmboxTrackModel *rhythmboxtracks, RhythmboxPlaylistModel *rhythmboxplaylists) : QStandardItemModel()
{
    //TODO: Eventually a pointer to the list of playlists
    //      needs to be passed in here.
    QList<QString> m_sPlaylists;
    
    
    m_pLibraryItem = new QStandardItem(tr("Library"));
    m_pCheeseburgerItem = new QStandardItem(QIcon(":/images/library/rhythmbox.png"), tr("Cheeseburger"));

    m_pRhythmboxLibraryItem = new QStandardItem(tr("Liberry"));
    m_pCheeseburgerItem->appendRow(m_pRhythmboxLibraryItem);
    
    m_sPlaylists = rhythmboxplaylists->getPlaylists();
    for (int i = 0; i < m_sPlaylists.size(); i++)
    {
        qDebug() << "Rendering Playlist:" << m_sPlaylists.at(i);
        
        QStandardItem *playlist = new QStandardItem(m_sPlaylists.at(i));
        
        playlist->setEditable(false);
        m_pCheeseburgerItem->appendRow(playlist);
    }
    
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
