#include <QImage>
#include <QOpenGLTexture>
#include <QPixmap>
#include <QSharedPointer>
#include <memory>

#include "widget/paintable.h"

QOpenGLTexture* createTexture(const QImage& image);
QOpenGLTexture* createTexture(const QPixmap& pixmap);
QOpenGLTexture* createTexture(const QSharedPointer<Paintable>& pPaintable);
QOpenGLTexture* createTexture(const std::shared_ptr<QImage>& pImage);
