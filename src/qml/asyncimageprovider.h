#pragma once

#include <QImage>
#include <QQuickAsyncImageProvider>
#include <QSize>
#include <QString>
#include <memory>

#include "library/coverart.h"
#include "library/coverartcache.h"
#include "library/trackcollectionmanager.h"

namespace mixxx {
namespace qml {

class AsyncImageResponse : public QQuickImageResponse {
    Q_OBJECT
  public:
    AsyncImageResponse(
            QString id,
            QSize requestedSize,
            std::shared_ptr<TrackCollectionManager> pTrackCollectionManager);

    QQuickTextureFactory* textureFactory() const override;

    /// Must be called on the main thread (the thread that the
    /// TrackCollectionManager lives on) before the response is
    /// returned to the QML engine. It performs the lightweight
    /// CoverInfo lookup, checks the CoverArtCache for a synchronous
    /// cache hit, and otherwise initiates an asynchronous load via
    /// CoverArtCache and connects to its coverFound signal.
    void initialize();

  private slots:
    void slotCoverFound(
            const QObject* pRequester,
            const CoverInfo& coverInfo,
            const QPixmap& pixmap);

  private:
    QString m_id;
    QSize m_requestedSize;
    std::shared_ptr<TrackCollectionManager> m_pTrackCollectionManager;
    CoverInfo m_coverInfo;
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
    std::shared_ptr<TrackCollectionManager> m_pTrackCollectionManager;
};

} // namespace qml
} // namespace mixxx
