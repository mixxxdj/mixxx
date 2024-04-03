#pragma once

#include <QOpenGLTexture>
#include <QSharedPointer>

class Paintable;

class OpenGLTexture2D : public QOpenGLTexture {
  public:
    OpenGLTexture2D();

    void setData(const QImage& image);
    void setData(const QPixmap& pixmap);
    void setData(const QSharedPointer<Paintable>& pPaintable);
    void setData(const std::shared_ptr<QImage>& pImage);
};
