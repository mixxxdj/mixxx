//
// C++ Implementation: wtreeitemfile
//
// Description: 
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "wtreeitemfile.h"
#include <qdragobject.h>

WTreeItemFile::WTreeItemFile(WTreeItem *parent, const QString &s1, const QString &s2) : WTreeItem( parent, s1, s2 )
{
}

WTreeItemFile::~WTreeItemFile()
{
}

void WTreeItemFile::popupMenu()
{
    qDebug("popup file");
}