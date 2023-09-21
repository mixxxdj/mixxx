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
            const QObject* pRequester,
            const CoverInfo& coverInfo,
            int desiredWidth = 0) {
        requestCover(pRequester, TrackPointer(), coverInfo, desiredWidth);
    }

    static void requestTrackCover(
            const QObject* pRequester,
            const TrackPointer& pTrack,
            int desiredWidth = 0);

    static QPixmap getCachedCover(
            const CoverInfo& coverInfo,
            int desiredWidth);

    static void requestUncachedCover(
            const QObject* pRequester,
            const CoverInfo& coverInfo,
            int desiredWidth);

    enum class Loading {
        CachedOnly,
        NoSignal,
        Default, // signal when done
    };

    // Only public for testing
    struct FutureResult {
        FutureResult()
                : pRequester(nullptr),
                  requestedCacheKey(CoverImageUtils::defaultCacheKey()),
                  signalWhenDone(false) {
        }
        FutureResult(
                const QObject* pRequestorArg,
                mixxx::cache_key_t requestedCacheKeyArg,
                bool signalWhenDoneArg)
                : pRequester(pRequestorArg),
                  requestedCacheKey(requestedCacheKeyArg),
                  signalWhenDone(signalWhenDoneArg) {
        }

        const QObject* pRequester;
        mixxx::cache_key_t requestedCacheKey;
        bool signalWhenDone;

        CoverArt coverArt;
    };
    // Load cover from path indicated in coverInfo. WARNING: This is run in a
    // worker thread.
    static FutureResult loadCover(
            const QObject* pRequester,
            TrackPointer pTrack,
            CoverInfo coverInfo,
            int desiredWidth,
            bool emitSignals);

  private slots:
    // Called when loadCover is complete in the main thread.
    void coverLoaded();

  signals:
    void coverFound(
            const QObject* requester,
            const CoverInfo& coverInfo,
            const QPixmap& pixmap);

  protected:
    CoverArtCache();
    ~CoverArtCache() override = default;
    friend class Singleton<CoverArtCache>;

  private:
    static void requestCover(
            const QObject* pRequester,
            const TrackPointer& /*optional*/ pTrack,
            const CoverInfo& coverInfo,
            int desiredWidth = 0);

    QPixmap tryLoadCover(
            const QObject* pRequester,
            const TrackPointer& pTrack,
            const CoverInfo& info,
            int desiredWidth,
            Loading loading);

    QSet<QPair<const QObject*, mixxx::cache_key_t>> m_runningRequests;
};

inline
QDebug operator<<(QDebug dbg, CoverArtCache::Loading loading) {
    return dbg << static_cast<int>(loading);
}
