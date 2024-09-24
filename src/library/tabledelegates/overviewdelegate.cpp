#include "library/tabledelegates/overviewdelegate.h"

#include "library/dao/trackdao.h"
#include "library/overviewcache.h"
#include "library/trackmodel.h"
#include "moc_overviewdelegate.cpp"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("OverviewDelegate");

inline TrackModel* asTrackModel(
        QTableView* pTableView) {
    auto* pTrackModel =
            dynamic_cast<TrackModel*>(pTableView->model());
    DEBUG_ASSERT(pTrackModel);
    return pTrackModel;
}

} // anonymous namespace

OverviewDelegate::OverviewDelegate(QTableView* pTableView)
        : TableItemDelegate(pTableView),
          m_pTrackModel(asTrackModel(pTableView)),
          m_pCache(OverviewCache::instance()) {
    VERIFY_OR_DEBUG_ASSERT(m_pCache) {
        kLogger.warning() << "Caching of overviews is not available";
        return;
    }

    connect(m_pCache,
            &OverviewCache::overviewReady,
            this,
            &OverviewDelegate::slotOverviewReady);

    connect(m_pCache,
            &OverviewCache::overviewChanged,
            this,
            &OverviewDelegate::overviewChanged,
            Qt::DirectConnection); // signal-to-signal
}

/// Maybe request repaint via dataChanged() by BaseTrackTableModel
void OverviewDelegate::slotOverviewReady(const QObject* pRequester,
        const TrackId trackId,
        bool pixmapValid,
        const QSize resizedToSize) {
    Q_UNUSED(resizedToSize);
    // kLogger.info() << "slotOverviewReady()" << trackId << pixmap << resizedToSize;

    if (pRequester == this && pixmapValid) {
        emit overviewReady(trackId);
    }
}

void OverviewDelegate::paintItem(QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    paintItemBackground(painter, option, index);

    if (!m_pCache) {
        return;
    }

    const TrackId trackId(m_pTrackModel->getTrackId(index));

    QPixmap pixmap = m_pCache->requestOverview(trackId, this, option.rect.size());
    if (!pixmap.isNull()) {
        // We have a cached pixmap, paint it.
        painter->drawPixmap(option.rect, pixmap);
    }
}
