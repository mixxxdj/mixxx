#include "wskincolor.h"

#include "skin/legacy/imgloader.h"

QSharedPointer<ImgSource> WSkinColor::loader
    = QSharedPointer<ImgSource>(new ImgLoader());

void WSkinColor::setLoader(QSharedPointer<ImgSource> ld) {
    loader = ld;
}

QColor WSkinColor::getCorrectColor(QColor c) {
    return loader->getCorrectColor(c);
}
