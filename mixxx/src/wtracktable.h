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

#include <qwidget.h>
#include <qtable.h>
#include <qdom.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class WTrackTable : public QTable  {
    Q_OBJECT
public: 
    WTrackTable(QWidget *parent=0, const char *name=0);
    ~WTrackTable();
    void setup(QDomNode node);
    void sortColumn(int col, bool ascending, bool);
//    void paintFocus(QPainter *p, const QRect &cr);
};

#endif
