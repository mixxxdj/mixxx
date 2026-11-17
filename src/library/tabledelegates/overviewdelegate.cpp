#include "library/tabledelegates/overviewdelegate.h"

#include <QMetaEnum>

#include "control/controlproxy.h"
#include "library/dao/trackdao.h"
#include "library/dao/trackschema.h"
#include "library/overviewcache.h"
#include "library/trackmodel.h"
#include "moc_overviewdelegate.cpp"
#include "util/logger.h"
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
        : CachedTableItemDelegate(pTableView,
                  asTrackModel(pTableView)->fieldIndex(LIBRARYTABLE_WAVESUMMARYHEX)),
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
}

qint64 OverviewDelegate::prefetchBytesPerRow() const {
    if (m_cellSize.isEmpty()) {
        return 0;
    }
    return qint64{m_cellSize.width()} * m_cellSize.height() * 4;
}

bool OverviewDelegate::prefetchRow(int row, bool /*visible*/) {
    const TrackId trackId = m_pTrackModel->getTrackId(
            m_pTableView->model()->index(row, 0));
    if (!trackId.isValid() || m_pCache->hasNoOverview(trackId)) {
        return true;
    }
    if (m_prefetchInFlight.contains(trackId)) {
        return false;
    }
    // Returns the overview if it is cached, does nothing if it is loading
    const QPixmap pixmap = m_pCache->requestUncachedOverview(
            m_type, m_signalColors, trackId, this, m_cellSize);
    if (!pixmap.isNull()) {
        return true;
    }
    // Also if another view requested it, so we repaint when it's ready
    if (m_pCache->isLoading(trackId)) {
        m_prefetchInFlight.insert(trackId);
    }
    return false;
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

void OverviewDelegate::emitOverviewRowsChanged(
        QSet<TrackId>&& trackIds, bool visibleOnly) {
    if (trackIds.isEmpty()) {
        return;
    }

    QList<int> rows;
    for (auto id : trackIds) {
        const QList<int> idRows = m_pTrackModel->getTrackRows(id);
        rows.append(idRows);
    }
    rows = rowsToRefresh(std::move(rows), visibleOnly);
    if (rows.isEmpty()) {
        // None visible, or none found, which happens during startup
        return;
    }
    emit overviewRowsChanged(rows);
}

/// Maybe request repaint via dataChanged() by BaseTrackTableModel
void OverviewDelegate::slotOverviewReady(const QObject* /*pRequester*/,
        const TrackId trackId,
        bool pixmapValid) {
    // kLogger.info() << "slotOverviewReady()" << trackId << "pixmap valid:" << pixmapValid;
    // Also tracks requests of other views, see prefetchRow()
    if (!m_prefetchInFlight.remove(trackId)) {
        return;
    }
    // Repaint, if another view loaded it we request our size. Only
    // visible rows, each refresh makes the QML library render.
    if (pixmapValid) {
        emitOverviewRowsChanged(QSet<TrackId>{trackId}, true);
    }
    processPrefetchQueue();
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
    // Cache misses are rendered by prefetchRow()
    QPixmap pixmap = m_pCache->requestCachedOverview(m_type,
            trackId,
            this,
            cellSize);
    // Also if another view is loading it, to repaint when it's ready
    schedulePrefetch(pixmap.isNull() && trackId.isValid() &&
            !m_prefetchInFlight.contains(trackId) &&
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
