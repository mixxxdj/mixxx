/***************************************************************************
                          WVUmeter.cpp  -  description
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

#include "wvumeter.h"

WVUmeter::WVUmeter(QWidget *parent, const char *name ) : QProgressBar(parent,name)
{
    // Set the number of steps and reset it:
    setTotalSteps(MAX_STEPS);
    setPercentageVisible( false );
    reset();
}

WVUmeter::~WVUmeter()
{
}

void WVUmeter::setValue(int iValue)
{
    setProgress(iValue);
}

