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

#include <qptrlist.h>
#include "trackplaylist.h"

/**
@author Tue Haste Andersen
*/
class TrackPlaylistList : public QPtrList<TrackPlaylist>
{
public:
    TrackPlaylistList();
    ~TrackPlaylistList();

protected:
    int compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2);
};

#endif
