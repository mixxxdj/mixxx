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

#include <QPixmap>
#include <QHash>
#include <QSharedPointer>

#include "skin/imgsource.h"

typedef QSharedPointer<QPixmap> QPixmapPointer;
typedef QWeakPointer<QPixmap> QWeakPixmapPointer;

class WPixmapStore {
  public:
    static QPixmapPointer getPixmap(const QString &fileName);
    static QPixmap* getPixmapNoCache(const QString &fileName);
    static void setLoader(QSharedPointer<ImgSource> ld);

  private:
    static QHash<QString, QWeakPixmapPointer> m_pixmapCache;
    static QSharedPointer<ImgSource> m_loader;
};

#endif
