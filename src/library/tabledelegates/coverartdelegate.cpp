#include "library/tabledelegates/coverartdelegate.h"

#include <QPainter>
#include <QTableView>

#include "library/coverartcache.h"
#include "library/dao/trackschema.h"
#include "library/trackmodel.h"
#include "moc_coverartdelegate.cpp"
#include "track/track.h"
#include "util/logger.h"
#include "util/time.h"

namespace {

const mixxx::Logger kLogger("CoverArtDelegate");

// E.g. the drive may be mounted again
const mixxx::Duration kMissingCoverRetryDelay = mixxx::Duration::fromSeconds(10);

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

bool CoverArtDelegate::prefetchRow(int row, bool visible) {
    const QModelIndex index = m_pTableView->model()->index(row, m_column);
    const CoverInfo coverInfo = m_pTrackModel->getCoverInfo(index);
    const mixxx::cache_key_t key = coverInfo.cacheKey();
    if (!coverInfo.hasImage() || isKnownMissing(key)) {
        return true;
    }
    if (m_prefetchInFlight.contains(key)) {
        // paintItem() records visible rows that wait for it
        return false;
    }
    // Legacy covers load the track and may update its metadata,
    // so only do that for visible rows
    if (!visible && coverInfo.imageDigest().isEmpty()) {
        return true;
    }
    if (!CoverArtCache::getCachedCover(coverInfo, m_cellWidth).isNull()) {
        return true;
    }
    // Insert first, coverFound() may be emitted synchronously
    m_pendingCacheRows.insert(key, row);
    m_prefetchInFlight.insert(key);
    if (!requestUncachedCover(coverInfo, m_cellWidth)) {
        m_prefetchInFlight.remove(key);
        m_pendingCacheRows.remove(key);
        m_missingSince.insert(key, mixxx::Time::elapsed());
        return true;
    }
    return false;
}

void CoverArtDelegate::prefetchRowsReplaced() {
    // Retry missing covers. Rows moved, they are recorded again when painted.
    m_missingSince.clear();
    m_pendingCacheRows.clear();
}

bool CoverArtDelegate::isKnownMissing(mixxx::cache_key_t key) const {
    const auto it = m_missingSince.constFind(key);
    return it != m_missingSince.constEnd() &&
            mixxx::Time::elapsed() - it.value() < kMissingCoverRetryDelay;
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
        // The result is reported with the track's key, which differs from
        // ours if the model is stale. The request may change it once, from
        // the legacy hash to the digest, from another thread.
        const auto key = coverInfo.cacheKey();
        const bool keyBefore = pTrack->getCoverInfo().cacheKey() == key;
        CoverArtCache::requestUncachedCover(this, pTrack, width);
        return keyBefore && pTrack->getCoverInfo().cacheKey() == key;
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
        m_missingSince.insert(requestedImageHash, mixxx::Time::elapsed());
    } else {
        // Only refresh the rows if loading succeeded, otherwise
        // we would enter an infinite repaint loop and churn CPU.
        // Only visible rows, each refresh makes the QML library render.
        refreshRows = rowsToRefresh(std::move(refreshRows), true);
        if (!refreshRows.isEmpty()) {
            emit rowsChanged(refreshRows);
        }
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
            loadNow = pixmap.isNull() && !isKnownMissing(key);
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
