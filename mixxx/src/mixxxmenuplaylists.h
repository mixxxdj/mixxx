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

#include <q3ptrlist.h>
//Added by qt3to4:
#include <Q3PopupMenu>
#include "trackplaylist.h"
#include <qobject.h>
#include <qaction.h>

class Q3PopupMenu;
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
    MixxxMenuPlaylists(Q3PopupMenu *pMenu, Track *pTrack);
    ~MixxxMenuPlaylists();
    
public slots:
    void slotUpdate(TrackPlaylistList *pPlaylists);
    /** When a menu item is activated, this slot is called to request the activation of the corresponding playlist */
    void slotRequestActive(int id);
    /** This slot is called from Track when a playlist has been activated. */
    void slotSetActive(TrackPlaylist *pTrackPlaylist);
    
private:
    Q3PopupMenu *m_pMenu;
    Track *m_pTrack;
    Q3PtrList<menuItem_t> m_qMenuList;
};

#endif
