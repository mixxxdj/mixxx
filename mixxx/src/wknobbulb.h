/***************************************************************************
                          wknobbulb.h  -  description
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

#ifndef WKNOBBULB_H
#define WKNOBBULB_H

#include <qwidget.h>
#include <wknob.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class WKnobBulb : public WKnob  {
    Q_OBJECT
public: 
    WKnobBulb(QWidget *parent=0, const char *name=0);
    ~WKnobBulb();
public slots:
    void slotSetBulb(bool state);
protected:
    void repaintScreen(const QRect *cr=0);
private:
    QPixmap *on, *off;
    bool bulbState;
};

#endif
