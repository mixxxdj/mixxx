#pragma once

#include <QSharedPointer>

class ImgSource;

class WSkinColor {
  public:
    static QColor getCorrectColor(QColor c);
    static void setLoader(QSharedPointer<ImgSource> ld);
  private:
    static QSharedPointer<ImgSource> loader;
};
