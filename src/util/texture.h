#include <QSharedPointer>
#include <memory>

class QImage;
class QOpenGLTexture;
class QPixmap;
class Paintable;

std::unique_ptr<QOpenGLTexture> createTexture(const QImage& image);
std::unique_ptr<QOpenGLTexture> createTexture(const QPixmap& pixmap);
std::unique_ptr<QOpenGLTexture> createTexture(const QSharedPointer<Paintable>& pPaintable);
std::unique_ptr<QOpenGLTexture> createTexture(const std::shared_ptr<QImage>& pImage);
