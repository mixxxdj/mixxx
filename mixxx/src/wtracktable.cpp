/***************************************************************************
                          tracklist.cpp  -  description
                             -------------------
    begin                : 10 02 2003
    copyright            : (C) 2003 by Ingo Kossyk & Tue Haste Andersen
    email                : kossyki@cs.tu-berlin.de & haste@diku.dk
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
#include <qdragobject.h>
#include <qpoint.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmime.h>




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
    //setFrameStyle(StyledPanel);
    //setPaletteBackgroundColor(QColor(148,171,194));
 	
	//Accept Drops 
    viewport()->setAcceptDrops( TRUE );
    setAcceptDrops( TRUE );
	
    // Font size
    QFont f("Helvetica");
    f.setPointSize(9);
    setFont(f);

    // Setup scrollbars
    setVScrollBarMode(AlwaysOn);
    setHScrollBarMode(AlwaysOff);
	//connect(this, SIGNAL(dropped(QDropEvent)), this , SLOT(contentsDrop(QDropEvent)));
	
}


WTrackTable::~WTrackTable()
{
}

void WTrackTable::setTrackList(TrackList* list){
	
	trList = list;
	
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
        setFixedSize(x,y);
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

    // Setup column widths
    setLeftMargin(0);
    //hideColumn(COL_INDEX);
    setColumnWidth(COL_SCORE, WWidget::selectNodeInt(node, "ColWidthScore"));
    setColumnWidth(COL_TITLE, WWidget::selectNodeInt(node, "ColWidthTitle"));
    setColumnWidth(COL_ARTIST, WWidget::selectNodeInt(node, "ColWidthArtist"));
    setColumnWidth(COL_COMMENT, WWidget::selectNodeInt(node, "ColWidthComment"));
    setColumnWidth(COL_TYPE, WWidget::selectNodeInt(node, "ColWidthType"));
    setColumnWidth(COL_DURATION, WWidget::selectNodeInt(node, "ColWidthDuration"));
    setColumnWidth(COL_BPM, WWidget::selectNodeInt(node, "ColWidthBpm"));
	setColumnWidth(COL_BITRATE, WWidget::selectNodeInt(node, "ColWidthBitrate"));
}

void WTrackTable::contentsMouseReleaseEvent( QMouseEvent * e)
{
	
	//do nothing
}
//An Drop has occured and an Dropevent is going to be decoded
//After that the Signal applyDir is being emited which is connected
//to the tracklists updateslot
void WTrackTable::contentsDropEvent( QDropEvent *e )
{
 if ( !QUriDrag::canDecode(e) ) {
        qDebug("Could not decode drag object..");
	 	e->ignore();
        return;
    }	
	QStrList lst;
	
	QUriDrag::decode( e, lst );
	e->accept();
	
	for ( uint i = 0; i < lst.count(); ++i ) { 
		QString * test = new QString(QUriDrag::uriToUnicodeUri(lst.at( i )));
		QString Dir = QUriDrag::uriToLocalFile(lst.at( i ));
		qDebug(Dir);
		if(test->endsWith(".xml")){
			Dir = QUriDrag::uriToUnicodeUri(lst.at( i ));
			QStringList lst( QStringList::split( "/", Dir ) );
			QStringList::Iterator lstIt = lst.end();
			--lstIt;
			emit(applyDir((*lstIt)));
			return;
			}
		
		emit(applyDir(Dir));
        }
		
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
