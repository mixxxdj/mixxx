/***************************************************************************
                          wwheel.h  -  description
                             -------------------
    begin                : Mon Jun 24 2002
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

#ifndef WWHEEL_H
#define WWHEEL_H

#include <qwidget.h>
#include <qpixmap.h>
#include <qevent.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class WWheel : public QWidget
{
   Q_OBJECT
public: 
    WWheel(QWidget *parent=0, const char *name=0);
    ~WWheel();
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *);
public slots:
    void setValue(int);
signals:
    void valueChanged(int);
private:
    int value, oldvalue, startval;
    static QPixmap **pix;
};

#endif
