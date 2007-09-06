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

#include <QList>
#include <QMenu>
#include "trackplaylist.h"
#include <QObject>
#include <QAction>

class QMenu;
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
    MixxxMenuPlaylists(QMenu *pMenu, Track *pTrack);
    ~MixxxMenuPlaylists();

public slots:
    void slotUpdate(TrackPlaylistList *pPlaylists);
    /** When a menu item is activated, this slot is called to request the activation of the corresponding playlist */
    void slotRequestActive(int id);
    /** This slot is called from Track when a playlist has been activated. */
    void slotSetActive(TrackPlaylist *pTrackPlaylist);

private:
    QMenu *m_pMenu;
    Track *m_pTrack;
    QList<menuItem_t *> m_qMenuList;
};

#endif
