//
// C++ Interface: wtreeitemplaylistroot
//
// Description: 
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WTREEITEMPLAYLISTROOT_H
#define WTREEITEMPLAYLISTROOT_H

#include "wtreeitem.h"


/**
@author Tue Haste Andersen
*/
class WTreeItemPlaylistRoot : public WTreeItem
{
public:
    WTreeItemPlaylistRoot(Q3ListView *parent, const QString &s1);
    ~WTreeItemPlaylistRoot();

    void popupMenu();
    void drag(QWidget *viewport); 
};

#endif
