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
#include <qwidget.h>
//Added by qt3to4:
#include <QPixmap>


Track *WTreeItem::spTrack = 0;

WTreeItem::WTreeItem(Q3ListView *parent) : Q3ListViewItem(parent)
{
    pix = 0;
}

WTreeItem::WTreeItem(Q3ListView *parent, const QString &s1) : Q3ListViewItem(parent, s1)
{
    pix = 0;
}

WTreeItem::WTreeItem(WTreeItem *parent) : Q3ListViewItem(parent)
{
    pix = 0;
}

WTreeItem::WTreeItem(WTreeItem *parent, const QString &s1) : Q3ListViewItem(parent, s1)
{
    pix = 0;
}

WTreeItem::WTreeItem(WTreeItem *parent, const QString &s1, const QString &s2) : Q3ListViewItem(parent, s1, s2)
{
    pix = 0;
}

WTreeItem::~WTreeItem()
{
}

QString WTreeItem::drag()
{
    return Q3UriDrag::localFileToUri(WTreeView::fullPath(this));
}

void WTreeItem::setTrack(Track *pTrack)
{
    spTrack = pTrack;
}

void WTreeItem::setPixmap(QPixmap *px)
{
    pix = px;
    setup();
    widthChanged(0);
    invalidateHeight();
    repaint();
}

const QPixmap *WTreeItem::pixmap(int i) const
{
    if (i)
        return 0;
    return pix;
}

