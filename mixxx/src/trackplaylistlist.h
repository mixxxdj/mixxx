//
// C++ Interface: trackplaylistlist
//
// Description: 
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TRACKPLAYLISTLIST_H
#define TRACKPLAYLISTLIST_H

#include <q3ptrlist.h>
#include "trackplaylist.h"

/**
@author Tue Haste Andersen
*/
class TrackPlaylistList : public Q3PtrList<TrackPlaylist>
{
public:
    TrackPlaylistList();
    ~TrackPlaylistList();

protected:
    int compareItems(Q3PtrCollection::Item item1, Q3PtrCollection::Item item2);
};

#endif
