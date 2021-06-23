#pragma once

#include "imgsource.h"

class ImgInvert : public ImgColorProcessor {

public:
    inline ImgInvert(ImgSource* parent) : ImgColorProcessor(parent) {}
    QColor doColorCorrection(const QColor& c) const override;
};
