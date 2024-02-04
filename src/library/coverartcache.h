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
            const CoverInfo& coverInfo) {
        requestCoverImpl(pRequester, TrackPointer(), coverInfo);
    }

    static void requestTrackCover(
            const QObject* pRequester,
            const TrackPointer& pTrack);

    static QPixmap getCachedCover(
            const CoverInfo& coverInfo,
            int desiredWidth);

    static void requestUncachedCover(
            const QObject* pRequester,
            const CoverInfo& coverInfo,
            int desiredWidth);

    static void requestUncachedCover(
            const QObject* pRequester,
            const TrackPointer& pTrack,
            int desiredWidth);

    // Only public for testing
    struct FutureResult {
        FutureResult()
                : requestedCacheKey(CoverImageUtils::defaultCacheKey()) {
        }
        FutureResult(
                mixxx::cache_key_t requestedCacheKeyArg)
                : requestedCacheKey(requestedCacheKeyArg) {
        }
        mixxx::cache_key_t requestedCacheKey;
        CoverArt coverArt;
    };
    // Load cover from path indicated in coverInfo. WARNING: This is run in a
    // worker thread.
    static FutureResult loadCover(
            TrackPointer pTrack,
            CoverInfo coverInfo,
            int desiredWidth);

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
    static void requestCoverImpl(
            const QObject* pRequester,
            const TrackPointer& /*optional*/ pTrack,
            const CoverInfo& coverInfo,
            int desiredWidth = 0); // <= 0: original size

    void tryLoadCover(
            const QObject* pRequester,
            const TrackPointer& pTrack,
            const CoverInfo& info,
            int desiredWidth);

    struct RequestData {
        const QObject* pRequester;
        int desiredWidth;
    };
    QMultiHash<mixxx::cache_key_t, RequestData> m_runningRequests;
};
