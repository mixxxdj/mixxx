/***************************************************************************
                          wbulb.h  -  description
                             -------------------
    begin                : Fri Jun 21 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
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

#ifndef WBULB_H
#define WBULB_H

#include <qwidget.h>
#include <qradiobutton.h>
#include <qpainter.h>
#include <qevent.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class WBulb : public QRadioButton  {
   Q_OBJECT
public: 
    WBulb(QWidget *parent=0, const char *name=0);
    ~WBulb();
protected:
    void drawButton (QPainter *);
    void mousePressEvent(QMouseEvent *e);
private:
    static QPixmap *bulbOn, *bulbOff;
};

#endif
