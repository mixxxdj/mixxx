//
// C++ Interface: mixxxmenuplaylists
//
// Description: 
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MIXXXMENUPLAYLISTS_H
#define MIXXXMENUPLAYLISTS_H

#include <qptrlist.h>
#include "trackplaylist.h"
#include <qobject.h>
#include <qaction.h>

class QPopupMenu;
class Track;
class TrackPlaylistList;

/**
Object keeping track of the playlists listed in the menu

@author Tue Haste Andersen
*/
class MixxxMenuPlaylists : public QObject
{
    Q_OBJECT
    
    typedef struct menuItem_t { 
        int id; 
        TrackPlaylist *pTrackPlaylist;
    } menuItem_t;

public:
    MixxxMenuPlaylists(QPopupMenu *pMenu, Track *pTrack);
    ~MixxxMenuPlaylists();
    
public slots:
    void slotUpdate(TrackPlaylistList *pPlaylists);
    /** When a menu item is activated, this slot is called to request the activation of the corresponding playlist */
    void slotRequestActive(int id);
    /** This slot is called from Track when a playlist has been activated. */
    void slotSetActive(TrackPlaylist *pTrackPlaylist);
    
private:
    QPopupMenu *m_pMenu;
    Track *m_pTrack;
    QPtrList<menuItem_t> m_qMenuList;
};

#endif
