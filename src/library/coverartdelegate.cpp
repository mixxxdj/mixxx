#include <QPainter>

#include "library/coverartcache.h"
#include "library/coverartdelegate.h"
#include "library/dao/trackdao.h"

CoverArtDelegate::CoverArtDelegate(QObject *parent)
        : QStyledItemDelegate(parent),
          m_bOnlyCachedCover(false),
          m_sDefaultCover(CoverArtCache::instance()->getDefaultCoverLocation()),
          m_iCoverLocationColumn(-1),
          m_iMd5Column(-1),
          m_iIdColumn(-1) {
    // This assumes that the parent is wtracktableview
    connect(parent, SIGNAL(onlyCachedCoverArt(bool)),
            this, SLOT(slotOnlyCachedCoverArt(bool)));

    TrackModel* pTrackModel = NULL;
    QTableView* pTableView = NULL;
    if (QTableView *tableView = qobject_cast<QTableView*>(parent)) {
        pTableView = tableView;
        pTrackModel = dynamic_cast<TrackModel*>(pTableView->model());
    }

    if (pTrackModel) {
        m_iMd5Column = pTrackModel->fieldIndex(LIBRARYTABLE_COVERART_MD5);
        m_iCoverLocationColumn = pTrackModel->fieldIndex(
            LIBRARYTABLE_COVERART_LOCATION);
        m_iTrackLocationColumn = pTrackModel->fieldIndex(
            TRACKLOCATIONSTABLE_LOCATION);
        m_iIdColumn = pTrackModel->fieldIndex(
            LIBRARYTABLE_ID);

        int coverColumn = pTrackModel->fieldIndex(LIBRARYTABLE_COVERART);
        pTableView->setColumnWidth(coverColumn, 100);
    }
}

CoverArtDelegate::~CoverArtDelegate() {
}

void CoverArtDelegate::slotOnlyCachedCoverArt(bool b) {
    m_bOnlyCachedCover = b;
}

void CoverArtDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    }

    if (m_iIdColumn == -1 || m_iCoverLocationColumn == -1 || m_iMd5Column == -1) {
        return;
    }

    int trackId = index.sibling(
        index.row(), m_iIdColumn).data().toInt();
    if (trackId < 1) {
        return;
    }

    QString coverLocation = index.sibling(
        index.row(), m_iCoverLocationColumn).data().toString();

    // drawing only an existing cover_art,
    // otherwise leave it blank...
    if (coverLocation != m_sDefaultCover) {
        CoverInfo info;
        info.trackId = trackId;
        info.coverLocation = coverLocation;
        info.md5Hash = index.sibling(
            index.row(), m_iMd5Column).data().toString();
        info.trackLocation = index.sibling(
            index.row(), m_iTrackLocationColumn).data().toString();
        QSize coverSize(100, option.rect.height());
        QPixmap pixmap = CoverArtCache::instance()->requestPixmap(info,
                                        coverSize, m_bOnlyCachedCover, true);
        // If the cover was not saved in the DB yet, the coverLocation&md5Hash
        // will not be in the track table. But both info will be loaded in the
        // DBHash (CoverCache::m_queueOfUpdates).
        // In this case, 'CoverArtCache::requestPixmap()' will update the
        // CoverInfo. So, we must check if this location is valid again,
        // in order to avoid displaying the default cover.
        if (!pixmap.isNull() && info.coverLocation != m_sDefaultCover) {
            int width = pixmap.width();
            if (option.rect.width() < width) {
                width = option.rect.width();
            }

            QRect target(option.rect.x(), option.rect.y(),
                         width, option.rect.height());
            QRect source(0, 0, width, pixmap.height());
            painter->drawPixmap(target, pixmap, source);
        }
    }
}
