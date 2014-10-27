#ifndef COVERARTCACHE_H
#define COVERARTCACHE_H

#include <QObject>
#include <QPixmap>

#include "library/coverart.h"
#include "util/singleton.h"

class CoverArtCache : public QObject, public Singleton<CoverArtCache> {
    Q_OBJECT
  public:
    /* This method is used to request a cover art pixmap.
     *
     * @param croppedSize : QSize(finalCoverWidth, finalCoverHeight)
     *      it determines the final cover size.
     *      Use QSize() to get the original size.
     *      NOTE!
     *          the cover will be resized to 'finalCoverWidth' and
     *          it'll be cropped from the top until the finalCoverHeight' pixel
     *
     * @param onlyCached : if it is 'true', the method will NOT try to load
     *      covers from the given 'coverLocation' and it will also NOT run the
     *      search algorithm.
     *      In this way, the method will just look into CoverCache and return
     *      a Pixmap if it is already loaded in the QPixmapCache.
     */
    QPixmap requestCover(const CoverInfo& info,
                         const QSize& croppedSize = QSize(0,0),
                         const bool onlyCached = false,
                         const bool issueRepaint = false);

    struct FutureResult {
        CoverArt cover;
        QSize croppedSize;
        bool issueRepaint;
    };

  public slots:
    // Called when loadCover is complete in the main thread.
    void coverLoaded();

  signals:
    void pixmapFound(int trackId, QPixmap pixmap);
    void requestRepaint();

  protected:
    CoverArtCache();
    virtual ~CoverArtCache();
    friend class Singleton<CoverArtCache>;

    // Load cover from path indicated in coverInfo. WARNING: This is run in a
    // worker thread.
    FutureResult loadCover(const CoverAndAlbumInfo& coverInfo,
                           const QSize &croppedSize,
                           const bool emitSignals);

  private:
    QSet<int> m_runningIds;
};

#endif // COVERARTCACHE_H
