#pragma once

#include "imgsource.h"

class ImgLoader : public ImgSource {

public:
    ImgLoader();
    QImage* getImage(const QString &fileName, double scaleFactor) const override;
};
