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
	QPixmap *button00, *button01, *button02, *button03, *button04, *button05,
			*button06, *button07, *button08, *button09, *button10, *button11,
			*button12, *button13, *button14, *button15, *button16, *button17,
			*button18, *button19, *button20, *button21, *button22, *button23,
			*button24, *button25, *button26, *button27, *button28, *button29,
			*button30, *button31;
	QRect *repaintRect;
};

#endif
