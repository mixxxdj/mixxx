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
#include <QMenu>
#include <QtDebug>
#include "track.h"
#include "trackplaylistlist.h"

MixxxMenuPlaylists::MixxxMenuPlaylists(QMenu * pMenu, Track * pTrack)
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

void MixxxMenuPlaylists::slotUpdate(TrackPlaylistList * pPlaylists)
{
    int i = 0;
    menuItem_t * it;

    qDebug("update menu");

    // Delete all items currently in the menu
    for (int i = 0; i < m_qMenuList.size(); ++i) {
        it = m_qMenuList[i];
        m_pMenu->removeItem(it->id);
        m_qMenuList.removeAt(i);
    }

    // Add items in qPlaylists to the menu
    QListIterator<TrackPlaylist*> it2(*pPlaylists);
    TrackPlaylist* current;
    while (it2.hasNext())
    {
        current = it2.next();
        menuItem_t *p = new menuItem_t;
        m_qMenuList.append(p);
        if (current)
        {
            p->pTrackPlaylist = current;
            p->id = m_pMenu->insertItem(current->getListName());
        }
    }
}

void MixxxMenuPlaylists::slotRequestActive(int id)
{
    //qDebug("request active %i",id);

    // Activate the playlist in the menu with the given id
    int i;
    QString name;
    menuItem_t * it;

    for (int i = 0; i < m_qMenuList.size(); ++i) {
        it = m_qMenuList[i];
        if (it->id==id) {
            m_pTrack->slotActivatePlaylist(it->pTrackPlaylist->getListName());
            break;
        }
    }
}

void MixxxMenuPlaylists::slotSetActive(TrackPlaylist * pTrackPlaylist)
{
    int i;
    m_pMenu->setItemChecked(m_pMenu->idAt(0), false);
    m_pMenu->setItemChecked(m_pMenu->idAt(1), false);

    //qDebug() << "set active" << pTrackPlaylist->getListName();

    menuItem_t * it;
    for (int i = 0; i < m_qMenuList.size(); ++i) {
        it = m_qMenuList[i];
        if (it->pTrackPlaylist==pTrackPlaylist)
            m_pMenu->setItemChecked(it->id, true);
        else
            m_pMenu->setItemChecked(it->id, false);
    }
}

