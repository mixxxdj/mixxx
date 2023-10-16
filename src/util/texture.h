#include <QImage>
#include <QOpenGLTexture>
#include <QPixmap>
#include <QSharedPointer>
#include <memory>

#include "widget/paintable.h"

std::unique_ptr<QOpenGLTexture> createTexture(const QImage& image);
std::unique_ptr<QOpenGLTexture> createTexture(const QPixmap& pixmap);
std::unique_ptr<QOpenGLTexture> createTexture(const QSharedPointer<Paintable>& pPaintable);
std::unique_ptr<QOpenGLTexture> createTexture(const std::shared_ptr<QImage>& pImage);
