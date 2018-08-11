
#include <QImageReader>
#include <QFileInfo>

#include "imgloader.h"
#include "widget/wwidget.h"

ImgLoader::ImgLoader() {
}

QImage* ImgLoader::getImage(const QString& fileName) const {
    return new QImage(fileName);
}

