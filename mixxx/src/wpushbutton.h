/***************************************************************************
                          wpushbutton.h  -  description
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

#ifndef WPUSHBUTTON_H
#define WPUSHBUTTON_H

#include <qwidget.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include "controlpushbutton.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class WPushButton : public QPushButton
{
    Q_OBJECT
public: 
    WPushButton(QWidget *parent=0, const char *name=0);
    ~WPushButton();
public slots:
    void setValue(int);
signals:
    void valueChanged(int);
    void valueOn();
private slots:
    void emitValueChanged(bool);
};

#endif
