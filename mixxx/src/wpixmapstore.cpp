/***************************************************************************
                          wpixmapstore.cpp  -  description
                             -------------------
    begin                : Mon Jul 28 2003
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

#include "wpixmapstore.h"

QPtrList<PixmapInfoType> WPixmapStore::list;

WPixmapStore::WPixmapStore() 
{
}

QPixmap *WPixmapStore::getPixmap(const QString &fileName)
{
    // Search for pixmap in list
    PixmapInfoType *info;
    for (info = list.first(); info; info = list.next())
    {
        if (fileName == info->path)
        {
            info->instCount++;
            return info->pixmap;
        }
    }

    // Pixmap wasn't found, construct it
//    qDebug("Loading pixmap %s",fileName.latin1());
    info = new PixmapInfoType;
    info->path = QString(fileName);
    info->pixmap = new QPixmap(fileName);
    info->instCount = 1;

    list.append(info);

    return info->pixmap;
}

void WPixmapStore::deletePixmap(QPixmap *p)
{
    // Search for pixmap in list
    PixmapInfoType *info;
    for (info = list.first(); info; info = list.next())
    {
        if (p == info->pixmap)
        {
            info->instCount--;
            if (info->instCount<1)
            {
                delete info->pixmap;
                list.remove(info);
                delete info;
            }
        }
    }
}
    
