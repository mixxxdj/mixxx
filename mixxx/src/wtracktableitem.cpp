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
#include "wtracktable.h"
#include "trackinfoobject.h"
#include <qpainter.h>
#include <qcolor.h>
#include <qrect.h>
#include <qstring.h>

QColor WTrackTableItem::kqRowColor1;
QColor WTrackTableItem::kqRowColor2;

WTrackTableItem::WTrackTableItem(TrackInfoObject *pTrackInfoObject, QTable *table, EditType et, const QString &text, enumType eType) : QTableItem(table, et, text)
{
    m_eType = eType;
    m_pTrackInfoObject = pTrackInfoObject;
}

WTrackTableItem::~WTrackTableItem()
{
}

void WTrackTableItem::setRowColors(QColor r1, QColor r2)
{
    kqRowColor1 = r1;
    kqRowColor2 = r2;
}

void WTrackTableItem::paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected)
{
    QColorGroup g( cg );
    // last row is the sum row - we want to make it more visible by
    // using a red background
//    g.setColor( QColorGroup::Text, QColor(0,0,0));
    if ((row()/2)*2 == row())
        g.setColor( QColorGroup::Base, kqRowColor1);
    else
        g.setColor( QColorGroup::Base, kqRowColor2);

    QTableItem::paint( p, g, cr, selected);
}
/*
    Returns a key which is used for sorting of the table.
*/
QString WTrackTableItem::key() const
{
    static QString sResult;
    switch (m_eType) {
    case typeText: 
        sResult = text();
        break;
    case typeNumber: case typeDuration:
//        sResult.fill('0', 10-text().length()); // Assume at most 10 digits
//        sResult += text();
        sResult = text().rightJustify(10,'0');
        break;
    }
    return sResult;
}

TrackInfoObject *WTrackTableItem::getTrackInfoObject()
{
    return m_pTrackInfoObject;
}

void WTrackTableItem::setContentFromEditor(QWidget *w)
{
    // Update cell
    QTableItem::setContentFromEditor(w);

    // If this is a comment column, update TrackInfoObject
    if (col()==COL_COMMENT)
        m_pTrackInfoObject->setComment(text());
}

int WTrackTableItem::alignment() const
{
   static int a;
    switch (m_eType) {
    case typeText: 
       a = Qt::AlignLeft;
        break;
    case typeNumber: case typeDuration:
       a = Qt::AlignRight;
       break;
    }
   return a;
}
