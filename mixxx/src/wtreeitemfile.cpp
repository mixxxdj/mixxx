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
#include <qcursor.h>
#include <qpopupmenu.h>
#include <qdragobject.h>
#include "track.h"
#include "wtreeitemfile.h"
#include "wtreeview.h"

WTreeItemFile::WTreeItemFile(WTreeItem *parent, const QString &s1, const QString &s2) : WTreeItem( parent, s1, s2 )
{
}

WTreeItemFile::~WTreeItemFile()
{
}

void WTreeItemFile::popupMenu()
{
    QPopupMenu *menu = new QPopupMenu();
    menu->insertItem("Player 1", this, SLOT(slotLoadPlayer1()));
    menu->insertItem("Player 2", this, SLOT(slotLoadPlayer2()));
    menu->exec(QCursor::pos());
}


void WTreeItemFile::slotLoadPlayer1()
{
    spTrack->slotLoadPlayer1(WTreeView::fullPath(this));
}

void WTreeItemFile::slotLoadPlayer2()
{
    spTrack->slotLoadPlayer2(WTreeView::fullPath(this));
}
