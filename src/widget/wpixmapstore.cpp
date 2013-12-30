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

#include "widget/wpixmapstore.h"

#include <QtDebug>

// static
QHash<QString, QWeakPixmapPointer> WPixmapStore::m_pixmapCache;
QSharedPointer<ImgSource> WPixmapStore::m_loader = QSharedPointer<ImgSource>();

// static
QPixmapPointer WPixmapStore::getPixmap(const QString& fileName) {
    // See if we have a cached value for the pixmap.
    QPixmapPointer pPixmap = m_pixmapCache.value(fileName, QPixmapPointer());
    if (pPixmap) {
        return pPixmap;
    }

    // Otherwise, construct it with the pixmap loader.
    //qDebug() << "WPixmapStore Loading pixmap from file" << fileName;
    QPixmap* loadedPixmap = getPixmapNoCache(fileName);

    if (loadedPixmap == NULL || loadedPixmap->isNull()) {
        qDebug() << "WPixmapStore couldn't load:" << fileName
                 << (loadedPixmap == NULL);
        return QPixmapPointer();
    }

    QPixmapPointer pixmapPointer = QPixmapPointer(loadedPixmap);
    m_pixmapCache[fileName] = pixmapPointer;
    return pixmapPointer;
}

// static
QPixmap* WPixmapStore::getPixmapNoCache(const QString& fileName) {
    QPixmap* pPixmap = NULL;
    if (m_loader) {
        QImage* img = m_loader->getImage(fileName);
#if QT_VERSION >= 0x040700
        pPixmap = new QPixmap();
        pPixmap->convertFromImage(*img);
#else
        pPixmap = new QPixmap(QPixmap::fromImage(*img));
#endif
        delete img;
    } else {
        pPixmap = new QPixmap(fileName);
    }
    return pPixmap;
}

void WPixmapStore::setLoader(QSharedPointer<ImgSource> ld) {
    m_loader = ld;

    // We shouldn't hand out pointers to existing pixmaps anymore since our
    // loader has changed. The pixmaps will get freed once all the widgets
    // referring to them are destroyed.
    m_pixmapCache.clear();
}
