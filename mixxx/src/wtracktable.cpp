/***************************************************************************
                          wtracktable.cpp  -  description
                             -------------------
    begin                : Sun May 4 2003
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

#include "wtracktable.h"
#include "wtracktableitem.h"
#include "wwidget.h"
#include "tracklist.h"
#include <qfont.h>
#include <qcolor.h>

WTrackTable::WTrackTable(QWidget *parent, const char *name) : QTable(10, 8, parent, name)
{
    setSorting(true);  
    setSelectionMode(QTable::SingleRow);
    setFocusStyle(QTable::FollowStyle);
    
    horizontalHeader()->setLabel(COL_SCORE, tr( "**" ) );
    horizontalHeader()->setLabel(COL_TITLE, tr( "Title" ) );
    horizontalHeader()->setLabel(COL_ARTIST, tr( "Artist" ) );
    horizontalHeader()->setLabel(COL_COMMENT, tr( "Comment" ) );
    horizontalHeader()->setLabel(COL_TYPE, tr( "Type" ) );
    horizontalHeader()->setLabel(COL_DURATION, tr( "Duration" ) );
    horizontalHeader()->setLabel(COL_BITRATE, tr( "Bitrate" ) );
    horizontalHeader()->setLabel(COL_INDEX, tr( "Index" ) );

    // Setup table properties
    setShowGrid(false);
    setFrameStyle(QFrame::NoFrame);
    //setPaletteBackgroundColor(QColor(148,171,194));

    // Font size
    QFont f("Helvetica");
    f.setPointSize(9);
    setFont(f);

    // Setup scrollbars
    setVScrollBarMode(AlwaysOn);
    setHScrollBarMode(AlwaysOff);
    
    // Setup tracklist collum widths
    setLeftMargin(0);
    setColumnWidth(COL_SCORE,20);
    setColumnWidth(COL_TITLE,240);
    setColumnWidth(COL_ARTIST,220);
    setColumnWidth(COL_COMMENT,246);
    setColumnWidth(COL_TYPE,30);
    setColumnWidth(COL_DURATION,50);
    setColumnWidth(COL_BITRATE,50);
    hideColumn(COL_INDEX);
}

WTrackTable::~WTrackTable()
{
}

void WTrackTable::setup(QDomNode node)
{
    // Position
    QString pos = WWidget::selectNodeQString(node, "Pos");
    int x = pos.left(pos.find(",")).toInt();
    int y = pos.mid(pos.find(",")+1).toInt();
    move(x,y);

    // Size
    QString size = WWidget::selectNodeQString(node, "Size");
    x = size.left(size.find(",")).toInt();
    y = size.mid(size.find(",")+1).toInt();
    setFixedSize(x,y);

    // Background color
    if (!WWidget::selectNode(node, "BgColor").isNull())
    {
        QColor c;
        c.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
        setPaletteBackgroundColor(c);
    }

    // Foreground color
    if (!WWidget::selectNode(node, "FgColor").isNull())
    {
        QColor c;
        c.setNamedColor(WWidget::selectNodeQString(node, "FgColor"));
        setPaletteForegroundColor(c);
    }

    // Row colors
    if (!WWidget::selectNode(node, "BgColorRowEven").isNull())
    {
        QColor r1;
        r1.setNamedColor(WWidget::selectNodeQString(node, "BgColorRowEven"));
        QColor r2;
        r2.setNamedColor(WWidget::selectNodeQString(node, "BgColorRowUneven"));
        WTrackTableItem::setRowColors(r1, r2);
    }


}


void WTrackTable::sortColumn(int col, bool ascending, bool)
{
    QTable::sortColumn(col,ascending,true);
}

