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

void WTreeItem::popupMenu()
{
}

void WTreeItem::drag(QWidget *viewport)
{
    QString source = WTreeView::fullPath(this);
    QStrList lst;
    lst.append(QUriDrag::localFileToUri(source));
    QUriDrag* ud = new QUriDrag(viewport);
    ud->setUris(lst);
    ud->dragCopy();
}

