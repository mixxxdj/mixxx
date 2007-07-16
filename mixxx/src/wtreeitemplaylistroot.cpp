//
// C++ Implementation: wtreeitemplaylistroot
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <q3popupmenu.h>
#include <qcursor.h>
#include "wtreeitemplaylistroot.h"
#include "track.h"

WTreeItemPlaylistRoot::WTreeItemPlaylistRoot(Q3ListView *parent, const QString &s1) : WTreeItem(parent, s1)
{
}

WTreeItemPlaylistRoot::~WTreeItemPlaylistRoot()
{
}

void WTreeItemPlaylistRoot::popupMenu()
{
    Q3PopupMenu *menu = new Q3PopupMenu();
    menu->insertItem("New", spTrack, SLOT(slotNewPlaylist()));
    menu->insertItem("Import", spTrack, SLOT(slotImportPlaylist()));
    menu->exec(QCursor::pos());
}

void WTreeItemPlaylistRoot::drag(QWidget *)
{
}

