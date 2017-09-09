#include "widget/wimagestore.h"

#include <QtDebug>
#include <QSvgRenderer>
#include <QPainter>

#include "skin/imgloader.h"

// static
QHash<QString, WImageStore::ImageInfoType*> WImageStore::m_dictionary;
QSharedPointer<ImgSource> WImageStore::m_loader
        = QSharedPointer<ImgSource>(new ImgLoader());

// static
QImage* WImageStore::getImageNoCache(const QString& fileName, double scaleFactor) {
    return getImageNoCache(PixmapSource(fileName), scaleFactor);
}

// static
QImage* WImageStore::getImage(const QString& fileName, double scaleFactor) {
    return getImage(PixmapSource(fileName), scaleFactor);
}

// static
QImage* WImageStore::getImage(const PixmapSource& source, double scaleFactor) {
    // Search for Image in list
    ImageInfoType* info = nullptr;
    QString key = source.getId() + QString::number(scaleFactor);

    QHash<QString, ImageInfoType*>::iterator it = m_dictionary.find(key);
    if (it != m_dictionary.end()) {
        info = it.value();
        info->instCount++;
        //qDebug() << "WImageStore returning cached Image for:" << source.getPath();
        return info->image;
    }

    // Image wasn't found, construct it
    //qDebug() << "WImageStore Loading Image from file" << source.getPath();

    QImage* loadedImage = getImageNoCache(source, scaleFactor);

    if (loadedImage == nullptr) {
        return nullptr;
    }

    if (loadedImage->isNull()) {
        qDebug() << "WImageStore couldn't load:" << source.getPath() << (loadedImage == nullptr);
        delete loadedImage;
        return nullptr;
    }

    info = new ImageInfoType;
    info->image = loadedImage;
    info->instCount = 1;
    m_dictionary.insert(key, info);
    return info->image;
}

// static
QImage* WImageStore::getImageNoCache(const PixmapSource& source, double scaleFactor) {
    QImage* pImage;
    if (source.isSVG()) {
        QSvgRenderer renderer;
        if (source.getData().isEmpty()) {
            renderer.load(source.getPath());
        } else {
            renderer.load(source.getData());
        }

        pImage = new QImage(renderer.defaultSize() * scaleFactor,
                            QImage::Format_ARGB32);
        pImage->fill(0x00000000);  // Transparent black.
        QPainter painter(pImage);
        renderer.render(&painter);
    } else {
        pImage = m_loader->getImage(source.getPath(), scaleFactor);
    }
    return pImage;
}

// static
void WImageStore::deleteImage(QImage* p)
{
    // Search for Image in list
    ImageInfoType *info = nullptr;
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
    m_loader->correctImageColors(p);
}

// static
bool WImageStore::willCorrectColors() {
    return m_loader->willCorrectColors();
};

// static
void WImageStore::setLoader(QSharedPointer<ImgSource> ld) {
    m_loader = ld;
}
