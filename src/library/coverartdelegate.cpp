#include "library/coverartdelegate.h"

#include <QPainter>
#include <algorithm>

#include "library/coverartcache.h"
#include "library/dao/trackschema.h"
#include "library/trackmodel.h"
#include "util/logger.h"
#include "widget/wlibrarytableview.h"

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
          m_iCoverSourceColumn(-1),
          m_iCoverTypeColumn(-1),
          m_iCoverLocationColumn(-1),
          m_iCoverHashColumn(-1),
          m_iTrackIdColumn(-1),
          m_iTrackLocationColumn(-1) {
    if (m_pCache) {
        connect(m_pCache,
                &CoverArtCache::coverFound,
                this,
                &CoverArtDelegate::slotCoverFound);
    } else {
        kLogger.warning()
                << "Caching of cover art is not available";
    }

    if (m_pTrackModel) {
        m_iCoverSourceColumn = m_pTrackModel->fieldIndex(
                LIBRARYTABLE_COVERART_SOURCE);
        m_iCoverTypeColumn = m_pTrackModel->fieldIndex(
            LIBRARYTABLE_COVERART_TYPE);
        m_iCoverHashColumn = m_pTrackModel->fieldIndex(
            LIBRARYTABLE_COVERART_HASH);
        m_iCoverLocationColumn = m_pTrackModel->fieldIndex(
                LIBRARYTABLE_COVERART_LOCATION);
        m_iTrackIdColumn = m_pTrackModel->fieldIndex(
                LIBRARYTABLE_ID);
        m_iTrackLocationColumn = m_pTrackModel->fieldIndex(
                TRACKLOCATIONSTABLE_LOCATION);
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
        quint16 requestedHash,
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
    QList<int> refreshRows = m_pendingCacheRows.values(requestedHash);
    m_pendingCacheRows.remove(requestedHash);
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

CoverInfo CoverArtDelegate::coverInfoForIndex(
        const QModelIndex& index) const {
    CoverInfo coverInfo;
    VERIFY_OR_DEBUG_ASSERT(m_iTrackIdColumn >= 0 &&
            m_iCoverSourceColumn >= 0 &&
            m_iCoverTypeColumn >= 0 &&
            m_iCoverLocationColumn >= 0 &&
            m_iCoverHashColumn >= 0) {
        return coverInfo;
    }
    coverInfo.hash =
            index.sibling(index.row(), m_iCoverHashColumn).data().toUInt();
    coverInfo.type = static_cast<CoverInfo::Type>(
            index.sibling(index.row(), m_iCoverTypeColumn).data().toInt());
    coverInfo.source = static_cast<CoverInfo::Source>(
            index.sibling(index.row(), m_iCoverSourceColumn).data().toInt());
    coverInfo.coverLocation =
            index.sibling(index.row(), m_iCoverLocationColumn).data().toString();
    coverInfo.trackLocation =
            index.sibling(index.row(), m_iTrackLocationColumn).data().toString();
    return coverInfo;
}

void CoverArtDelegate::paintItem(
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
