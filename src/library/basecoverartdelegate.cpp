#include "library/coverartdelegate.h"

#include <QPainter>
#include <algorithm>

#include "library/coverartcache.h"
#include "library/dao/trackschema.h"
#include "library/trackmodel.h"
#include "util/logger.h"
#include "widget/wlibrarytableview.h"

namespace {

const mixxx::Logger kLogger("BaseCoverArtDelegate");

inline TrackModel* asTrackModel(
        QTableView* pTableView) {
    auto* pTrackModel =
            dynamic_cast<TrackModel*>(pTableView->model());
    DEBUG_ASSERT(pTrackModel);
    return pTrackModel;
}

} // anonymous namespace

BaseCoverArtDelegate::BaseCoverArtDelegate(QTableView* parent)
        : TableItemDelegate(parent),
          m_pTrackModel(asTrackModel(parent)),
          m_pCache(CoverArtCache::instance()),
          m_inhibitLazyLoading(false) {
    if (m_pCache) {
        connect(m_pCache,
                &CoverArtCache::coverFound,
                this,
                &BaseCoverArtDelegate::slotCoverFound);
    } else {
        kLogger.warning()
                << "Caching of cover art is not available";
    }
}

void BaseCoverArtDelegate::emitRowsChanged(
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

void BaseCoverArtDelegate::slotInhibitLazyLoading(
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
    // in turn may trigger new signals for BaseCoverArtDelegate!
    m_cacheMissRows = QList<int>();
    emitRowsChanged(std::move(staleRows));
}

void BaseCoverArtDelegate::slotCoverFound(
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

TrackPointer BaseCoverArtDelegate::loadTrackByLocation(
        const QString& trackLocation) const {
    VERIFY_OR_DEBUG_ASSERT(m_pTrackModel) {
        return TrackPointer();
    }
    return m_pTrackModel->getTrackByRef(
            TrackRef::fromFileInfo(trackLocation));
}

void BaseCoverArtDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    paintItemBackground(painter, option, index);

    CoverInfo coverInfo = coverInfoForIndex(index);
    if (CoverImageUtils::isValidHash(coverInfo.hash)) {
        VERIFY_OR_DEBUG_ASSERT(m_pCache) {
            return;
        }
        const double scaleFactor =
                getDevicePixelRatioF(static_cast<QWidget*>(parent()));
        QPixmap pixmap = m_pCache->tryLoadCover(
                this,
                coverInfo,
                option.rect.width() * scaleFactor,
                m_inhibitLazyLoading ? CoverArtCache::Loading::CachedOnly : CoverArtCache::Loading::Default);
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
                m_pendingCacheRows.insertMulti(coverInfo.hash, index.row());
            }
        } else {
            // Cache hit
            pixmap.setDevicePixelRatio(scaleFactor);
            painter->drawPixmap(option.rect.topLeft(), pixmap);
            return;
        }
    }
}
