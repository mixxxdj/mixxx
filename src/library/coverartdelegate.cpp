#include <QPainter>

#include "library/coverartcache.h"
#include "library/coverartdelegate.h"
#include "library/dao/trackdao.h"

CoverArtDelegate::CoverArtDelegate(QObject *parent)
        : QStyledItemDelegate(parent),
          m_pTableView(NULL),
          m_pTrackModel(NULL),
          m_bIsLocked(false),
          m_sDefaultCover(CoverArtCache::instance()->getDefaultCoverLocation()),
          m_iCoverLocationColumn(-1),
          m_iMd5Column(-1) {
    // This assumes that the parent is wtracktableview
    connect(parent, SIGNAL(lockCoverArtDelegate(bool)),
            this, SLOT(slotLock(bool)));

    if (QTableView *tableView = qobject_cast<QTableView*>(parent)) {
        m_pTableView = tableView;
        m_pTrackModel = dynamic_cast<TrackModel*>(m_pTableView->model());
    }

    if (m_pTrackModel) {
        m_iMd5Column = m_pTrackModel->fieldIndex(LIBRARYTABLE_COVERART_MD5);
        m_iCoverLocationColumn = m_pTrackModel->fieldIndex(
                                    LIBRARYTABLE_COVERART_LOCATION);

        int coverColumn = m_pTrackModel->fieldIndex(LIBRARYTABLE_COVERART);
        m_pTableView->setColumnWidth(coverColumn, 100);
    }
}

CoverArtDelegate::~CoverArtDelegate() {
}

void CoverArtDelegate::slotLock(bool lock) {
    m_bIsLocked = lock;
}

void CoverArtDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    }

    if (!m_pTrackModel || m_iCoverLocationColumn == -1 || m_iMd5Column == -1) {
        return;
    }

    int trackId = m_pTrackModel->getTrackId(index);
    if (trackId < 1) {
        return;
    }

    QString coverLocation = index.sibling(index.row(),
                                          m_iCoverLocationColumn
                                          ).data().toString();

    // drawing only an existing cover_art,
    // otherwise leave it blank...
    if (coverLocation != m_sDefaultCover) {

        QString md5Hash = index.sibling(index.row(),
                                        m_iMd5Column
                                        ).data().toString();

        QSize coverSize(100, option.rect.height());

        // If the CoverDelegate is locked, it must not try
        // to load (from coverLocation) and search covers.
        // It means that in this cases it will just draw
        // covers which are already in the pixmapcache.
        QPixmap pixmap = CoverArtCache::instance()->requestPixmap(
                trackId, coverLocation, md5Hash, coverSize, !m_bIsLocked, true);

        if (!pixmap.isNull()) {
            // It already got a cropped pixmap (from covercache)
            // that fit to the cell.

            // If you want to change the cropped_cover size,
            // you MUST do it in CoverArtCache::cropCover()
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
