/***************************************************************************
                          wtracktable.h  -  description
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

#ifndef WTRACKTABLE_H
#define WTRACKTABLE_H

#include "tracklist.h"

#include <qwidget.h>
#include <qtable.h>
#include <qdom.h>
#include <qevent.h>
/**
  *@author Tue & Ken Haste Andersen
  */
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class QTable;
class TrackList;
class QWidget;
	
class WTrackTable : public QTable  {
    Q_OBJECT
public: 
    WTrackTable(QWidget *parent=0, const char *name=0);
    ~WTrackTable();
    void setup(QDomNode node);
    void sortColumn(int col, bool ascending, bool);
	TrackList * trList;
	void setTrackList(TrackList* list);
signals:
    void applyDir(QString );
	
//    void paintFocus(QPainter *p, const QRect &cr);

protected slots:    
	void contentsDropEvent( QDropEvent * );
	void contentsMouseReleaseEvent( QMouseEvent * );
};

#endif
