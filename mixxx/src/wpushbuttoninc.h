/***************************************************************************
                          wpushbuttoninc.h  -  description
                             -------------------
    begin                : Mon Jul 7 2003
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

#ifndef WPUSHBUTTONINC_H
#define WPUSHBUTTONINC_H

#include <qwidget.h>
#include <wpushbutton.h>

/**
  * A one state WPushButton which connects to a ControlPotmeter by
  * incrementing or decrementing it's value when clicked.
  *
  *@author Tue & Ken Haste Andersen
  */

class WPushButtonInc : public WPushButton  {
   Q_OBJECT
public:
    WPushButtonInc(QWidget *parent=0, const char *name=0);
    ~WPushButtonInc();
    void setup(QDomNode node);
    /** Sets the increment value when left and right clicking */
    void setInc(double dValueIncLeft, double dValueIncRight);
    /** Mouse pressed */
    void mousePressEvent(QMouseEvent *e);
    /** Mouse released */
    void mouseReleaseEvent(QMouseEvent *e);
private:
    /** Increments sent out when left and right clicking */
    double m_dValueIncLeft, m_dValueIncRight;
};

#endif
