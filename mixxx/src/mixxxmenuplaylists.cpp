//
// C++ Implementation: mixxxmenuplaylists
//
// Description: 
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "mixxxmenuplaylists.h"
#include <qpopupmenu.h>
#include "track.h"
#include "trackplaylistlist.h"

MixxxMenuPlaylists::MixxxMenuPlaylists(QPopupMenu *pMenu, Track *pTrack)
{
    m_pMenu = pMenu;
    m_pTrack = pTrack;
    connect(m_pTrack, SIGNAL(updateMenu(const TrackPlaylistList &)), this, SLOT(slotUpdate(const TrackPlaylistList &)));
    connect(m_pTrack, SIGNAL(activePlaylist(TrackPlaylist *)), this, SLOT(slotSetActive(TrackPlaylist *)));
    connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotRequestActive(int)));
    m_pTrack->updatePlaylistViews();
}

MixxxMenuPlaylists::~MixxxMenuPlaylists()
{
}

void MixxxMenuPlaylists::slotUpdate(const TrackPlaylistList &qPlaylists)
{
    //qDebug("update menu");
    
    // Delete all items currently in the menu
    QPtrList<menuItem_t>::iterator it = m_qMenuList.begin();
    while (it!=m_qMenuList.end())
    {    
        m_pMenu->removeItem((*it)->id);
        m_qMenuList.remove((*it));
        it = m_qMenuList.begin();
    }
    
    // Add items in qPlaylists to the menu
    TrackPlaylistList::iterator it2 = qPlaylists.begin();
    while (it2!=qPlaylists.end())
    {
        menuItem_t *p = new menuItem_t;
        m_qMenuList.append(p);
        p->pTrackPlaylist = (*it2);
        p->id = m_pMenu->insertItem((*it2)->getListName());
        ++it2;
    }
}

void MixxxMenuPlaylists::slotRequestActive(int id)
{
    //qDebug("request active %i",id);
    
    // Activate the playlist in the menu with the given id
    QString name;
    QPtrList<menuItem_t>::iterator it = m_qMenuList.begin();
    while (it!=m_qMenuList.end())
    {
        if ((*it)->id==id)
        {
            m_pTrack->slotActivatePlaylist((*it)->pTrackPlaylist->getListName());
            break;
        }
        ++it;
    }
}

void MixxxMenuPlaylists::slotSetActive(TrackPlaylist *pTrackPlaylist)
{
    m_pMenu->setItemChecked(m_pMenu->idAt(0), false);
    m_pMenu->setItemChecked(m_pMenu->idAt(1), false);
    
    //qDebug("set active %s",pTrackPlaylist->getListName().latin1());
       
    QPtrList<menuItem_t>::iterator it = m_qMenuList.begin();
    while (it!=m_qMenuList.end())
    {
        if ((*it)->pTrackPlaylist==pTrackPlaylist)
            m_pMenu->setItemChecked((*it)->id, true);
        else            
            m_pMenu->setItemChecked((*it)->id, false);
        ++it;
    }
}

