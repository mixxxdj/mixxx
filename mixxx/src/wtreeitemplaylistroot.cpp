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

#include <qpopupmenu.h>
#include <qcursor.h>
#include "wtreeitemplaylistroot.h"
#include "track.h"

WTreeItemPlaylistRoot::WTreeItemPlaylistRoot(QListView *parent, const QString &s1) : WTreeItem(parent, s1)
{
}

WTreeItemPlaylistRoot::~WTreeItemPlaylistRoot()
{
}

void WTreeItemPlaylistRoot::popupMenu()
{
    QPopupMenu *menu = new QPopupMenu();
    menu->insertItem("New", spTrack, SLOT(slotNewPlaylist()));
    menu->insertItem("Import", spTrack, SLOT(slotImportPlaylist()));
    menu->exec(QCursor::pos());
}

void WTreeItemPlaylistRoot::drag(QWidget *)
{
}

