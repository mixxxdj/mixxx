#include "library/tabledelegates/coverartdelegate.h"

#include <QPainter>
#include <QTableView>
#include <algorithm>

#include "library/coverartcache.h"
#include "library/dao/trackschema.h"
#include "library/trackmodel.h"
#include "moc_coverartdelegate.cpp"
#include "track/track.h"
#include "util/logger.h"
#include "util/make_const_iterator.h"

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
        : CachedTableItemDelegate(parent,
                  asTrackModel(parent)->fieldIndex(LIBRARYTABLE_COVERART)),
          m_pTrackModel(asTrackModel(parent)),
          m_pCache(CoverArtCache::instance()) {
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

qint64 CoverArtDelegate::prefetchBytesPerRow() const {
    if (!m_pCache) {
        return 0;
    }
    // Most covers are square
    return qint64{m_cellWidth} * m_cellWidth * 4;
}

void CoverArtDelegate::prefetchRow(int row, bool visible) {
    const QModelIndex index = m_pTableView->model()->index(row, m_column);
    const CoverInfo coverInfo = m_pTrackModel->getCoverInfo(index);
    const mixxx::cache_key_t key = coverInfo.cacheKey();
    if (!coverInfo.hasImage() || m_noImageKeys.contains(key)) {
        return;
    }
    if (m_prefetchInFlight.contains(key)) {
        // paintItem() records visible rows that wait for it
        return;
    }
    // Legacy covers load the track and may update its metadata,
    // so only do that for visible rows
    if (!visible && coverInfo.imageDigest().isEmpty()) {
        return;
    }
    if (!CoverArtCache::getCachedCover(coverInfo, m_cellWidth).isNull()) {
        return;
    }
    // Insert first, coverFound() may be emitted synchronously. Only visible
    // rows are refreshed, each refresh makes the QML library render.
    if (visible) {
        m_pendingCacheRows.insert(key, row);
    }
    m_prefetchInFlight.insert(key);
    if (!requestUncachedCover(coverInfo, m_cellWidth)) {
        m_prefetchInFlight.remove(key);
        m_pendingCacheRows.remove(key);
        // Don't retry on every pass
        m_noImageKeys.insert(key);
    }
}

void CoverArtDelegate::prefetchRowsReplaced() {
    // Retry missing covers, e.g. after the drive was mounted again. Rows
    // and keys may have changed (legacy covers), so forget the requests,
    // CoverArtCache merges repeated ones.
    m_noImageKeys.clear();
    m_pendingCacheRows.clear();
    m_prefetchInFlight.clear();
}

void CoverArtDelegate::emitRowsChanged(
        QList<int>&& rows) {
    if (rows.isEmpty()) {
        return;
    }
    // Sort in ascending order...
    std::sort(rows.begin(), rows.end());
    // ...and then deduplicate...
    constErase(
            &rows,
            make_const_iterator(
                    rows, std::unique(rows.begin(), rows.end())),
            rows.constEnd());
    // ...before emitting the signal.
    DEBUG_ASSERT(!rows.isEmpty());
    emit rowsChanged(std::move(rows));
}

bool CoverArtDelegate::requestUncachedCover(
        const CoverInfo& coverInfo,
        int width) {
    if (coverInfo.imageDigest().isEmpty()) {
        // This happens if we have the legacy hash
        // The CoverArtCache will take care of the update
        const auto pTrack = m_pTrackModel->getTrackByRef(
                TrackRef::fromFilePath(coverInfo.trackLocation));
        if (!pTrack) {
            return false;
        }
        // The result is reported with the track's key, which differs
        // from ours if the model is stale. Compare before requesting,
        // the request may update the track's cover from another thread.
        const bool sameKey = pTrack->getCoverInfo().cacheKey() == coverInfo.cacheKey();
        CoverArtCache::requestUncachedCover(this, pTrack, width);
        return sameKey;
    } else {
        // This is the fast path with an internal temporary track
        CoverArtCache::requestUncachedCover(this, coverInfo, width);
    }
    return true;
}

void CoverArtDelegate::slotCoverFound(
        const QObject* pRequester,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap) {
    if (pRequester != this) {
        return;
    }
    mixxx::cache_key_t requestedImageHash = coverInfo.cacheKey();
    QList<int> refreshRows = m_pendingCacheRows.values(requestedImageHash);
    m_pendingCacheRows.remove(requestedImageHash);
    if (pixmap.isNull()) {
        // CoverArtCache already logged a warning
        kLogger.debug() << "Failed to load cover" << coverInfo;
        m_noImageKeys.insert(requestedImageHash);
    } else {
        // Only refresh the rows if loading succeeded, otherwise
        // we would enter an infinite repaint loop and churn CPU.
        emitRowsChanged(std::move(refreshRows));
    }
    if (m_prefetchInFlight.remove(requestedImageHash)) {
        processPrefetchQueue();
    }
}

void CoverArtDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    VERIFY_OR_DEBUG_ASSERT(m_pTrackModel) {
        return;
    }
    CoverInfo coverInfo = m_pTrackModel->getCoverInfo(index);
    const double scaleFactor = m_pTableView->devicePixelRatioF();
    const int width = static_cast<int>(option.rect.width() * scaleFactor);
    m_cellWidth = width;
    bool drewPixmap = false;
    bool loadNow = false;
    if (coverInfo.hasImage()) {
        VERIFY_OR_DEBUG_ASSERT(m_pCache) {
            return;
        }
        // Cache misses are loaded by prefetchRow()
        QPixmap pixmap = CoverArtCache::getCachedCover(coverInfo, width);
        const mixxx::cache_key_t key = coverInfo.cacheKey();
        if (pixmap.isNull() && m_prefetchInFlight.contains(key)) {
            // Requested for another row with the same cover, or this row
            // moved since the request (sorting)
            if (!m_pendingCacheRows.contains(key, index.row())) {
                m_pendingCacheRows.insert(key, index.row());
            }
        } else {
            loadNow = pixmap.isNull() && !m_noImageKeys.contains(key);
        }
        if (!pixmap.isNull()) {
            // Cache hit
            pixmap.setDevicePixelRatio(scaleFactor);
            painter->drawPixmap(option.rect.topLeft(), pixmap);
            drewPixmap = true;
        }
    }
    // Also without an image, rows further down may have one
    schedulePrefetch(loadNow);
    // Fallback: Use the (background) color as a placeholder:
    //  - while the cover art is loaded asynchronously in the background
    //  - if the audio file with embedded cover art is missing
    //  - if the external image file with custom cover art is missing
    // Since the background color is calculated from the cover art image
    // it is optional and may not always be available. The background
    // color may even be set manually without having a cover image.
    if (!drewPixmap) {
        if (coverInfo.color) {
            painter->fillRect(option.rect, mixxx::RgbColor::toQColor(coverInfo.color));
        } else {
            paintItemBackground(painter, option, index);
        }
    }

    // Draw a border if the cover art cell has focus
    if (option.state & QStyle::State_HasFocus) {
        drawBorder(painter, m_focusBorderColor, option.rect);
    }
}
