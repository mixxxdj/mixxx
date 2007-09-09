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
#include <q3popupmenu.h>
#include <q3dragobject.h>
#include <q3cstring.h>
#include "trackplaylist.h"
#include "track.h"
#include <qcursor.h>

WTreeItemPlaylist::WTreeItemPlaylist(WTreeItem * parent, TrackPlaylist * pPlaylist) : WTreeItem(parent)
{
    setRenameEnabled(0, true);
    m_pPlaylist = pPlaylist;
    setText(0, m_pPlaylist->getListName());

//    QPixmap *p = new QPixmap(QString("C:\Documents and Settings\Tue\My Documents\cvs\mixxx\src\icons\unknown.png"));

//    setPixmap(p);

    setup();
    widthChanged( 0 );
    invalidateHeight();
    repaint();

}

/*
   WTreeItemPlaylist::WTreeItemPlaylist(WTreeItem *parent, const QString &s1) : WTreeItem(parent, s1)
   {
    setRenameEnabled(0, true);
   }
 */

WTreeItemPlaylist::~WTreeItemPlaylist()
{
}

void WTreeItemPlaylist::popupMenu()
{
    Q3PopupMenu * menu = new Q3PopupMenu();
    menu->insertItem("Rename", this, SLOT(slotRename()));
    menu->insertItem("Delete", this, SLOT(slotDelete()));
    menu->exec(QCursor::pos());
}

QString WTreeItemPlaylist::drag()
{
    return name();
}

QString WTreeItemPlaylist::name()
{
    return text(0);
}

void WTreeItemPlaylist::slotRename()
{
    startRename(0);
}

void WTreeItemPlaylist::slotDelete()
{
    spTrack->slotDeletePlaylist(text(0));
}

void WTreeItemPlaylist::okRename(int col)
{
    Q3ListViewItem::okRename(col);
    m_pPlaylist->setListName(text(0));
}
