#include "library/coverartdelegate.h"

#include <QPainter>
#include <algorithm>

#include "library/coverartcache.h"
#include "library/dao/trackschema.h"
#include "library/trackmodel.h"
#include "moc_coverartdelegate.cpp"
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
    // Reset the member variable before mutating the aggregated
    // rows list (-> implicit sharing) and emitting a signal that
    // in turn may trigger new signals for CoverArtDelegate!
    QList<int> staleRows = std::move(m_cacheMissRows);
    DEBUG_ASSERT(m_cacheMissRows.isEmpty());
    emitRowsChanged(std::move(staleRows));
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

TrackPointer CoverArtDelegate::loadTrackByLocation(
        const QString& trackLocation) const {
    VERIFY_OR_DEBUG_ASSERT(m_pTrackModel) {
        return TrackPointer();
    }
    return m_pTrackModel->getTrackByRef(
            TrackRef::fromFilePath(trackLocation));
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
    bool drewPixmap = false;
    if (coverInfo.hasImage()) {
        VERIFY_OR_DEBUG_ASSERT(m_pCache) {
            return;
        }
        const double scaleFactor = qobject_cast<QWidget*>(parent())->devicePixelRatioF();
        QPixmap pixmap = CoverArtCache::getCachedCover(
                coverInfo,
                static_cast<int>(option.rect.width() * scaleFactor));
        if (pixmap.isNull()) {
            // Cache miss
            if (m_inhibitLazyLoading) {
                // We are requesting cache-only covers and got a cache
                // miss. Record this row so that when we switch to requesting
                // non-cache we can request an update.
                m_cacheMissRows.append(index.row());
            } else {
                if (coverInfo.imageDigest().isEmpty()) {
                    // This happens if we have the legacy hash
                    // The CoverArtCache will take care of the update
                    const auto pTrack = loadTrackByLocation(coverInfo.trackLocation);
                    CoverArtCache::requestTrackCover(
                            this,
                            pTrack,
                            static_cast<int>(option.rect.width() * scaleFactor));
                } else {
                    // This is the fast path with an internal temporary track
                    CoverArtCache::requestCover(
                            this,
                            coverInfo,
                            static_cast<int>(option.rect.width() * scaleFactor));
                }
                m_pendingCacheRows.insert(coverInfo.cacheKey(), index.row());
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
    if (!drewPixmap && coverInfo.color) {
        painter->fillRect(option.rect, mixxx::RgbColor::toQColor(coverInfo.color));
    }

    // Draw a border if the cover art cell has focus
    if (option.state & QStyle::State_HasFocus) {
        // This uses a color from the stylesheet:
        // WTrackTableView {
        //   qproperty-focusBorderColor: red;
        // }
        QPen borderPen(
                m_pFocusBorderColor,
                1,
                Qt::SolidLine,
                Qt::SquareCap);
        painter->setPen(borderPen);
        painter->setBrush(QBrush(Qt::transparent));
        painter->drawRect(
                option.rect.left(),
                option.rect.top(),
                option.rect.width() - 1,
                option.rect.height() - 1);
    }
}
