/***************************************************************************
                          wbpmnumber.h  -  description
                             -------------------
    begin                : Wed Jun 18 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
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

#ifndef WBPMNUMBER_H
#define WBPMNUMBER_H

#include <qwidget.h>
#include <qlcdnumber.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class WBPMNumber : public QLCDNumber  {
    Q_OBJECT
public: 
    WBPMNumber(QWidget *parent=0, const char *name=0);
    ~WBPMNumber();
signals:
    void valueChanged(int);
public slots:
    void setValue(int);
};

#endif
