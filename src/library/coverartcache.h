#ifndef COVERARTCACHE_H
#define COVERARTCACHE_H

#include <QObject>
#include <QPixmap>

#include "library/coverart.h"
#include "util/singleton.h"
#include "trackinfoobject.h"

class CoverArtCache : public QObject, public Singleton<CoverArtCache> {
    Q_OBJECT
  public:
    /* This method is used to request a cover art pixmap.
     *
     * @param pRequestor : an arbitrary pointer (can be any number you'd like,
     *      really) that will be provided in the coverFound signal for this
     *      request. This allows you to match requests with their responses.
     *
     * @param onlyCached : if it is 'true', the method will NOT try to load
     *      covers from the given 'coverLocation' and it will also NOT run the
     *      search algorithm.
     *      In this way, the method will just look into CoverCache and return
     *      a Pixmap if it is already loaded in the QPixmapCache.
     *
     * TODO(rryan): Provide a QObject* and a SLOT to invoke directly. Why make
     * everyone filter the signals they receive?
     */
    QPixmap requestCover(const CoverInfo& info,
                         const QObject* pRequestor,
                         const int requestReference,
                         const int desiredWidth = 0,
                         const bool onlyCached = false,
                         const bool signalWhenDone = true);

    // Guesses the cover art for the provided tracks by searching the tracks'
    // metadata and folders for image files. All I/O is done in a separate
    // thread.
    void requestGuessCovers(QList<TrackPointer> tracks);
    void requestGuessCover(TrackPointer pTrack);

    struct FutureResult {
        FutureResult() : pRequestor(NULL),
                         requestReference(0),
                         desiredWidth(0),
                         signalWhenDone(false) {
        }

        CoverArt cover;
        const QObject* pRequestor;
        int requestReference;
        int desiredWidth;
        bool signalWhenDone;
    };

  public slots:
    // Called when loadCover is complete in the main thread.
    void coverLoaded();

  signals:
    void coverFound(const QObject* requestor, int requestReference,
                    const CoverInfo& info, QPixmap pixmap, bool fromCache);

  protected:
    CoverArtCache();
    virtual ~CoverArtCache();
    friend class Singleton<CoverArtCache>;

    // Load cover from path indicated in coverInfo. WARNING: This is run in a
    // worker thread.
    FutureResult loadCover(const CoverInfo& coverInfo,
                           const QObject* pRequestor,
                           const int requestReference,
                           const int desiredWidth,
                           const bool emitSignals);

    // Guesses the cover art for each track.
    void guessCovers(QList<TrackPointer> tracks);
    void guessCover(TrackPointer pTrack);

  private:
    QSet<QPair<const QObject*, int> > m_runningRequests;
};

#endif // COVERARTCACHE_H
