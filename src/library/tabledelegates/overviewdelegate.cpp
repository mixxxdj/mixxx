#include "library/tabledelegates/overviewdelegate.h"

#include "library/dao/trackdao.h"
#include "library/trackmodel.h"
#include "moc_overviewdelegate.cpp"
#include "util/logger.h"
#include "util/make_const_iterator.h"
#include "util/timer.h"
#include "waveform/overviews/overviewcache.h"
#include "widget/wlibrary.h"

namespace {

const mixxx::Logger kLogger("OverviewDelegate");

inline TrackModel* asTrackModel(
        QTableView* pTableView) {
    auto* pTrackModel =
            dynamic_cast<TrackModel*>(pTableView->model());
    DEBUG_ASSERT(pTrackModel);
    return pTrackModel;
}

inline WLibrary* findLibraryWidgetParent(QWidget* pWidget) {
    while (pWidget) {
        WLibrary* pLibrary = qobject_cast<WLibrary*>(pWidget);
        if (pLibrary) {
            return pLibrary;
        }

        pWidget = pWidget->parentWidget();
    }

    return nullptr;
}

} // anonymous namespace

OverviewDelegate::OverviewDelegate(QTableView* parent)
        : TableItemDelegate(parent),
          m_pTrackModel(asTrackModel(parent)),
          m_pCache(OverviewCache::instance()),
          m_inhibitLazyLoading(false) {
    WLibrary* pLibrary = findLibraryWidgetParent(parent);
    if (pLibrary) {
        m_signalColors = pLibrary->getOverviewSignalColors();
    }

    if (m_pCache) {
        connect(m_pCache,
                &OverviewCache::overviewReady,
                this,
                &OverviewDelegate::slotOverviewReady);

        connect(m_pCache,
                &OverviewCache::overviewChanged,
                this,
                &OverviewDelegate::slotOverviewChanged);
    } else {
        kLogger.warning() << "Caching of overviews is not available";
    }
}

void OverviewDelegate::emitRowsChanged(QList<int>&& rows) {
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

void OverviewDelegate::slotInhibitLazyLoading(bool inhibitLazyLoading) {
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

void OverviewDelegate::slotOverviewReady(const QObject* pRequester, TrackId trackId) {
    kLogger.info() << "slotOverviewReady()" << this << pRequester << trackId;

    if (pRequester == this) {
        emit overviewChanged(trackId);
    }
}

void OverviewDelegate::slotOverviewChanged(TrackId trackId) {
    kLogger.info() << "slotOverviewChanged()" << trackId;

    emit overviewChanged(trackId);
}

void OverviewDelegate::paintItem(QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    ScopedTimer t(QStringLiteral("OverviewDelegate::paintItem"));

    paintItemBackground(painter, option, index);

    TrackId trackId(m_pTrackModel->getTrackId(index));

    const double scaleFactor = qobject_cast<QWidget*>(parent())->devicePixelRatioF();

    // QPixmap pixmap = , option.rect.size() * scaleFactor
    QImage image = m_pCache->getCachedOverviewImage(trackId, m_signalColors);
    if (image.isNull()) {
        // Cache miss
        if (m_inhibitLazyLoading) {
            // We are requesting cache-only covers and got a cache
            // miss. Record this row so that when we switch to requesting
            // non-cache we can request an update.
            m_cacheMissRows.append(index.row());
        } else {
            m_pCache->requestOverview(trackId,
                    m_signalColors,
                    this,
                    option.rect.size() * scaleFactor);
        }
    } else {
        // Cache hit
        // pixmap.setDevicePixelRatio(scaleFactor);
        // painter->drawPixmap(option.rect, pixmap);
        painter->drawImage(option.rect, image);
    }
}
