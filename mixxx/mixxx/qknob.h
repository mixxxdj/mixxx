/***************************************************************************
                          qknob.h  -  description
                             -------------------
    begin                : Wed Feb 27 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QKNOB_H
#define QKNOB_H

#include <qwidget.h>
#include <qdial.h>
#include <qpixmap.h>

/**
  *@author Tue and Ken Haste Andersen
  */

class QKnob : public QDial  {
   Q_OBJECT
public: 
	QKnob(QWidget *parent=0, const char *name=0);
	~QKnob();
protected:
	void repaintScreen(const QRect *cr=0);
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
