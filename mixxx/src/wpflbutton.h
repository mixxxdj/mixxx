/***************************************************************************
                          wpflbutton.h  -  description
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

#ifndef WPFLBUTTON_H
#define WPFLBUTTON_H

#include <qwidget.h>
#include <qcheckbox.h>
#include <qpainter.h>
#include <qpixmap.h>

// #include "controlpushbutton.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class WPFLButton : public QCheckBox  {
   Q_OBJECT
public: 
    WPFLButton(QWidget *parent=0, const char *name=0);
    ~WPFLButton();
public slots:
    void setValue(int);
signals:
    void valueChanged(int);
protected:
    void drawButton (QPainter *);
private slots:
    void emitValueChanged(bool);
private:
    static QPixmap *buttonUp, *buttonDown;
    static int instantiateNo;
};

#endif
