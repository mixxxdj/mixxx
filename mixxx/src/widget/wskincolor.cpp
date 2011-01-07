#include "wskincolor.h"

QSharedPointer<ImgSource> WSkinColor::loader;

void WSkinColor::setLoader(QSharedPointer<ImgSource> ld) {
    loader = ld;
}

QColor WSkinColor::getCorrectColor(QColor c) {
    if (loader == 0) {
        return c;
    } else {
        return loader->getCorrectColor(c);
    }
}

