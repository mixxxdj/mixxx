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

// Visible covers are requested regardless
constexpr int kMaxPrefetchInFlight = 4;

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
          m_cellWidth(0),
          m_pCache(CoverArtCache::instance()),
          m_column(m_pTrackModel->fieldIndex(LIBRARYTABLE_COVERART)) {
    if (m_pCache) {
        connect(m_pCache,
                &CoverArtCache::coverFound,
                this,
                &CoverArtDelegate::slotCoverFound);
    } else {
        kLogger.warning()
                << "Caching of cover art is not available";
    }
    if (m_pCache) {
        enablePrefetch();
    }
}

void CoverArtDelegate::prefetchWindow() {
    if (m_cellWidth <= 0) {
        return;
    }
    QHash<mixxx::cache_key_t, qsizetype> queued;
    m_prefetchQueue.clear();
    // Most covers are square
    const qint64 bytesPerRow = qint64{m_cellWidth} * m_cellWidth * 4;
    int visibleCount = 0;
    const QList<int> rows = prefetchRows(bytesPerRow, &visibleCount);
    for (qsizetype i = 0; i < rows.size(); ++i) {
        const int row = rows[i];
        const QModelIndex index = m_pTableView->model()->index(row, m_column);
        const CoverInfo coverInfo = m_pTrackModel->getCoverInfo(index);
        const mixxx::cache_key_t key = coverInfo.cacheKey();
        if (!coverInfo.hasImage() || m_noImageKeys.contains(key)) {
            continue;
        }
        // Legacy covers load the track and may update its metadata,
        // so only do that for visible rows
        if (i >= visibleCount && coverInfo.imageDigest().isEmpty()) {
            continue;
        }
        const auto it = queued.constFind(key);
        if (it != queued.constEnd()) {
            m_prefetchQueue[*it].rows.append(row);
        } else if (CoverArtCache::getCachedCover(coverInfo, m_cellWidth).isNull()) {
            queued.insert(key, m_prefetchQueue.size());
            m_prefetchQueue.append({coverInfo, {row}, i < visibleCount});
        }
    }
    pumpPrefetchQueue();
}

void CoverArtDelegate::prefetchRowsReplaced() {
    // Retry missing covers, e.g. after the drive was mounted again
    m_noImageKeys.clear();
}

void CoverArtDelegate::pumpPrefetchQueue() {
    if (!isPrefetchActive()) {
        m_prefetchQueue.clear();
        return;
    }
    // Visible items are at the front of the queue
    while (!m_prefetchQueue.isEmpty() &&
            (m_prefetchQueue.first().visible ||
                    m_prefetchInFlight.size() < kMaxPrefetchInFlight)) {
        const PrefetchItem item = m_prefetchQueue.takeFirst();
        const mixxx::cache_key_t key = item.coverInfo.cacheKey();
        // Rows may have moved since the request (sorting), so
        // also refresh the current ones when it finishes
        for (int row : item.rows) {
            m_pendingCacheRows.insert(key, row);
        }
        if (m_prefetchInFlight.contains(key)) {
            continue;
        }
        // Insert before requesting, coverFound() may be emitted synchronously
        m_prefetchInFlight.insert(key);
        if (!requestUncachedCover(item.coverInfo, m_cellWidth)) {
            m_prefetchInFlight.remove(key);
            m_pendingCacheRows.remove(key);
        }
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
        CoverArtCache::requestUncachedCover(this, pTrack, width);
        // The result is reported with the track's key, which differs
        // from ours if the model is stale
        return pTrack->getCoverInfo().cacheKey() == coverInfo.cacheKey();
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
    if (m_prefetchInFlight.remove(requestedImageHash)) {
        pumpPrefetchQueue();
    }
    if (pixmap.isNull()) {
        // CoverArtCache already logged a warning
        kLogger.debug() << "Failed to load cover" << coverInfo;
        m_noImageKeys.insert(requestedImageHash);
    } else {
        // Only refresh the rows if loading succeeded, otherwise
        // we would enter an infinite repaint loop and churn CPU.
        emitRowsChanged(std::move(refreshRows));
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
    if (!coverInfo.hasImage()) {
        // Rows further down may have covers
        schedulePrefetch();
    } else {
        VERIFY_OR_DEBUG_ASSERT(m_pCache) {
            return;
        }
        // Cache misses are loaded by prefetchWindow()
        QPixmap pixmap = CoverArtCache::getCachedCover(coverInfo, width);
        const mixxx::cache_key_t key = coverInfo.cacheKey();
        schedulePrefetch(pixmap.isNull() &&
                !m_prefetchInFlight.contains(key) &&
                !m_noImageKeys.contains(key));
        if (!pixmap.isNull()) {
            // Cache hit
            pixmap.setDevicePixelRatio(scaleFactor);
            painter->drawPixmap(option.rect.topLeft(), pixmap);
            drewPixmap = true;
        }
    }
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
