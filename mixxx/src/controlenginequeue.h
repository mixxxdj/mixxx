/***************************************************************************
                          controlenginequeue.h  -  description
                             -------------------
    begin                : Wed Feb 26 2003
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

#ifndef CONTROLENGINEQUEUE_H
#define CONTROLENGINEQUEUE_H

#include <qptrqueue.h>
#include <qptrlist.h>
#include <qmutex.h>

class ControlEngine;

/**
  *@author Tue & Ken Haste Andersen
  */

struct ControlEngineQueueItem
{
    int no;
    double value;
};

class ControlEngineQueue
{
public: 
    ControlEngineQueue(QPtrList<ControlEngine> *_list);
    ~ControlEngineQueue();
    void add(ControlEngineQueueItem *item);
    void sync();
private:
    QPtrQueue<ControlEngineQueueItem> queue;
    QPtrList<ControlEngine> *list;
    QMutex mutex;
};

#endif
