/***************************************************************************
                          controlenginequeue.cpp  -  description
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

#include "controlenginequeue.h"
#include "controlengine.h"

ControlEngineQueue::ControlEngineQueue(QPtrList<ControlEngine> *_list)
{
    list = _list;
    queue.setAutoDelete(true);
}

ControlEngineQueue::~ControlEngineQueue()
{
}

void ControlEngineQueue::add(ControlEngineQueueItem *item)
{
    mutex.lock();
    queue.enqueue(item);
    mutex.unlock();
}

void ControlEngineQueue::sync()
{
    // If possible lock mutex and process queue
    if (mutex.tryLock())
    {
        for (int i=0; i<queue.count(); i++)
        {
            ControlEngineQueueItem *item = queue.dequeue();

            //qDebug("item %i, value %f",item->no, item->value);
            list->at(item->no)->setExtern(item->value);
            queue.remove();
            delete item;
        }
        
        // Unlock mutex
        mutex.unlock();
    }
}
