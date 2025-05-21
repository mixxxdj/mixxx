#include "library/tabledelegates/overviewdelegate.h"

#include <QMetaEnum>

#include "control/controlproxy.h"
#include "library/dao/trackdao.h"
#include "library/overviewcache.h"
#include "library/trackmodel.h"
#include "moc_overviewdelegate.cpp"
#include "util/logger.h"
#include "util/make_const_iterator.h"
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

OverviewDelegate::OverviewDelegate(QTableView* pTableView)
        : TableItemDelegate(pTableView),
          m_pTrackModel(asTrackModel(pTableView)),
          m_pCache(OverviewCache::instance()),
          m_type(mixxx::OverviewType::RGB),
          m_inhibitLazyLoading(false) {
    WLibrary* pLibrary = findLibraryWidgetParent(pTableView);
    if (pLibrary) {
        m_signalColors = pLibrary->getOverviewSignalColors();
    }

    connect(m_pCache,
            &OverviewCache::overviewReady,
            this,
            &OverviewDelegate::slotOverviewReady);

    connect(m_pCache,
            &OverviewCache::overviewChanged,
            this,
            &OverviewDelegate::slotOverviewChanged);

    m_pTypeControl = make_parented<ControlProxy>(
            QStringLiteral("[Waveform]"),
            QStringLiteral("WaveformOverviewType"),
            this);
    m_pTypeControl->connectValueChanged(this, &OverviewDelegate::slotTypeControlChanged);
    slotTypeControlChanged(m_pTypeControl->get());
}

void OverviewDelegate::slotTypeControlChanged(double v) {
    // Assert that v is in enum range to prevent UB.
    DEBUG_ASSERT(v >= 0 && v < QMetaEnum::fromType<mixxx::OverviewType>().keyCount());
    mixxx::OverviewType type = static_cast<mixxx::OverviewType>(static_cast<int>(v));
    if (type == m_type) {
        return;
    }

    m_type = type;
    // Instantly update visible overviews so we get a live preview
    // when changing the type in the preferences.
    m_pTableView->update();
}

void OverviewDelegate::emitOverviewRowsChanged(QSet<TrackId>&& trackIds) {
    if (trackIds.isEmpty()) {
        return;
    }

    QList<int> rows;
    for (auto id : trackIds) {
        const QList<int> idRows = m_pTrackModel->getTrackRows(id);
        rows.append(idRows);
    }
    if (rows.isEmpty()) {
        // For some reason this can happen during startup
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
    emit overviewRowsChanged(std::move(rows));
}

void OverviewDelegate::slotInhibitLazyLoading(bool inhibitLazyLoading) {
    m_inhibitLazyLoading = inhibitLazyLoading;
    if (m_inhibitLazyLoading || m_cacheMissIds.isEmpty()) {
        return;
    }
    // If we can request non-cache covers now, request updates
    // for all rows that were cache misses since the last time.
    // Reset the member variable before mutating the aggregated
    // rows list (-> implicit sharing) and emitting a signal that
    // in turn may trigger new signals for CoverArtDelegate!
    QSet<TrackId> staleIds = std::move(m_cacheMissIds);
    DEBUG_ASSERT(m_cacheMissIds.isEmpty());
    emitOverviewRowsChanged(std::move(staleIds));
}

/// Maybe request repaint via dataChanged() by BaseTrackTableModel
void OverviewDelegate::slotOverviewReady(const QObject* pRequester,
        const TrackId trackId,
        bool pixmapValid) {
    // kLogger.info() << "slotOverviewReady()" << trackId << "pixmap valid:" << pixmapValid;

    if (pRequester == this && pixmapValid) {
        emitOverviewRowsChanged(QSet<TrackId>{trackId});
    }
}

/// Maybe request repaint via dataChanged() by BaseTrackTableModel
void OverviewDelegate::slotOverviewChanged(const TrackId trackId) {
    // kLogger.info() << "slotOverviewChanged()" << trackId;
    emitOverviewRowsChanged(QSet<TrackId>{trackId});
}

void OverviewDelegate::paintItem(QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    const TrackId trackId(m_pTrackModel->getTrackId(index));
    const double scaleFactor = m_pTableView->devicePixelRatioF();
    QPixmap pixmap = m_pCache->requestCachedOverview(m_type,
            trackId,
            this,
            option.rect.size() * scaleFactor);
    if (pixmap.isNull()) {
        // Cache miss
        if (m_inhibitLazyLoading) {
            // We are requesting cache-only covers and got a cache
            // miss. Record this row so that when we switch to requesting
            // non-cache we can request an update.
            m_cacheMissIds.insert(trackId);
        } else {
            pixmap = m_pCache->requestUncachedOverview(m_type,
                    m_signalColors,
                    trackId,
                    this,
                    option.rect.size() * scaleFactor);
        }
        paintItemBackground(painter, option, index);
    } else {
        // We have a cached pixmap, paint it
        pixmap.setDevicePixelRatio(scaleFactor);
        painter->drawPixmap(option.rect, pixmap);
    }
}
