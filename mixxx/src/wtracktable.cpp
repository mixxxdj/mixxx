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

#include <qfont.h>
#include <qcolor.h>

WTrackTable::WTrackTable(QWidget *parent, const char *name) : QTable(10, 7, parent, name)
{
    setSorting(true);
    setSelectionMode(QTable::SingleRow);
    setFocusStyle(QTable::FollowStyle);
    
    horizontalHeader()->setLabel( 0, tr( "**" ) );
    horizontalHeader()->setLabel( 1, tr( "Title" ) );
    horizontalHeader()->setLabel( 2, tr( "Artist" ) );
    horizontalHeader()->setLabel( 3, tr( "Type" ) );
    horizontalHeader()->setLabel( 4, tr( "Duration" ) );
    horizontalHeader()->setLabel( 5, tr( "Bitrate" ) );
    horizontalHeader()->setLabel( 6, tr( "Index" ) );

    // Setup background color
    setPaletteBackgroundColor(QColor(168,181,164));
    setBackgroundMode(Qt::PaletteBackground);
    setShowGrid(false);
    
    // Setup scrollbars
    setVScrollBarMode(AlwaysOn);
    setHScrollBarMode(AlwaysOff);
    
    // Setup tracklist collum widths
    setLeftMargin(0);
    setColumnWidth(0,20);
    setColumnWidth(1,240);
    setColumnWidth(2,220);
    setColumnWidth(3,30);
    setColumnWidth(4,50);
    setColumnWidth(5,50);
    hideColumn(6);
}

WTrackTable::~WTrackTable()
{
}

void WTrackTable::sortColumn(int col, bool ascending, bool)
{
    QTable::sortColumn(col,ascending,true);
}
