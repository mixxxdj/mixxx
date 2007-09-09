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
#include "wskincolor.h"
#include "trackinfoobject.h"

#include <qfont.h>
#include <QtDebug>
#include <qcolor.h>
#include <q3dragobject.h>
#include <qpoint.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmime.h>
//Added by qt3to4:
#include <Q3Frame>
#include "trackinfoobject.h"

#include "dlgbpmtap.h"


#include <qwidget.h>

/*Constructor, sets up attributes in playlist*/
WTrackTable::WTrackTable(QWidget * parent, const char * name) : Q3Table(0, ROW_NO, parent, name)
{
    setSorting(true);
    setSelectionMode(Q3Table::SingleRow);
    setFocusStyle(Q3Table::FollowStyle);

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
    setFrameStyle(Q3Frame::NoFrame);

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

    connect(this, SIGNAL(doubleClicked(int, int, int, const QPoint &)), this, SLOT(slotMouseDoubleClicked(int, int, int, const QPoint &)));
    bpmTapDlg = 0;

}


WTrackTable::~WTrackTable()
{

    if(bpmTapDlg)
        delete bpmTapDlg;

}

/*Graphically sets up playlist and library directory*/
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
        //setBaseSize(x,y);
        setFixedSize(x,y);
        //setMaximumWidth(x);
        //setMaximumHeight(y);
        resizeContents(x,y);
    }

    // Background color
    QColor bgc(255,255,255);
    if (!WWidget::selectNode(node, "BgColor").isNull())
    {
        bgc.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
    }
    setPaletteBackgroundColor(WSkinColor::getCorrectColor(bgc));

    // Foreground color
    QColor fgc(0,0,0);
    if (!WWidget::selectNode(node, "FgColor").isNull())
    {
        fgc.setNamedColor(WWidget::selectNodeQString(node, "FgColor"));
    }
    setPaletteForegroundColor(WSkinColor::getCorrectColor(fgc));

    // Row colors
    if (!WWidget::selectNode(node, "BgColorRowEven").isNull())
    {
        QColor r1;
        r1.setNamedColor(WWidget::selectNodeQString(node, "BgColorRowEven"));
        QColor r2;
        r2.setNamedColor(WWidget::selectNodeQString(node, "BgColorRowUneven"));
        WTrackTableItem::setRowColors(WSkinColor::getCorrectColor(r1),
                                      WSkinColor::getCorrectColor(r2));
    }

    // BPM confidence colors
    if (!WWidget::selectNode(node, "BgColorBpmNoConfirm").isNull())
    {
        QColor c1;
        c1.setNamedColor(WWidget::selectNodeQString(node, "BgColorBpmNoConfirm"));
        QColor c2;
        c2.setNamedColor(WWidget::selectNodeQString(node, "BgColorBpmConfirm"));
        WTrackTableItem::setBpmBgColors(WSkinColor::getCorrectColor(c1),
                                        WSkinColor::getCorrectColor(c2));
    }

    // Setup column widths
    setLeftMargin(0);
    hideColumn(COL_INDEX);
    adjustColumn(COL_SCORE);

    //setColumnStretchable(COL_TITLE, true);
    //setColumnStretchable(COL_ARTIST, true);
    setColumnStretchable(COL_COMMENT, true);
    adjustColumn(COL_TYPE);
    adjustColumn(COL_DURATION);
    adjustColumn(COL_BPM);
    adjustColumn(COL_BITRATE);
    setColumnStretchable(COL_SCORE,false);
    setColumnStretchable(COL_TYPE,false);
    setColumnStretchable(COL_DURATION,false);
    setColumnStretchable(COL_BPM, false);
    setColumnStretchable(COL_BITRATE,false);

    typedef QMap<int,QString> ColMap;
    ColMap ColumnMap;
    ColumnMap[COL_SCORE] = "ColWidthScore";
    ColumnMap[COL_TITLE] = "ColWidthTitle";
    ColumnMap[COL_ARTIST] = "ColWidthArtist";
    ColumnMap[COL_COMMENT] = "ColWidthComment";
    ColumnMap[COL_TYPE] = "ColWidthType";
    ColumnMap[COL_DURATION] = "ColWidthDuration";
    ColumnMap[COL_BITRATE] = "ColWidthBitrate";
    ColumnMap[COL_BPM] = "ColWidthBpm";

    QMapIterator<int,QString> i(ColumnMap);
    while (i.hasNext()) {
        i.next();
        if (!WWidget::selectNode(node, i.value()).isNull() && WWidget::selectNodeQString(node, i.value()).toInt() != columnWidth(i.key())) {
            qDebug("Correcting Column Width from %i to %i",columnWidth(i.key()),WWidget::selectNodeQString(node, i.value()).toInt());
            setColumnWidth(i.key(),WWidget::selectNodeQString(node, i.value()).toInt());
        }
    }
}

/*sorts a given column*/
void WTrackTable::sortColumn(int col, bool ascending, bool)
{
    Q3Table::sortColumn(col,ascending,true);
}

/*checks for Mouse action*/
void WTrackTable::slotMousePressed(int row, int col, int button, const QPoint &)
{
    if (col!=COL_COMMENT && button==Qt::RightButton)
    {
        WTrackTableItem * p = (WTrackTableItem *)item(row,col);
        if (p)
        {
            TrackInfoObject * pTrackInfoObject = p->getTrackInfoObject();
            if (pTrackInfoObject)
                emit(mousePressed(pTrackInfoObject, button));
        }
    }
}

void WTrackTable::slotMouseDoubleClicked(int row, int col, int button, const QPoint &)
{
    if(col == COL_BPM && button==Qt::LeftButton)
    {
        WTrackTableItem * p = (WTrackTableItem *)item(row,col);

        if(p)
        {
            TrackInfoObject * pTrackInfoObject = p->getTrackInfoObject();
            if(pTrackInfoObject)
            {
                if(bpmTapDlg)
                    delete bpmTapDlg;

#ifdef __EXPERIMENTAL_BPM__

                bpmTapDlg = new DlgBpmTap(NULL, pTrackInfoObject, NULL);
                bpmTapDlg->show();
#endif
            }
        }
    }

}

/*enables contents to be dragable*/
Q3DragObject * WTrackTable::dragObject()
{
    WTrackTableItem * p = (WTrackTableItem *)item(currentRow(),currentColumn());
    TrackInfoObject * pTrackInfoObject = p->getTrackInfoObject();

    Q3UriDrag * ud = new Q3UriDrag(this);
    ud->setFileNames(QStringList(pTrackInfoObject->getLocation()));

    return ud;
}
