/***************************************************************************
                          monitor.h  -  description
                             -------------------
    begin                : Sun Apr 28 2002
    copyright            : (C) 2002 by 
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FAKEMONITOR_H
#define FAKEMONITOR_H


/**
  *@author 
  */

class FakeMonitor {
public: 
	FakeMonitor();
	FakeMonitor(double);
	~FakeMonitor();
    double read();
    void write(double);
    void add(double);
private:
    double value;
};

#endif
