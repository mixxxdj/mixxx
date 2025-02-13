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
        : TableItemDelegate(parent),
          m_pTrackModel(asTrackModel(parent)),
          m_pCache(CoverArtCache::instance()),
          m_inhibitLazyLoading(false),
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

void CoverArtDelegate::requestUncachedCover(
        const CoverInfo& coverInfo,
        int width,
        int row) const {
    if (coverInfo.imageDigest().isEmpty()) {
        // This happens if we have the legacy hash
        // The CoverArtCache will take care of the update
        const auto pTrack = m_pTrackModel->getTrackByRef(
                TrackRef::fromFilePath(coverInfo.trackLocation));
        CoverArtCache::requestUncachedCover(this, pTrack, width);
    } else {
        // This is the fast path with an internal temporary track
        CoverArtCache::requestUncachedCover(this, coverInfo, width);
    }
    m_pendingCacheRows.insert(coverInfo.cacheKey(), row);
}

void CoverArtDelegate::slotInhibitLazyLoading(
        bool inhibitLazyLoading) {
    m_inhibitLazyLoading = inhibitLazyLoading;
    if (m_inhibitLazyLoading || m_cacheMissRows.isEmpty()) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrackModel) {
        return;
    }
    const double scaleFactor = m_pTableView->devicePixelRatioF();
    const int width = static_cast<int>(m_pTableView->columnWidth(m_column) * scaleFactor);

    for (int row : std::as_const(m_cacheMissRows)) {
        const QModelIndex index = m_pTableView->model()->index(row, m_column);
        const QRect rect = m_pTableView->visualRect(index);
        if (rect.intersects(m_pTableView->rect())) {
            const CoverInfo coverInfo = m_pTrackModel->getCoverInfo(index);
            requestUncachedCover(coverInfo, width, row);
        }
    }
    m_cacheMissRows.clear();
}

void CoverArtDelegate::slotCoverFound(
        const QObject* pRequester,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap) {
    Q_UNUSED(pixmap);
    if (pRequester != this) {
        return;
    }
    mixxx::cache_key_t requestedImageHash = coverInfo.cacheKey();
    QList<int> refreshRows = m_pendingCacheRows.values(requestedImageHash);
    m_pendingCacheRows.remove(requestedImageHash);
    if (pixmap.isNull()) {
        kLogger.warning() << "Failed to load cover" << coverInfo;
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
    bool drewPixmap = false;
    if (coverInfo.hasImage()) {
        VERIFY_OR_DEBUG_ASSERT(m_pCache) {
            return;
        }
        const double scaleFactor = m_pTableView->devicePixelRatioF();
        const int width = static_cast<int>(option.rect.width() * scaleFactor);
        QPixmap pixmap = CoverArtCache::getCachedCover(coverInfo, width);
        if (pixmap.isNull()) {
            // Cache miss
            if (m_inhibitLazyLoading) {
                // We are requesting cache-only covers and got a cache
                // miss. Record row in a list for later lookup.
                if (!m_cacheMissRows.contains(index.row())) {
                    cleanCacheMissRows();
                    m_cacheMissRows.insert(index.row());
                }
            } else {
                requestUncachedCover(coverInfo, width, index.row());
            }
        } else {
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

void CoverArtDelegate::cleanCacheMissRows() const {
    auto it = m_cacheMissRows.cbegin();
    while (it != m_cacheMissRows.cend()) {
        const QModelIndex index = m_pTableView->model()->index(*it, m_column);
        const QRect rect = m_pTableView->visualRect(index);
        if (!rect.intersects(m_pTableView->rect())) {
            // Cover image row is no longer shown. We keep the set
            // small which likely reuses the allocatd memory later
            it = constErase(&m_cacheMissRows, it);
        } else {
            ++it;
        }
    }
}
