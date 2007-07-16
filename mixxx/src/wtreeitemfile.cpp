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
#include <q3popupmenu.h>
#include <q3dragobject.h>
#include "track.h"
#include "wtreeitemfile.h"
#include "wtreeview.h"
#include "controlobject.h"

WTreeItemFile::WTreeItemFile(WTreeItem *parent, const QString &s1, const QString &s2) : WTreeItem( parent, s1, s2 )
{
//    setPixmap(new QPixmap(QString("C:\Documents and Settings\Tue\My Documents\cvs\mixxx\src\icons\unknown.png")));
}

WTreeItemFile::~WTreeItemFile()
{
}

void WTreeItemFile::popupMenu()
{
    Q3PopupMenu *menu = new Q3PopupMenu();
    int id;
    
    id = menu->insertItem("Player 1", this, SLOT(slotLoadPlayer1()));
    if (ControlObject::getControl(ConfigKey("[Channel1]","play"))->get()==1.)
        menu->setItemEnabled(id, false);
		    
    id = menu->insertItem("Player 2", this, SLOT(slotLoadPlayer2()));
    if (ControlObject::getControl(ConfigKey("[Channel2]","play"))->get()==1.)
        menu->setItemEnabled(id, false);

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
