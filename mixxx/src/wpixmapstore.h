/***************************************************************************
                          wpixmapstore.h  -  description
                             -------------------
    begin                : Mon Jun 28 2003
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

#ifndef WPIXMAPSTORE_H
#define WPIXMAPSTORE_H

#include <qpixmap.h>
#include <qptrlist.h>

/**
  *
  *@author Tue & Ken Haste Andersen
  */

typedef struct
{
    QPixmap *pixmap;
    QString path;
    int instCount;
} PixmapInfoType;

class WPixmapStore {
public: 
    WPixmapStore();
    static QPixmap *getPixmap(const QString &fileName);
    static void deletePixmap(QPixmap *p);
private:
    /** List of pixmaps already instantiated */
    static QPtrList<PixmapInfoType> list;
};

#endif
