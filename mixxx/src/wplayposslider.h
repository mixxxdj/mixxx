/***************************************************************************
                          wplayposslider.h  -  description
                             -------------------
    begin                : Tue Jun 25 2002
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

#ifndef WPLAYPOSSLIDER_H
#define WPLAYPOSSLIDER_H

#include <qwidget.h>
#include <qpixmap.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class WPlayposSlider : public QWidget  {
   Q_OBJECT
public: 
	WPlayposSlider(QWidget *parent=0, const char *name=0);
	~WPlayposSlider();
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
public slots:
    void setValue(int);
signals:
    void valueChanged(int);
protected:
    void paintEvent(QPaintEvent *e);
private:
    static QPixmap *slider, *marker;
    int value, pix_length, poss;
    static int instantiateNo;
};

#endif
