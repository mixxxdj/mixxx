#include "library/tabledelegates/overviewdelegate.h"

#include "library/dao/trackdao.h"
#include "library/overviewcache.h"
#include "library/trackmodel.h"
#include "moc_overviewdelegate.cpp"
// #include "track/track.h"
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

OverviewDelegate::OverviewDelegate(QTableView* parent)
        : TableItemDelegate(parent),
          m_pTrackModel(asTrackModel(parent)),
          m_pCache(OverviewCache::instance()) {
    /*
    TrackModel* pTrackModel = nullptr;
    if (QTableView* pTableView = qobject_cast<QTableView*>(parent)) {
        m_pTableView = pTableView;
        pTrackModel = dynamic_cast<TrackModel*>(pTableView->model());
    }
    */

    /*
    if (pTrackModel) {
        m_iIdColumn = pTrackModel->fieldIndex(LIBRARYTABLE_ID);
        m_iOverviewColumn = pTrackModel->fieldIndex(LIBRARYTABLE_WAVESUMMARYHEX);
    }
    */

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

    // (const QObject*, TrackId, QPixmap, QSize))
    // (const QObject*, TrackId, QPixmap, QSize))
}

/*
OverviewDelegate::~OverviewDelegate() {
}
*/

void OverviewDelegate::slotOverviewReady(const QObject* pRequester,
        TrackId trackId,
        QPixmap pixmap,
        QSize resizedToSize) {
    kLogger.info() << "slotOverviewReady()" << trackId << pixmap << resizedToSize;

    if (pRequester == this && !pixmap.isNull()) {
        int row = m_trackIdToRow.take(trackId);
        emit overviewReady(row);
        //, m_iOverviewColumn
    }
}

void OverviewDelegate::slotOverviewChanged(TrackId trackId) {
    emit overviewChanged(trackId);
}

void OverviewDelegate::paintItem(QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    paintItemBackground(painter, option, index);

    /*
    OverviewCache* pCache = OverviewCache::instance();
    if (pCache == nullptr) {
        return;
    }

    TrackId trackId(index.sibling(index.row(), m_iIdColumn).data().toInt());
    */

    TrackId trackId(m_pTrackModel->getTrackId(index));

    QPixmap pixmap = m_pCache->requestOverview(trackId, this, option.rect.size());
    if (!pixmap.isNull()) {
        // int width = math_min(pixmap.width(), option.rect.width());
        // int height = math_min(pixmap.height(), option.rect.height());
        // QRect target(option.rect.x(), option.rect.y(),                     width, height);
        // QRect source(0, 0, target.width(), target.height());
        // painter->drawPixmap(target, pixmap, source);
        // QRect source(0, 0, pixmap.width(), pixmap.height());
        painter->drawPixmap(option.rect, pixmap);
    } else {
        m_trackIdToRow[trackId] = index.row();
    }

    // TrackModel *pTrackModel = dynamic_cast<TrackModel*>(m_pTableView->model());
    /*
    if (m_pTrackModel) {
        TrackPointer pTrack = m_pTrackModel->getTrack(index);
        if (pTrack) {
            ConstWaveformPointer pWaveform = pTrack->getWaveform();
            if (pWaveform) {
                kLogger.debug() << "Drawing overview for track" << pTrack->getId();
                painter->drawImage(option.rect, pWaveform->renderToImage());
            }
        }
    }
    */
}
