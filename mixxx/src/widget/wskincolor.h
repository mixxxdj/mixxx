#pragma once

#include <QColor>
#include <memory>

class ImgSource;

class WSkinColor {
  public:
    static QColor getCorrectColor(QColor c);
    static void setLoader(std::shared_ptr<ImgSource> ld);

  private:
    static std::shared_ptr<ImgSource> loader;
};
