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
    connect(m_pTrack, SIGNAL(updateMenu(TrackPlaylistList *)), this, SLOT(slotUpdate(TrackPlaylistList *)));
    connect(m_pTrack, SIGNAL(activePlaylist(TrackPlaylist *)), this, SLOT(slotSetActive(TrackPlaylist *)));
    connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotRequestActive(int)));
    m_pTrack->updatePlaylistViews();
}

MixxxMenuPlaylists::~MixxxMenuPlaylists()
{
}

void MixxxMenuPlaylists::slotUpdate(TrackPlaylistList *pPlaylists)
{
    //qDebug("update menu");
    
    // Delete all items currently in the menu
    menuItem_t *it = m_qMenuList.first();
    while (it)
    {    
        m_pMenu->removeItem(it->id);
        m_qMenuList.remove(it);
        it = m_qMenuList.first();
    }
    
    // Add items in qPlaylists to the menu
    TrackPlaylist *it2 = pPlaylists->first();
    while (it2)
    {
        menuItem_t *p = new menuItem_t;
        m_qMenuList.append(p);
        p->pTrackPlaylist = it2;
        p->id = m_pMenu->insertItem(it2->getListName());
        it2 = pPlaylists->next();
    }
}

void MixxxMenuPlaylists::slotRequestActive(int id)
{
    //qDebug("request active %i",id);
    
    // Activate the playlist in the menu with the given id
    QString name;
    menuItem_t *it = m_qMenuList.first();
    while (it)
    {
        if (it->id==id)
        {
            m_pTrack->slotActivatePlaylist(it->pTrackPlaylist->getListName());
            break;
        }
        it = m_qMenuList.next();
    }
}

void MixxxMenuPlaylists::slotSetActive(TrackPlaylist *pTrackPlaylist)
{
    m_pMenu->setItemChecked(m_pMenu->idAt(0), false);
    m_pMenu->setItemChecked(m_pMenu->idAt(1), false);
    
    //qDebug("set active %s",pTrackPlaylist->getListName().latin1());
       
    menuItem_t *it = m_qMenuList.first();
    while (it)
    {
        if (it->pTrackPlaylist==pTrackPlaylist)
            m_pMenu->setItemChecked(it->id, true);
        else            
            m_pMenu->setItemChecked(it->id, false);
        it = m_qMenuList.next();
    }
}

