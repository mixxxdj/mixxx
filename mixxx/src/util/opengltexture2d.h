#pragma once

#include <QOpenGLTexture>
#include <memory>

class Paintable;
class QImage;

/// This is an QOpenGLTexture, with additional methods to set the texture data,
/// and default settings for 2D painting with lienar filtering and wrap mode.
class OpenGLTexture2D : public QOpenGLTexture {
  public:
    OpenGLTexture2D();

    void setData(const QImage& image);
    void setData(const QPixmap& pixmap);
    void setData(const Paintable& paintable);
};
