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

    // Setup column widths
    setLeftMargin(0);
    hideColumn(COL_INDEX);
    setColumnWidth(COL_SCORE, WWidget::selectNodeInt(node, "ColWidthScore"));
    setColumnWidth(COL_TITLE, WWidget::selectNodeInt(node, "ColWidthTitle"));
    setColumnWidth(COL_ARTIST, WWidget::selectNodeInt(node, "ColWidthArtist"));
    setColumnWidth(COL_COMMENT, WWidget::selectNodeInt(node, "ColWidthComment"));
    setColumnWidth(COL_TYPE, WWidget::selectNodeInt(node, "ColWidthType"));
    setColumnWidth(COL_DURATION, WWidget::selectNodeInt(node, "ColWidthDuration"));
    setColumnWidth(COL_BITRATE, WWidget::selectNodeInt(node, "ColWidthBitrate"));
}


void WTrackTable::sortColumn(int col, bool ascending, bool)
{
    QTable::sortColumn(col,ascending,true);
}

/*
void WTrackTable::paintFocus(QPainter *p, const QRect &cr)
{

}
*/