#include "wskincolor.h"

QSharedPointer<ImgSource> WSkinColor::loader = QSharedPointer<ImgSource>();

void WSkinColor::setLoader(QSharedPointer<ImgSource> ld) {
    loader = ld;
}

QColor WSkinColor::getCorrectColor(QColor c) {
    if (loader) {
        return loader->getCorrectColor(c);
    } else {
        return c;
    }
}

