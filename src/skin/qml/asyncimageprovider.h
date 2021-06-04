#include <QImage>
#include <QQuickAsyncImageProvider>
#include <QRunnable>
#include <QSize>
#include <QString>
#include <QThreadPool>

#include "library/coverart.h"

namespace mixxx {
namespace skin {
namespace qml {

class AsyncImageResponse : public QQuickImageResponse, public QRunnable {
  public:
    AsyncImageResponse(const QString& id, const QSize& requestedSize);
    QQuickTextureFactory* textureFactory() const override;
    void run() override;

    QString m_id;
    QSize m_requestedSize;
    QImage m_image;
};

class AsyncImageProvider : public QQuickAsyncImageProvider {
  public:
    QQuickImageResponse* requestImageResponse(
            const QString& id, const QSize& requestedSize) override;

    static const QString kProviderName;
    static QUrl trackLocationToCoverArtUrl(const QString& trackLocation);
    static QString coverArtUrlIdToTrackLocation(const QString& coverArtUrlId);

  private:
    QThreadPool pool;
};

} // namespace qml
} // namespace skin
} // namespace mixxx
