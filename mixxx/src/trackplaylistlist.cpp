//
// C++ Implementation: trackplaylistlist
//
// Description: 
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "trackplaylistlist.h"

TrackPlaylistList::TrackPlaylistList()
{
}


TrackPlaylistList::~TrackPlaylistList()
{
}

int TrackPlaylistList::compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2)
{
    TrackPlaylist *p1 = (TrackPlaylist *)item1;
    TrackPlaylist *p2 = (TrackPlaylist *)item2;

    if (p1->getListName()==p2->getListName())
        return 0;
    else if (p1->getListName()>p2->getListName())
        return 1;
    else
        return -1;
}
