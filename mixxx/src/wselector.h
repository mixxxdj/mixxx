/***************************************************************************
                          wselector.h  -  description
                             -------------------
    begin                : Sun Nov 17 2002
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

#ifndef WSELECTOR_H
#define WSELECTOR_H

#include <qslider.h>
#include <qpainter.h>
#include <qpixmap.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class WSelector : public QSlider  {
public: 
    WSelector(QWidget *parent=0, const char *name=0);
    ~WSelector();
protected:
    void drawButton (QPainter *);
private:
    static QPixmap *sliderA, *sliderB;

};

#endif
