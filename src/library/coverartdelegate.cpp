#include <QPainter>

#include "library/coverartdelegate.h"
#include "library/dao/trackdao.h"

CoverArtDelegate::CoverArtDelegate(QObject *parent)
        : QStyledItemDelegate(parent),
          m_pCoverCache(CoverArtCache::instance()),
          m_bOnlyCachedCover(false),
          m_sDefaultCover(m_pCoverCache->getDefaultCoverLocation()),
          m_iCoverLocationColumn(-1),
          m_iCoverHashColumn(-1),
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
        m_iCoverHashColumn = pTrackModel->fieldIndex(LIBRARYTABLE_COVERART_HASH);
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

    if (m_iIdColumn == -1 || m_iCoverLocationColumn == -1 || m_iCoverHashColumn == -1) {
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
        info.hash = index.sibling(
            index.row(), m_iCoverHashColumn).data().toString();
        info.trackLocation = index.sibling(
            index.row(), m_iTrackLocationColumn).data().toString();
        QSize coverSize(100, option.rect.height());
        QPixmap pixmap = m_pCoverCache->requestPixmap(info, coverSize,
                                                      m_bOnlyCachedCover, true);

        if (info.coverLocation.isEmpty()) {
            info.coverLocation = m_pCoverCache->trackInDBHash(info.trackId);
        }

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
