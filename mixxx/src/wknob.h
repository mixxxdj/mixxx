/***************************************************************************
                          wknob.h  -  description
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

#ifndef WKNOB_H
#define WKNOB_H

#include <qwidget.h>
#include <qdial.h>
#include <qpixmap.h>
#include <qrect.h>


/**
  *@author Tue & Ken Haste Andersen
  */

class WKnob : public QDial  {
   Q_OBJECT
public: 
	WKnob(QWidget *parent=0, const char *name=0);
	~WKnob();
protected:
	void repaintScreen(const QRect *cr=0);
    void repaintScreen(const QRect *cr, QPixmap *p);
    QPixmap *getKnob();
private:
    static QPixmap **pix;
};

#endif
