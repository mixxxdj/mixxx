/***************************************************************************
                          wtracktableitem.cpp  -  description
                             -------------------
    begin                : Mon May 5 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "wtracktableitem.h"
#include <qpainter.h>
#include <qcolor.h>
#include <qrect.h>

WTrackTableItem::WTrackTableItem(QTable *table, EditType et, const QString &text) : QTableItem(table, et, text)
{
}

WTrackTableItem::~WTrackTableItem()
{
}

void WTrackTableItem::paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected)
{
    QColorGroup g( cg );
    // last row is the sum row - we want to make it more visible by
    // using a red background
    g.setColor( QColorGroup::Text, QColor(0,200,0));
    if ((row()/2)*2 == row())
        g.setColor( QColorGroup::Base, QColor(0,0,0));
    else
        g.setColor( QColorGroup::Base, QColor(0,0,0));
    
    QTableItem::paint( p, g, cr, selected );
}
