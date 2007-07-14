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

#include <qtable.h>
#include <qdom.h>
#include <qevent.h>

class QWidget;

// Defines the rows in the table.
const int COL_SCORE = 0;
const int COL_TITLE = 1;
const int COL_ARTIST = 2;
const int COL_COMMENT = 3;
const int COL_TYPE = 6;
const int COL_DURATION = 4;
const int COL_BITRATE = 7;
const int COL_BPM = 5;
const int COL_INDEX = 8;

const int ROW_NO = 9;

/**
  *@author Tue & Ken Haste Andersen
  */
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class QTable;
class QWidget;
class TrackInfoObject;
class DlgBPMTap;

class WTrackTable : public QTable
{
    Q_OBJECT
public:
    WTrackTable(QWidget *parent=0, const char *name=0);
    ~WTrackTable();
    void setup(QDomNode node);
    void sortColumn(int col, bool ascending, bool);
protected slots:
    void slotMousePressed(int row, int col, int button, const QPoint &);
    void slotMouseDoubleClicked(int row, int col, int button, const QPoint &);
    QDragObject *dragObject();
signals:
    void mousePressed(TrackInfoObject *pTrackInfoObject, int button);

protected:
    DlgBPMTap *bpmTapDlg;
};

#endif
