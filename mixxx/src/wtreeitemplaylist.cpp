//
// C++ Implementation: wtreeitemplaylist
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "wtreeitemplaylist.h"
#include <qpopupmenu.h>
#include <qdragobject.h>
#include <qcstring.h>

WTreeItemPlaylist::WTreeItemPlaylist(WTreeItem *parent, const QString &s1) : WTreeItem(parent, s1)
{
    setRenameEnabled(0, true);
}

WTreeItemPlaylist::~WTreeItemPlaylist()
{
}

void WTreeItemPlaylist::popupMenu()
{
}

void WTreeItemPlaylist::drag(QWidget *viewport)
{
    QString s = text(0);
    QTextDrag *td = new QTextDrag(s, viewport);
    const QCString type("Playlist");
    td->setSubtype(type);
    td->dragCopy();
}
