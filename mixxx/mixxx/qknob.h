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
	QPixmap *button000;
	QPixmap *button045;
	QPixmap *button090;
	QPixmap *button135;
	QPixmap *button180;
	QPixmap *button225;
	QPixmap *button270;
	QPixmap *button315;

	QRect *repaintRect;
};

#endif
