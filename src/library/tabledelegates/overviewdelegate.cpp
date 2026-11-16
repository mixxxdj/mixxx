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

// Visible overviews are requested regardless
constexpr int kMaxPrefetchInFlight = 4;

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
          m_type(mixxx::OverviewType::RGB) {
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

    enablePrefetch();
}

void OverviewDelegate::prefetchWindow() {
    // The queue is rebuilt on every pass, so rows that were
    // scrolled past are never rendered.
    if (m_cellSize.isEmpty()) {
        return;
    }
    QSet<TrackId> queued;
    m_prefetchQueue.clear();
    const qint64 bytesPerRow = qint64{m_cellSize.width()} * m_cellSize.height() * 4;
    int visibleCount = 0;
    const QList<int> rows = prefetchRows(bytesPerRow, &visibleCount);
    for (qsizetype i = 0; i < rows.size(); ++i) {
        const TrackId trackId = m_pTrackModel->getTrackId(
                m_pTableView->model()->index(rows[i], 0));
        if (!trackId.isValid() || queued.contains(trackId) ||
                m_pCache->hasNoOverview(trackId)) {
            continue;
        }
        if (!m_pCache->requestCachedOverview(m_type, trackId, this, m_cellSize).isNull()) {
            continue;
        }
        queued.insert(trackId);
        m_prefetchQueue.append({trackId, i < visibleCount});
    }
    pumpPrefetchQueue();
}

void OverviewDelegate::pumpPrefetchQueue() {
    if (!isPrefetchActive()) {
        m_prefetchQueue.clear();
        return;
    }
    // Visible items are at the front of the queue
    while (!m_prefetchQueue.isEmpty() &&
            (m_prefetchQueue.first().visible ||
                    m_prefetchInFlight.size() < kMaxPrefetchInFlight)) {
        const TrackId trackId = m_prefetchQueue.takeFirst().trackId;
        m_pCache->requestUncachedOverview(
                m_type, m_signalColors, trackId, this, m_cellSize);
        if (m_pCache->isLoading(trackId)) {
            m_prefetchInFlight.insert(trackId);
        }
    }
}

void OverviewDelegate::slotTypeControlChanged(double v) {
    // Assert that v is in enum range to prevent UB.
    DEBUG_ASSERT(v >= 0 && v < QMetaEnum::fromType<mixxx::OverviewType>().keyCount());
    mixxx::OverviewType type = static_cast<mixxx::OverviewType>(static_cast<int>(v));
    if (type == m_type) {
        return;
    }

    m_type = type;
    schedulePrefetch(true);
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

/// Maybe request repaint via dataChanged() by BaseTrackTableModel
void OverviewDelegate::slotOverviewReady(const QObject* pRequester,
        const TrackId trackId,
        bool pixmapValid) {
    // kLogger.info() << "slotOverviewReady()" << trackId << "pixmap valid:" << pixmapValid;
    const bool wasPrefetching = m_prefetchInFlight.remove(trackId);

    if (pRequester == this && pixmapValid) {
        emitOverviewRowsChanged(QSet<TrackId>{trackId});
    }
    if (wasPrefetching) {
        pumpPrefetchQueue();
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
    const QSize cellSize = option.rect.size() * scaleFactor;
    m_cellSize = cellSize;
    // Cache misses are rendered by prefetchWindow()
    QPixmap pixmap = m_pCache->requestCachedOverview(m_type,
            trackId,
            this,
            cellSize);
    schedulePrefetch(pixmap.isNull() && !m_pCache->isLoading(trackId) &&
            !m_pCache->hasNoOverview(trackId));
    if (pixmap.isNull()) {
        paintItemBackground(painter, option, index);
    } else {
        // We have a cached pixmap, paint it
        pixmap.setDevicePixelRatio(scaleFactor);
        painter->drawPixmap(option.rect, pixmap);
    }

    // Draw a border if the cover art cell has focus
    if (option.state & QStyle::State_HasFocus) {
        drawBorder(painter, m_focusBorderColor, option.rect);
    }
}
