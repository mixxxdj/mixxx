/***************************************************************************
                          wvumeter.h  -  description
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

#ifndef WVUMETER_H
#define WVUMETER_H

#include <qwidget.h>
#include <qprogressbar.h>
#include "defs.h"

/**
  *@author Tue & Ken Haste Andersen
  */

#define MAX_STEPS 127

class WVUmeter : public QProgressBar
{
    Q_OBJECT
public: 
    WVUmeter(QWidget *parent=0, const char *name=0);
    ~WVUmeter();
signals:
    void valueChanged(int);
public slots:
    void setValue(int);

};

#endif
