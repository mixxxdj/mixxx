//
// C++ Interface: wtreeitemfile
//
// Description: 
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WTREEITEMFILE_H
#define WTREEITEMFILE_H

#include "wtreeitem.h"

/**
@author Tue Haste Andersen
*/
class WTreeItemFile : public WTreeItem
{
public:
    WTreeItemFile(WTreeItem *parent, const QString &s1, const QString &s2 );
    ~WTreeItemFile();

    void popupMenu();
};

#endif
