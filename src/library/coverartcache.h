#pragma once

#include <QObject>
#include <QPair>
#include <QPixmap>
#include <QSet>
#include <QtDebug>

#include "library/coverart.h"
#include "track/track_decl.h"
#include "util/singleton.h"

class CoverArtCache : public QObject, public Singleton<CoverArtCache> {
    Q_OBJECT
  public:
    static void requestCover(
            const QObject* pRequestor,
            const CoverInfo& coverInfo) {
        requestCover(pRequestor, coverInfo, TrackPointer());
    }
    static void requestTrackCover(
            const QObject* pRequestor,
            const TrackPointer& pTrack);

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
     */
    enum class Loading {
        CachedOnly,
        NoSignal,
        Default, // signal when done
    };
    QPixmap tryLoadCover(
            const QObject* pRequestor,
            const CoverInfo& info,
            int desiredWidth = 0, // <= 0: original size
            Loading loading = Loading::Default) {
        return tryLoadCover(
                pRequestor,
                TrackPointer(),
                info,
                desiredWidth,
                loading);
    }

    // Only public for testing
    struct FutureResult {
        FutureResult()
                : pRequestor(nullptr),
                  requestedHash(CoverImageUtils::defaultHash()),
                  signalWhenDone(false),
                  coverInfoUpdated(false) {
        }

        const QObject* pRequestor;
        quint16 requestedHash;
        bool signalWhenDone;

        CoverArt coverArt;
        bool coverInfoUpdated;
    };
    // Load cover from path indicated in coverInfo. WARNING: This is run in a
    // worker thread.
    static FutureResult loadCover(
            const QObject* pRequestor,
            TrackPointer pTrack,
            CoverInfo coverInfo,
            int desiredWidth,
            bool emitSignals);

  private slots:
    // Called when loadCover is complete in the main thread.
    void coverLoaded();

  signals:
    void coverFound(
            const QObject* requestor,
            const CoverInfo& coverInfo,
            const QPixmap& pixmap,
            quint16 requestedHash,
            bool coverInfoUpdated);

  protected:
    CoverArtCache();
    ~CoverArtCache() override = default;
    friend class Singleton<CoverArtCache>;

  private:
    static void requestCover(
            const QObject* pRequestor,
            const CoverInfo& coverInfo,
            const TrackPointer& /*optional*/ pTrack);

    QPixmap tryLoadCover(
            const QObject* pRequestor,
            const TrackPointer& pTrack,
            const CoverInfo& info,
            int desiredWidth,
            Loading loading);

    QSet<QPair<const QObject*, quint16>> m_runningRequests;
};

inline
QDebug operator<<(QDebug dbg, CoverArtCache::Loading loading) {
    return dbg << static_cast<int>(loading);
}
