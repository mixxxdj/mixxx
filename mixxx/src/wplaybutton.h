/***************************************************************************
                          wplaybutton.h  -  description
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

#ifndef WPLAYBUTTON_H
#define WPLAYBUTTON_H

#include <qwidget.h>
#include <qpushbutton.h>
#include <qpainter.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class WPlayButton : public QPushButton  {
   Q_OBJECT
public: 
    WPlayButton(QWidget *parent=0, const char *name=0);
    ~WPlayButton();
protected:
    void drawButton (QPainter *);
private:
    QPixmap *buttonUp, *buttonDown;
};

#endif
