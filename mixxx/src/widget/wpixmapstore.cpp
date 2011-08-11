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
//Added by qt3to4:
#include <QPixmap>
#include <QtDebug>

QHash<QString, PixmapInfoType*> WPixmapStore::dictionary;

QSharedPointer<ImgSource> WPixmapStore::loader = QSharedPointer<ImgSource>();

QPixmap * WPixmapStore::getPixmap(const QString &fileName)
{
    // Search for pixmap in list
    PixmapInfoType* info = NULL;

    QHash<QString, PixmapInfoType*>::iterator it = dictionary.find(fileName);
    if (it != dictionary.end()) {
        info = it.value();
        info->instCount++;
        //qDebug() << "WPixmapStore returning cached pixmap for:" << fileName;
        return info->pixmap;
    }

    // Pixmap wasn't found, construct it
    //qDebug() << "WPixmapStore Loading pixmap from file" << fileName;

    QPixmap* loadedPixmap = NULL;
    if (loader) {
        QImage* img = loader->getImage(fileName);

        if (img != NULL && !img->isNull()) {
            // ack, hacky; there must be a better way (we're using pixmap
            // pointers, but perhaps qt4 expects that you'll just copy?) --kousu
            // 2009/03
            loadedPixmap = new QPixmap(QPixmap::fromImage(*img));
        }
        // No longer need the original QImage (I hope...) - adam_d
        delete img;
    } else {
        loadedPixmap = new QPixmap(fileName);
    }

    if (loadedPixmap == NULL || loadedPixmap->isNull()) {
        qDebug() << "WPixmapStore couldn't load:" << fileName << (loadedPixmap == NULL);
        delete loadedPixmap;
        return NULL;
    }

    info = new PixmapInfoType;
    info->pixmap = loadedPixmap;
    info->instCount = 1;
    dictionary.insert(fileName, info);
    return info->pixmap;
}

QPixmap * WPixmapStore::getPixmapNoCache(const QString& fileName) {
    if (loader) {
        QImage * img = loader->getImage(fileName);
        QPixmap r = QPixmap::fromImage(*img);
        delete img;
        return new QPixmap(r); //ack, hacky; there must be a better way (we're using pixmap pointers, but perhaps qt4 expects that you'll just copy?) --kousu 2009/03
    } else {
        return new QPixmap(fileName);
    }
}

void WPixmapStore::deletePixmap(QPixmap * p)
{
    // Search for pixmap in list
    PixmapInfoType *info = NULL;
    QMutableHashIterator<QString, PixmapInfoType*> it(dictionary);

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

void WPixmapStore::emptyStore() {

}

void WPixmapStore::setLoader(QSharedPointer<ImgSource> ld) {
    loader = ld;
}
