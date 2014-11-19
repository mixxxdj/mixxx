#include "widget/wimagestore.h"

#include <QtDebug>
#include <QSvgRenderer>
#include <QPainter>

// static
QHash<QString, WImageStore::ImageInfoType*> WImageStore::m_dictionary;
QSharedPointer<ImgSource> WImageStore::m_loader = QSharedPointer<ImgSource>();

// static
QImage* WImageStore::getImageNoCache(const QString& fileName) {
    return getImageNoCache(PixmapSource(fileName));
}

// static
QImage* WImageStore::getImage(const QString& fileName) {
    return getImage(PixmapSource(fileName));
}

// static
QImage* WImageStore::getImage(const PixmapSource& source) {
    // Search for Image in list
    ImageInfoType* info = NULL;

    QHash<QString, ImageInfoType*>::iterator it = m_dictionary.find(source.getId());
    if (it != m_dictionary.end()) {
        info = it.value();
        info->instCount++;
        //qDebug() << "WImageStore returning cached Image for:" << source.getPath();
        return info->image;
    }

    // Image wasn't found, construct it
    //qDebug() << "WImageStore Loading Image from file" << source.getPath();

    QImage* loadedImage = getImageNoCache(source);

    if (loadedImage == NULL) {
        return NULL;
    }

    if (loadedImage->isNull()) {
        qDebug() << "WImageStore couldn't load:" << source.getPath() << (loadedImage == NULL);
        delete loadedImage;
        return NULL;
    }

    info = new ImageInfoType;
    info->image = loadedImage;
    info->instCount = 1;
    m_dictionary.insert(source.getId(), info);
    return info->image;
}

// static
QImage* WImageStore::getImageNoCache(const PixmapSource& source) {
    QImage* pImage;
    if (source.isSVG()) {
        QSvgRenderer renderer;
        if (source.getData().isEmpty()) {
            renderer.load(source.getPath());
        } else {
            renderer.load(source.getData());
        }

        pImage = new QImage(renderer.defaultSize(), QImage::Format_ARGB32);
        pImage->fill(0x00000000);  // Transparent black.
        QPainter painter(pImage);
        renderer.render(&painter);
    } else {
        if (m_loader) {
            pImage = m_loader->getImage(source.getPath());
        } else {
            pImage = new QImage(source.getPath());
        }
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
