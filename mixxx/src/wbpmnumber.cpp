/***************************************************************************
                          wbpmnumber.cpp  -  description
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

#include "wbpmnumber.h"

WBPMNumber::WBPMNumber(QWidget *parent, const char *name ) : QLCDNumber(parent,name)
{
}

WBPMNumber::~WBPMNumber()
{
}

void WBPMNumber::setValue(int fValue)
{
    display(fValue);
}
