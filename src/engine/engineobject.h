/***************************************************************************
                          engineobject.h  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
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

#ifndef ENGINEOBJECT_H
#define ENGINEOBJECT_H

#include <qobject.h>
#include "defs.h"

/**
  *@author Tue and Ken Haste Andersen
  */

class EngineObject : public QObject {
    Q_OBJECT
public:
    EngineObject();
    virtual ~EngineObject();
    virtual void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iLen) = 0;
//    static int getPlaySrate();

//protected:
//    void setPlaySrate(int srate);

//private:
//    static int PLAY_SRATE;
};


#endif
