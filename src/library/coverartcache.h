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
     * @param onlyCached : if it is 'true', the method will NOT try to load
     *      covers from the given 'coverLocation' and it will also NOT run the
     *      search algorithm.
     *      In this way, the method will just look into CoverCache and return
     *      a Pixmap if it is already loaded in the QPixmapCache.
     */
    QPixmap requestCover(const CoverInfo& info,
                         const int desiredWidth = 0,
                         const bool onlyCached = false,
                         const bool signalWhenDone = true);

    struct FutureResult {
        CoverArt cover;
        int desiredWidth;
        bool signalWhenDone;
    };

  public slots:
    // Called when loadCover is complete in the main thread.
    void coverLoaded();

  signals:
    void pixmapFound(int trackId, QPixmap pixmap);

  protected:
    CoverArtCache();
    virtual ~CoverArtCache();
    friend class Singleton<CoverArtCache>;

    // Load cover from path indicated in coverInfo. WARNING: This is run in a
    // worker thread.
    FutureResult loadCover(const CoverInfo& coverInfo,
                           const int desiredWidth,
                           const bool emitSignals);

  private:
    QSet<int> m_runningIds;
};

#endif // COVERARTCACHE_H
