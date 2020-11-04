#include "library/coverartdelegate.h"

#include <QPainter>
#include <algorithm>

#include "library/coverartcache.h"
#include "library/dao/trackschema.h"
#include "library/trackmodel.h"
#include "track/track.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("CoverArtDelegate");

inline TrackModel* asTrackModel(
        QTableView* pTableView) {
    auto* pTrackModel =
            dynamic_cast<TrackModel*>(pTableView->model());
    DEBUG_ASSERT(pTrackModel);
    return pTrackModel;
}

} // anonymous namespace

CoverArtDelegate::CoverArtDelegate(QTableView* parent)
        : TableItemDelegate(parent),
          m_pTrackModel(asTrackModel(parent)),
          m_pCache(CoverArtCache::instance()),
          m_inhibitLazyLoading(false) {
    if (m_pCache) {
        connect(m_pCache,
                &CoverArtCache::coverFound,
                this,
                &CoverArtDelegate::slotCoverFound);
    } else {
        kLogger.warning()
                << "Caching of cover art is not available";
    }
}

void CoverArtDelegate::emitRowsChanged(
        QList<int>&& rows) {
    if (rows.isEmpty()) {
        return;
    }
    // Sort in ascending order...
    std::sort(rows.begin(), rows.end());
    // ...and then deduplicate...
    rows.erase(std::unique(rows.begin(), rows.end()), rows.end());
    // ...before emitting the signal.
    DEBUG_ASSERT(!rows.isEmpty());
    emit rowsChanged(std::move(rows));
}

void CoverArtDelegate::slotInhibitLazyLoading(
        bool inhibitLazyLoading) {
    m_inhibitLazyLoading = inhibitLazyLoading;
    if (m_inhibitLazyLoading || m_cacheMissRows.isEmpty()) {
        return;
    }
    // If we can request non-cache covers now, request updates
    // for all rows that were cache misses since the last time.
    auto staleRows = m_cacheMissRows;
    // Reset the member variable before mutating the aggregated
    // rows list (-> implicit sharing) and emitting a signal that
    // in turn may trigger new signals for CoverArtDelegate!
    m_cacheMissRows = QList<int>();
    emitRowsChanged(std::move(staleRows));
}

void CoverArtDelegate::slotCoverFound(
        const QObject* pRequestor,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap,
        mixxx::cache_key_t requestedImageHash,
        bool coverInfoUpdated) {
    Q_UNUSED(pixmap);
    if (pRequestor != this) {
        return;
    }
    if (coverInfoUpdated) {
        const auto pTrack =
                loadTrackByLocation(coverInfo.trackLocation);
        if (pTrack) {
            kLogger.info()
                    << "Updating cover info of track"
                    << coverInfo.trackLocation;
            pTrack->setCoverInfo(coverInfo);
        }
    }
    QList<int> refreshRows = m_pendingCacheRows.values(requestedImageHash);
    m_pendingCacheRows.remove(requestedImageHash);
    emitRowsChanged(std::move(refreshRows));
}

TrackPointer CoverArtDelegate::loadTrackByLocation(
        const QString& trackLocation) const {
    VERIFY_OR_DEBUG_ASSERT(m_pTrackModel) {
        return TrackPointer();
    }
    return m_pTrackModel->getTrackByRef(
            TrackRef::fromFileInfo(trackLocation));
}

void CoverArtDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    paintItemBackground(painter, option, index);

    CoverInfo coverInfo = m_pTrackModel->getCoverInfo(index);
    VERIFY_OR_DEBUG_ASSERT(m_pTrackModel) {
        return;
    }
    if (coverInfo.hasImage()) {
        VERIFY_OR_DEBUG_ASSERT(m_pCache) {
            return;
        }
        const double scaleFactor =
                getDevicePixelRatioF(static_cast<QWidget*>(parent()));
        QPixmap pixmap = m_pCache->tryLoadCover(this,
                coverInfo,
                static_cast<int>(option.rect.width() * scaleFactor),
                m_inhibitLazyLoading ? CoverArtCache::Loading::CachedOnly
                                     : CoverArtCache::Loading::Default);
        if (pixmap.isNull()) {
            // Cache miss
            if (m_inhibitLazyLoading) {
                // We are requesting cache-only covers and got a cache
                // miss. Record this row so that when we switch to requesting
                // non-cache we can request an update.
                m_cacheMissRows.append(index.row());
            } else {
                // If we asked for a non-cache image and got a null pixmap,
                // then our request was queued.
                m_pendingCacheRows.insert(coverInfo.cacheKey(), index.row());
            }
        } else {
            // Cache hit
            pixmap.setDevicePixelRatio(scaleFactor);
            painter->drawPixmap(option.rect.topLeft(), pixmap);
            return;
        }
    }
    // Fallback: Use the (background) color as a placeholder:
    //  - while the cover art is loaded asynchronously in the background
    //  - if the audio file with embedded cover art is missing
    //  - if the external image file with custom cover art is missing
    // Since the background color is calculated from the cover art image
    // it is optional and may not always be available
    if (coverInfo.color) {
        painter->fillRect(option.rect, mixxx::RgbColor::toQColor(coverInfo.color));
    }
}
