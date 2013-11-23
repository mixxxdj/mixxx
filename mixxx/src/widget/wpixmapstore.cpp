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

#include <QPixmap>
#include <QtDebug>

// static
QHash<QString, WPixmapStore::PixmapInfoType*> WPixmapStore::m_dictionary;
QSharedPointer<ImgSource> WPixmapStore::m_loader = QSharedPointer<ImgSource>();

// static
QPixmap * WPixmapStore::getPixmap(const QString &fileName) {
    // Search for pixmap in list
    PixmapInfoType* info = NULL;

    QHash<QString, PixmapInfoType*>::iterator it = m_dictionary.find(fileName);
    if (it != m_dictionary.end()) {
        info = it.value();
        info->instCount++;
        //qDebug() << "WPixmapStore returning cached pixmap for:" << fileName;
        return info->pixmap;
    }

    // Pixmap wasn't found, construct it
    //qDebug() << "WPixmapStore Loading pixmap from file" << fileName;
    
    QPixmap* loadedPixmap = getPixmapNoCache(fileName);

    if (loadedPixmap == NULL || loadedPixmap->isNull()) {
        qDebug() << "WPixmapStore couldn't load:" << fileName << (loadedPixmap == NULL);
        delete loadedPixmap;
        return NULL;
    }

    info = new PixmapInfoType;
    info->pixmap = loadedPixmap;
    info->instCount = 1;
    m_dictionary.insert(fileName, info);
    return info->pixmap;
}

// static
QPixmap * WPixmapStore::getPixmapNoCache(const QString& fileName) {
    QPixmap* pPixmap;
    if (m_loader) {
        QImage * img = m_loader->getImage(fileName);
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

// static 
void WPixmapStore::deletePixmap(QPixmap * p)
{
    // Search for pixmap in list
    PixmapInfoType *info = NULL;
    QMutableHashIterator<QString, PixmapInfoType*> it(m_dictionary);

    while (it.hasNext())
    {
        info = it.next().value();
        if (p == info->pixmap)
        {
            info->instCount--;
            if (info->instCount<1)
            {
                it.remove();
                delete info->pixmap;
                delete info;
            }

            break;
        }
    }
}

void WPixmapStore::setLoader(QSharedPointer<ImgSource> ld) {
    m_loader = ld;
}
