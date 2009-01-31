#include "wskincolor.h"

ImgSource * WSkinColor::loader = 0;

void WSkinColor::setLoader(ImgSource * ld) {
    loader = ld;
}

QColor WSkinColor::getCorrectColor(QColor c) {
    if (loader == 0) {
        return c;
    } else {
        return loader->getCorrectColor(c);
    }
}

