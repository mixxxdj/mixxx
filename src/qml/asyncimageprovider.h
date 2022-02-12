#include <QImage>
#include <QQuickAsyncImageProvider>
#include <QRunnable>
#include <QSize>
#include <QString>
#include <QThreadPool>
#include <memory>

#include "library/coverart.h"
#include "library/trackcollectionmanager.h"

namespace mixxx {
namespace qml {

class AsyncImageResponse : public QQuickImageResponse, public QRunnable {
    Q_OBJECT
  public:
    AsyncImageResponse(
            QString id,
            QSize requestedSize,
            std::shared_ptr<TrackCollectionManager> pTrackCollectionManager);

    QQuickTextureFactory* textureFactory() const override;

    void run() override;

  private:
    QString m_id;
    QSize m_requestedSize;
    std::shared_ptr<TrackCollectionManager> m_pTrackCollectionManager;

    QImage m_image;
};

class AsyncImageProvider : public QQuickAsyncImageProvider {
  public:
    AsyncImageProvider(std::shared_ptr<TrackCollectionManager> pTrackCollectionManager);

    QQuickImageResponse* requestImageResponse(
            const QString& id, const QSize& requestedSize) override;

    static const QString kProviderName;
    static QUrl trackLocationToCoverArtUrl(const QString& trackLocation);
    static QString coverArtUrlIdToTrackLocation(const QString& coverArtUrlId);

  private:
    QThreadPool pool;
    std::shared_ptr<TrackCollectionManager> m_pTrackCollectionManager;
};

} // namespace qml
} // namespace mixxx
