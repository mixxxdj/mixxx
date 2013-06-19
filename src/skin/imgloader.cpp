#include "imgloader.h"
#include "widget/wwidget.h"

ImgLoader::ImgLoader() {
}

QImage * ImgLoader::getImage(QString img) {
    return new QImage(img);
}

