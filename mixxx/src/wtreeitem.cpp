//
// C++ Implementation: wtreeitem
//
// Description: 
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "wtreeitem.h"
#include "wtreeview.h"

Track *WTreeItem::spTrack = 0;

WTreeItem::WTreeItem(QListView *parent) : QListViewItem(parent)
{
}

WTreeItem::WTreeItem(QListView *parent, const QString &s1) : QListViewItem(parent, s1)
{
}

WTreeItem::WTreeItem(WTreeItem *parent) : QListViewItem(parent)
{
}

WTreeItem::WTreeItem(WTreeItem *parent, const QString &s1) : QListViewItem(parent, s1)
{
}

WTreeItem::WTreeItem(WTreeItem *parent, const QString &s1, const QString &s2) : QListViewItem(parent, s1, s2)
{
}

WTreeItem::~WTreeItem()
{
}

QString WTreeItem::drag()
{
    return QUriDrag::localFileToUri(WTreeView::fullPath(this));
}

void WTreeItem::setTrack(Track *pTrack)
{
    spTrack = pTrack;
}


