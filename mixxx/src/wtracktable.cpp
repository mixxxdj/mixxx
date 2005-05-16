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
#include <qfont.h>
#include <qcolor.h>
#include <qdragobject.h>
#include <qpoint.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmime.h>
#include "trackinfoobject.h"

WTrackTable::WTrackTable(QWidget *parent, const char *name) : QTable(0, ROW_NO, parent, name)
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
    horizontalHeader()->setLabel(COL_BPM, tr( "BPM" ) );
    horizontalHeader()->setLabel(COL_INDEX, tr( "Index" ) );

    // Setup table properties
    setShowGrid(false);
    setFrameStyle(QFrame::NoFrame);

    //Accept Drops
    viewport()->setAcceptDrops(true);
    setAcceptDrops(true);
    setDragEnabled(true);

    // Allow table reordering
    setRowMovingEnabled(true);
    
    // Setup scrollbars
    setVScrollBarMode(AlwaysOn);
    setHScrollBarMode(AlwaysOff);

    connect(this, SIGNAL(pressed(int, int, int, const QPoint &)), this, SLOT(slotMousePressed(int, int, int, const QPoint &)));
}


WTrackTable::~WTrackTable()
{
}

void WTrackTable::setup(QDomNode node)
{
    // Position
    if (!WWidget::selectNode(node, "Pos").isNull())
    {
        QString pos = WWidget::selectNodeQString(node, "Pos");
        int x = pos.left(pos.find(",")).toInt();
        int y = pos.mid(pos.find(",")+1).toInt();
        move(x,y);
    }

    // Size
    if (!WWidget::selectNode(node, "Size").isNull())
    {
        QString size = WWidget::selectNodeQString(node, "Size");
        int x = size.left(size.find(",")).toInt();
        int y = size.mid(size.find(",")+1).toInt();
        setBaseSize(x,y);
        resizeContents(x,y);
    }

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

    // BPM confidence colors
    if (!WWidget::selectNode(node, "BgColorBpmNoConfirm").isNull())
    {
        QColor c1;
        c1.setNamedColor(WWidget::selectNodeQString(node, "BgColorBpmNoConfirm"));
        QColor c2;
        c2.setNamedColor(WWidget::selectNodeQString(node, "BgColorBpmConfirm"));
        WTrackTableItem::setBpmBgColors(c1, c2);
    }
    
    // Setup column widths
    setLeftMargin(0);
    hideColumn(COL_INDEX);
    adjustColumn(COL_SCORE);
    //setColumnStretchable(COL_TITLE, true);
    //setColumnStretchable(COL_ARTIST, true);
    //setColumnStretchable(COL_COMMENT, true);
    adjustColumn(COL_TYPE);
    adjustColumn(COL_DURATION);
//    adjustColumn(COL_BPM);
    adjustColumn(COL_BITRATE);
    //setColumnStretchable(COL_SCORE,true);
    setColumnStretchable(COL_TYPE,true);
    setColumnStretchable(COL_DURATION,true);
//    setColumnStretchable(COL_BPM,true);
    setColumnStretchable(COL_BITRATE,true);

    if (!WWidget::selectNode(node, "ColWidthBpm").isNull())
    {
        int width = WWidget::selectNodeQString(node, "ColWidthBpm").toInt();
        setColumnWidth(COL_BPM,width);
    }				
}

void WTrackTable::sortColumn(int col, bool ascending, bool)
{
    QTable::sortColumn(col,ascending,true);
}

void WTrackTable::slotMousePressed(int row, int col, int button, const QPoint &)
{
    if (col!=COL_COMMENT && col!=COL_BPM)
    {
        WTrackTableItem *p = (WTrackTableItem *)item(row,col);
        if (p)
        {
            TrackInfoObject *pTrackInfoObject = p->getTrackInfoObject();
            if (pTrackInfoObject)
                emit(mousePressed(pTrackInfoObject, button));
        }
    }
}

QDragObject *WTrackTable::dragObject()
{
    WTrackTableItem *p = (WTrackTableItem *)item(currentRow(),currentColumn());
    TrackInfoObject *pTrackInfoObject = p->getTrackInfoObject();

    QUriDrag *ud = new QUriDrag(this);
    ud->setFileNames(QStringList(pTrackInfoObject->getLocation()));

    return ud;
}
