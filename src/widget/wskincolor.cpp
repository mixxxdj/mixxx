#include "wskincolor.h"

#include "skin/legacy/imgloader.h"

std::shared_ptr<ImgSource> WSkinColor::loader = std::make_shared<ImgLoader>();

void WSkinColor::setLoader(std::shared_ptr<ImgSource> ld) {
    loader = ld;
}

QColor WSkinColor::getCorrectColor(QColor c) {
    return loader->getCorrectColor(c);
}
