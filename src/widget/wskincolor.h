#pragma once

#include <QSharedPointer>
#include "skin/legacy/imgsource.h"

class WSkinColor {
  public:
    static QColor getCorrectColor(QColor c);
    static void setLoader(QSharedPointer<ImgSource> ld);
  private:
    static QSharedPointer<ImgSource> loader;
};
