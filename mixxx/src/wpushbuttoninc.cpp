/***************************************************************************
                          wpushbuttoninc.cpp  -  description
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

#include "wpushbuttoninc.h"

WPushButtonInc::WPushButtonInc(QWidget *parent, const char *name ) : WPushButton(parent,name)
{
    setStates(1);
    m_dValueIncLeft = 1.;
    m_dValueIncRight = 1.;
}

WPushButtonInc::~WPushButtonInc()
{
}

void WPushButtonInc::setInc(double dValueIncLeft, double dValueIncRight)
{
    m_dValueIncLeft = dValueIncLeft;
    m_dValueIncRight = dValueIncRight;    
}


void WPushButtonInc::mousePressEvent(QMouseEvent *e)
{
    m_bPressed = true;

    // Calculate new state if the left mouse button was used to press the button
    if (e->button()==Qt::LeftButton)
        emit(valueChangedLeftDown(m_dValueIncLeft));
    else if (e->button()==Qt::RightButton)
        emit(valueChangedRightDown(m_dValueIncRight));

    update();
}

void WPushButtonInc::mouseReleaseEvent(QMouseEvent *e)
{
    m_bPressed = false;

    if (e->button()==Qt::LeftButton)
        emit(valueChangedLeftUp(m_dValueIncLeft));
    else if (e->button()==Qt::RightButton)
        emit(valueChangedRightUp(m_dValueIncRight));

    update();
}
