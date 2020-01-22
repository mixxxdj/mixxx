#include <QPainter>

#include "library/coverartdelegate.h"
#include "library/coverartcache.h"
#include "library/dao/trackschema.h"
#include "library/trackmodel.h"
#include "widget/wlibrarytableview.h"
#include "util/compatibility.h"
#include "util/math.h"


CoverArtDelegate::CoverArtDelegate(WLibraryTableView* parent)
        : TableItemDelegate(parent),
          m_pTableView(parent),
          m_bOnlyCachedCover(false),
          m_iCoverColumn(-1),
          m_iCoverSourceColumn(-1),
          m_iCoverTypeColumn(-1),
          m_iCoverLocationColumn(-1),
          m_iCoverHashColumn(-1),
          m_iTrackLocationColumn(-1),
          m_iIdColumn(-1) {
    // This assumes that the parent is wtracktableview
    connect(parent,
            &WLibraryTableView::onlyCachedCoverArt,
            this,
            &CoverArtDelegate::slotOnlyCachedCoverArt);

    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache) {
        connect(pCache,
                &CoverArtCache::coverFound,
                this,
                &CoverArtDelegate::slotCoverFound);
    }

    TrackModel* pTrackModel = nullptr;
    QTableView* pTableView = qobject_cast<QTableView*>(parent);
    if (pTableView) {
        pTrackModel = dynamic_cast<TrackModel*>(pTableView->model());
    }

    if (pTrackModel) {
        m_iCoverColumn = pTrackModel->fieldIndex(
            LIBRARYTABLE_COVERART);
        m_iCoverSourceColumn = pTrackModel->fieldIndex(
            LIBRARYTABLE_COVERART_SOURCE);
        m_iCoverTypeColumn = pTrackModel->fieldIndex(
            LIBRARYTABLE_COVERART_TYPE);
        m_iCoverHashColumn = pTrackModel->fieldIndex(
            LIBRARYTABLE_COVERART_HASH);
        m_iCoverLocationColumn = pTrackModel->fieldIndex(
            LIBRARYTABLE_COVERART_LOCATION);
        m_iTrackLocationColumn = pTrackModel->fieldIndex(
            TRACKLOCATIONSTABLE_LOCATION);
        m_iIdColumn = pTrackModel->fieldIndex(
            LIBRARYTABLE_ID);
    }
}

void CoverArtDelegate::slotOnlyCachedCoverArt(bool b) {
    m_bOnlyCachedCover = b;

    // If we can request non-cache covers now, request updates for all rows that
    // were cache misses since the last time.
    if (!m_bOnlyCachedCover) {
        foreach (int row, m_cacheMissRows) {
            emit coverReadyForCell(row, m_iCoverColumn);
        }
        m_cacheMissRows.clear();
    }
}

void CoverArtDelegate::slotCoverFound(const QObject* pRequestor,
                                      const CoverInfoRelative& info,
                                      QPixmap pixmap, bool fromCache) {
    if (pRequestor == this && !pixmap.isNull() && !fromCache) {
        // qDebug() << "CoverArtDelegate::slotCoverFound" << pRequestor << info
        //          << pixmap.size();
        QLinkedList<int> rows = m_hashToRow.take(info.hash);
        foreach(int row, rows) {
            emit coverReadyForCell(row, m_iCoverColumn);
        }
    }
}

void CoverArtDelegate::paintItem(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache == NULL || m_iIdColumn == -1 || m_iCoverSourceColumn == -1 ||
            m_iCoverTypeColumn == -1 || m_iCoverLocationColumn == -1 ||
            m_iCoverHashColumn == -1) {
        return;
    }

    CoverInfo info;
    info.type = static_cast<CoverInfo::Type>(
        index.sibling(index.row(), m_iCoverTypeColumn).data().toInt());

    // We don't support types other than METADATA or FILE currently.
    if (info.type != CoverInfo::METADATA && info.type != CoverInfo::FILE) {
        return;
    }

    info.source = static_cast<CoverInfo::Source>(
        index.sibling(index.row(), m_iCoverSourceColumn).data().toInt());
    info.coverLocation = index.sibling(index.row(), m_iCoverLocationColumn).data().toString();
    info.hash = index.sibling(index.row(), m_iCoverHashColumn).data().toUInt();
    info.trackLocation = index.sibling(index.row(), m_iTrackLocationColumn).data().toString();

    double scaleFactor = getDevicePixelRatioF(static_cast<QWidget*>(parent()));
    // We listen for updates via slotCoverFound above and signal to
    // BaseSqlTableModel when a row's cover is ready.
    QPixmap pixmap = pCache->requestCover(info, this, option.rect.width() * scaleFactor,
                                          m_bOnlyCachedCover, true);
    if (!pixmap.isNull()) {
        pixmap.setDevicePixelRatio(scaleFactor);
        painter->drawPixmap(option.rect.topLeft(), pixmap);
    } else if (!m_bOnlyCachedCover) {
        // If we asked for a non-cache image and got a null pixmap, then our
        // request was queued.
        m_hashToRow[info.hash].append(index.row());
    } else {
        // Otherwise, we are requesting cache-only covers and got a cache
        // miss. Record this row so that when we switch to requesting non-cache
        // we can request an update.
        m_cacheMissRows.append(index.row());
    }
}
