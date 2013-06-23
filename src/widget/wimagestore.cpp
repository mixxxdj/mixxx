#include "wimagestore.h"

#include <QtDebug>

// static
QHash<QString, WImageStore::ImageInfoType*> WImageStore::m_dictionary;
QSharedPointer<ImgSource> WImageStore::m_loader = QSharedPointer<ImgSource>();

// static
QImage * WImageStore::getImage(const QString &fileName) {
    // Search for Image in list
    ImageInfoType* info = NULL;

    QHash<QString, ImageInfoType*>::iterator it = m_dictionary.find(fileName);
    if (it != m_dictionary.end()) {
        info = it.value();
        info->instCount++;
        //qDebug() << "WImageStore returning cached Image for:" << fileName;
        return info->image;
    }

    // Image wasn't found, construct it
    //qDebug() << "WImageStore Loading Image from file" << fileName;
    
    QImage* loadedImage = getImageNoCache(fileName); 

    if (loadedImage == NULL) {
        return NULL; 
    }


    if (loadedImage->isNull()) {
        qDebug() << "WImageStore couldn't load:" << fileName << (loadedImage == NULL);
        delete loadedImage;
        return NULL;
    }

    info = new ImageInfoType;
    info->image = loadedImage;
    info->instCount = 1;
    m_dictionary.insert(fileName, info);
    return info->image;
}

// static
QImage * WImageStore::getImageNoCache(const QString& fileName) {
    QImage* pImage;
    if (m_loader) {
        pImage = m_loader->getImage(fileName);
    } else {
        pImage = new QImage(fileName);
    }
    return pImage; 
}

// static 
void WImageStore::deleteImage(QImage * p)
{
    // Search for Image in list
    ImageInfoType *info = NULL;
    QMutableHashIterator<QString, ImageInfoType*> it(m_dictionary);

    while (it.hasNext())
    {
        info = it.next().value();
        if (p == info->image)
        {
            info->instCount--;
            if (info->instCount<1)
            {
                it.remove();
                delete info->image;
                delete info;
            }
            break;
        }
    }
}

// static
void WImageStore::correctImageColors(QImage* p) {
    if (m_loader) {
        m_loader->correctImageColors(p);
    }
}

// static
void WImageStore::setLoader(QSharedPointer<ImgSource> ld) {
    m_loader = ld;
}


